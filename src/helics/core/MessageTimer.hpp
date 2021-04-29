/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../common/AsioContextManager.h"
#include "ActionMessage.hpp"

#include <asio/steady_timer.hpp>
#include <memory>
#include <mutex>
#include <vector>

namespace helics {
/** class containing a message timer for sending messages at particular points in time
 */
class MessageTimer: public std::enable_shared_from_this<MessageTimer> {
  public:
    using time_type = decltype(std::chrono::steady_clock::now());
    explicit MessageTimer(std::function<void(ActionMessage&&)> sFunction);
    /** add a timer and message to the queue
    @return an index for referencing the timer in the future*/
    int32_t addTimerFromNow(std::chrono::nanoseconds time, ActionMessage mess);
    /** add a timer and message to the queue
    @return an index for referencing the timer in the future*/
    int32_t addTimer(time_type expirationTime, ActionMessage mess);
    /** cancel a timer by index*/
    void cancelTimer(int32_t index);
    /** cancel all timers*/
    void cancelAll();
    /** update the message time of a timer and its message*/
    void updateTimer(int32_t timerIndex, time_type expirationTime, ActionMessage mess);
    /** update the message time of a timer keeping its message the same
    if the message has been sent already the message stored is invalid
    @return true if the update was successful and false if not*/
    bool updateTimer(int32_t timerIndex, time_type expirationTime);
    /** update the message time of a timer keeping its message the same
    the function will add time to the timer
    @return true if the update was successful and false if not*/
    bool addTimeToTimer(int32_t timerIndex, std::chrono::nanoseconds time);
    /** update the message associated with a timer*/
    void updateMessage(int32_t timerIndex, ActionMessage mess);
    /** execute the send function associated with a message*/
    void sendMessage(int32_t timerIndex);

  private:
    std::mutex timerLock;  //!< lock protecting the timer buffers
    std::vector<ActionMessage> buffers;
    std::vector<time_type> expirationTimes;
    const std::function<void(ActionMessage&&)>
        sendFunction;  //!< the callback to use when sending a message
    std::vector<std::shared_ptr<asio::steady_timer>> timers;
    std::shared_ptr<AsioContextManager>
        contextPtr;  //!< context manager to for handling real time operations
    decltype(contextPtr->startContextLoop())
        loopHandle;  //!< loop controller for async real time operations
};
}  // namespace helics
