/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "MessageTimer.hpp"

#include <iostream>
#include <utility>

namespace helics {
MessageTimer::MessageTimer(std::function<void(ActionMessage&&)> sFunction):
    sendFunction(std::move(sFunction)), contextPtr(AsioContextManager::getContextPointer()),
    loopHandle(contextPtr->startContextLoop())
{
}

static void processTimerCallback(std::shared_ptr<MessageTimer> mtimer,
                                 int32_t index,
                                 const std::error_code& ec)
{
    if (ec != asio::error::operation_aborted) {
        try {
            mtimer->sendMessage(index);
        }
        catch (std::exception& e) {
            std::cerr << "exception caught from sendMessage:" << e.what() << std::endl;
        }
    }
}

int32_t MessageTimer::addTimerFromNow(std::chrono::nanoseconds time, ActionMessage mess)
{
    return addTimer(std::chrono::steady_clock::now() + time, std::move(mess));
}

int32_t MessageTimer::addTimer(time_type expirationTime, ActionMessage mess)
{
    // these two calls need to be before the lock
    auto timer = std::make_shared<asio::steady_timer>(contextPtr->getBaseContext());
    timer->expires_at(expirationTime);

    std::unique_lock<std::mutex> lock(timerLock);

    auto index = static_cast<int32_t>(timers.size());
    auto timerCallback = [ptr = shared_from_this(), index](const std::error_code& ec) {
        processTimerCallback(ptr, index, ec);
    };
    buffers.push_back(std::move(mess));
    expirationTimes.push_back(expirationTime);
    timers.push_back(std::move(timer));
    if (expirationTime > std::chrono::steady_clock::now()) {
        timers.back()->async_wait(timerCallback);
    } else {
        lock.unlock();
        timerCallback(std::error_code{});
    }

    return index;
}

void MessageTimer::cancelTimer(int32_t index)
{
    std::lock_guard<std::mutex> lock(timerLock);
    if ((index >= 0) && (index < static_cast<int32_t>(timers.size()))) {
        buffers[index].setAction(CMD_IGNORE);
        timers[index]->cancel();
    }
}

void MessageTimer::cancelAll()
{
    std::lock_guard<std::mutex> lock(timerLock);
    for (auto& buf : buffers) {
        buf.setAction(CMD_IGNORE);
    }
    for (auto& tmr : timers) {
        tmr->cancel();
    }
}

void MessageTimer::updateTimer(int32_t timerIndex, time_type expirationTime, ActionMessage mess)
{
    std::lock_guard<std::mutex> lock(timerLock);
    if ((timerIndex >= 0) && (timerIndex < static_cast<int32_t>(timers.size()))) {
        timers[timerIndex]->expires_at(expirationTime);
        expirationTimes[timerIndex] = expirationTime;
        buffers[timerIndex] = std::move(mess);

        auto timerCallback = [ptr = shared_from_this(), timerIndex](const std::error_code& ec) {
            processTimerCallback(ptr, timerIndex, ec);
        };

        timers[timerIndex]->async_wait(timerCallback);
    }
}

bool MessageTimer::addTimeToTimer(int32_t timerIndex, std::chrono::nanoseconds time)
{
    std::lock_guard<std::mutex> lock(timerLock);
    if ((timerIndex >= 0) && (timerIndex < static_cast<int32_t>(timers.size()))) {
        auto newTime = timers[timerIndex]->expires_at() + time;
        timers[timerIndex]->expires_at(newTime);
        auto timerCallback = [ptr = shared_from_this(), timerIndex](const std::error_code& ec) {
            processTimerCallback(ptr, timerIndex, ec);
        };
        expirationTimes[timerIndex] = newTime;
        auto ret = (buffers[timerIndex].action() != CMD_IGNORE);
        timers[timerIndex]->async_wait(timerCallback);
        return ret;
    }
    return false;
}

bool MessageTimer::updateTimer(int32_t timerIndex, time_type expirationTime)
{
    std::lock_guard<std::mutex> lock(timerLock);
    if ((timerIndex >= 0) && (timerIndex < static_cast<int32_t>(timers.size()))) {
        timers[timerIndex]->expires_at(expirationTime);
        auto timerCallback = [ptr = shared_from_this(), timerIndex](const std::error_code& ec) {
            processTimerCallback(ptr, timerIndex, ec);
        };
        expirationTimes[timerIndex] = expirationTime;
        auto ret = (buffers[timerIndex].action() != CMD_IGNORE);
        timers[timerIndex]->async_wait(timerCallback);
        return ret;
    }
    return false;
}

/** update the message associated with a timer*/
void MessageTimer::updateMessage(int32_t timerIndex, ActionMessage mess)
{
    std::lock_guard<std::mutex> lock(timerLock);
    if ((timerIndex >= 0) && (timerIndex < static_cast<int32_t>(timers.size()))) {
        buffers[timerIndex] = std::move(mess);
    }
}

void MessageTimer::sendMessage(int32_t timerIndex)
{
    std::unique_lock<std::mutex> lock(timerLock);
    if ((timerIndex >= 0) && (timerIndex < static_cast<int32_t>(timers.size()))) {
        if (std::chrono::steady_clock::now() >= expirationTimes[timerIndex]) {
            if (buffers[timerIndex].action() != CMD_IGNORE) {
                ActionMessage buf = std::move(buffers[timerIndex]);
                buffers[timerIndex].setAction(CMD_IGNORE);  // clear out the action
                lock.unlock();  // don't keep a lock while calling a callback
                sendFunction(std::move(buf));
            }
        }
    }
}

}  // namespace helics
