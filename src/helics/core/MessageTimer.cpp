/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "MessageTimer.hpp"

namespace helics
{
MessageTimer::MessageTimer (std::function<void(ActionMessage &&)> sFunction) : sendFunction (std::move (sFunction)), servicePtr(AsioServiceManager::getServicePointer ())
{
    std::cout << "getting loop handle for timer" << std::endl;
    loopHandle = servicePtr->startServiceLoop ();
    std::cout << "got loop handle for timer" << std::endl;
}

static void
processTimerCallback (std::shared_ptr<MessageTimer> mtimer, int32_t index, const boost::system::error_code &ec)
{
    if (ec != boost::asio::error::operation_aborted)
    {
        try
        {
            mtimer->sendMessage (index);
        }
        catch (std::exception &e)
        {
            std::cout << "exception caught from addActionMessage" << std::endl;
        }
    }
}

int32_t MessageTimer::addTimerFromNow (std::chrono::nanoseconds time, ActionMessage mess)
{
    return addTimer (std::chrono::steady_clock::now () + time, std::move (mess));
}

int32_t MessageTimer::addTimer (time_type expireTime, ActionMessage mess)
{
    // these two calls need to be before the lock
    auto timer = std::make_shared<boost::asio::steady_timer> (servicePtr->getBaseService ());
    timer->expires_at (expireTime);
    std::lock_guard<std::mutex> lock (timerLock);
    auto index = static_cast<int32_t> (timers.size ());
    auto timerCallback = [ptr = shared_from_this (), index](const boost::system::error_code &ec) {
        processTimerCallback (ptr, index, ec);
    };

    timer->async_wait (timerCallback);
    timers.push_back (std::move (timer));
    buffers.push_back (std::move (mess));
    expirationTimes.push_back (expireTime);
    return index;
}

void MessageTimer::cancelTimer (int32_t index)
{
    std::lock_guard<std::mutex> lock (timerLock);
    if ((index >= 0) && (index < static_cast<int32_t> (timers.size ())))
    {
        timers[index]->cancel ();
        buffers[index].setAction (CMD_IGNORE);
    }
}

void MessageTimer::cancelAll ()
{
    std::lock_guard<std::mutex> lock (timerLock);
    for (auto &tmr : timers)
    {
        tmr->cancel ();
    }
    for (auto &buf : buffers)
    {
        buf.setAction (CMD_IGNORE);
    }
}

void MessageTimer::updateTimer (int32_t timerIndex, time_type expirationTime, ActionMessage mess)
{
    std::lock_guard<std::mutex> lock (timerLock);
    if ((timerIndex >= 0) && (timerIndex < static_cast<int32_t> (timers.size ())))
    {
        timers[timerIndex]->expires_at (expirationTime);
        auto timerCallback = [ptr = shared_from_this (), timerIndex](const boost::system::error_code &ec) {
            processTimerCallback (ptr, timerIndex, ec);
        };
        expirationTimes[timerIndex] = expirationTime;
        buffers[timerIndex] = std::move (mess);
        timers[timerIndex]->async_wait (timerCallback);
    }
}

bool MessageTimer::addTimeToTimer (int32_t timerIndex, std::chrono::nanoseconds time)
{
    std::lock_guard<std::mutex> lock (timerLock);
    if ((timerIndex >= 0) && (timerIndex < static_cast<int32_t> (timers.size ())))
    {
        auto newTime = timers[timerIndex]->expires_at () + time;
        timers[timerIndex]->expires_at (newTime);
        auto timerCallback = [ptr = shared_from_this (), timerIndex](const boost::system::error_code &ec) {
            processTimerCallback (ptr, timerIndex, ec);
        };
        expirationTimes[timerIndex] = newTime;
        auto ret = (buffers[timerIndex].action () != CMD_IGNORE);
        timers[timerIndex]->async_wait (timerCallback);
        return ret;
    }
    return false;
}

bool MessageTimer::updateTimer (int32_t timerIndex, time_type expirationTime)
{
    std::lock_guard<std::mutex> lock (timerLock);
    if ((timerIndex >= 0) && (timerIndex < static_cast<int32_t> (timers.size ())))
    {
        timers[timerIndex]->expires_at (expirationTime);
        auto timerCallback = [ptr = shared_from_this (), timerIndex](const boost::system::error_code &ec) {
            processTimerCallback (ptr, timerIndex, ec);
        };
        expirationTimes[timerIndex] = expirationTime;
        auto ret = (buffers[timerIndex].action () != CMD_IGNORE);
        timers[timerIndex]->async_wait (timerCallback);
        return ret;
    }
    return false;
}

/** update the message associated with a timer*/
void MessageTimer::updateMessage (int32_t timerIndex, ActionMessage mess)
{
    std::lock_guard<std::mutex> lock (timerLock);
    if ((timerIndex >= 0) && (timerIndex < static_cast<int32_t> (timers.size ())))
    {
        buffers[timerIndex] = std::move (mess);
    }
}

void MessageTimer::sendMessage (int32_t timerIndex)
{
    std::unique_lock<std::mutex> lock (timerLock);
    if ((timerIndex >= 0) && (timerIndex < static_cast<int32_t> (timers.size ())))
    {
        if (std::chrono::steady_clock::now () >= expirationTimes[timerIndex])
        {
            if (buffers[timerIndex].action () != CMD_IGNORE)
            {
                auto buf = std::move (buffers[timerIndex]);
                buffers[timerIndex].setAction (CMD_IGNORE);
                lock.unlock ();  // don't keep a lock while calling a callback
                sendFunction (std::move (buf));
            }
        }
    }
}

}  // namespace helics
