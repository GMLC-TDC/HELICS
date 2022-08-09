/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "ActionMessage.hpp"
#include "BaseTimeCoordinator.hpp"
#include "CoreFederateInfo.hpp"
#include "TimeDependencies.hpp"

#include "json/forwards.h"
#include <atomic>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace helics {

/** class implementing a global time coordination in a non distributed fashion
 */
class GlobalTimeCoordinator: public BaseTimeCoordinator {
  private:
    // the variables for time coordination
    Time currentMinTime{Time::minVal()};
    TimeState currentTimeState{TimeState::initialized};
    Time nextEvent{Time::maxVal()};
    int execChecks{0};
    GlobalFederateId lastExec{0};

  protected:
    bool iterating{false};  //!< flag indicating that the min dependency is iterating

  public:
    GlobalTimeCoordinator() = default;

    /** compute updates to time values
    and send an update if needed
    */
    virtual bool updateTimeFactors() override;

  private:
    void transmitTimingMessagesUpstream(ActionMessage& msg) const;
    void transmitTimingMessagesDownstream(ActionMessage& msg,
                                          GlobalFederateId skipFed = GlobalFederateId{}) const;

    void sendTimeUpdateRequest(Time triggerTime);

  public:
    /** check if entry to the executing state can be granted*/
    virtual MessageProcessingResult
        checkExecEntry(GlobalFederateId triggerFed = GlobalFederateId{}) override;

    /** generate a string with the current time status*/
    virtual std::string printTimeStatus() const override;
    /** generate debugging time information*/
    virtual void generateDebuggingTimeInfo(Json::Value& base) const override;

    /** get the current next time*/
    virtual Time getNextTime() const override { return currentMinTime; }
};
}  // namespace helics
