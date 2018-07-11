/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../common/AsioServiceManager.h"
#include "ActionMessage.hpp"
#include <memory>
#include <mutex>
#include <boost/asio/steady_timer.hpp>

namespace helics
{
/** class containing a message timer for sending messages at particular points in time
 */
class MessageTimer : public std::enable_shared_from_this<MessageTimer>
{
  public:
    using time_type = decltype (std::chrono::steady_clock::now ());
    explicit MessageTimer (std::function<void(ActionMessage &&)> sFunction);
    /** ad a timer and message to the queue
    @returns an index for referencing the timer in the future*/
    int32_t addTimerFromNow (std::chrono::nanoseconds time, ActionMessage mess);
    /** ad a timer and message to the queue
    @returns an index for referencing the timer in the future*/
    int32_t addTimer (time_type expirationTime, ActionMessage mess);
    /** cancel a timer by index*/
    void cancelTimer (int32_t index);
    /** cancel all timers*/
    void cancelAll ();
    /** update the message time of a timer and its message*/
    void updateTimer (int32_t timerIndex, time_type expirationTime, ActionMessage mess);
    /** update the message time of a timer keeping its message the same
    if the message has been sent already the message stored is invalid
    @return true if the update was successful and false if not*/
    bool updateTimer (int32_t timerIndex, time_type expirationTime);
    /** update the message time of a timer keeping its message the same
    the function will add time to the timer
    @return true if the update was successful and false if not*/
    bool addTimeToTimer (int32_t timerIndex, std::chrono::nanoseconds time);
    /** update the message associated with a timer*/
    void updateMessage (int32_t timerIndex, ActionMessage mess);
    /** execute the send function associated with a message*/
    void sendMessage (int32_t timerIndex);

  private:
    std::mutex timerLock;  //!< lock protecting the timer buffers
    std::vector<std::shared_ptr<boost::asio::steady_timer>> timers;
    const std::function<void(ActionMessage &&)> sendFunction;  //!< the callback to use when sending a message
    std::vector<ActionMessage> buffers;
    std::vector<time_type> expirationTimes;
    std::shared_ptr<AsioServiceManager> servicePtr;  //!< service manager to for handling real time operations
    decltype (servicePtr->runServiceLoop ()) loopHandle;  //!< loop controller for async real time operations
};
}  // namespace helics
