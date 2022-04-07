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

/** class managing the coordination of time in HELICS for forwarding object (cores, brokers)
the time coordinator manages dependencies and computes whether time can advance or enter execution
mode
*/
class GlobalTimeCoordinator: public BaseTimeCoordinator {
  private:
    // the variables for time coordination
    Time currentMinTime{Time::minVal()};
    TimeState currentTimeState{TimeState::initialized};
    Time nextEvent{Time::maxVal()};
    std::int32_t sequenceCounter{0};

  protected:
    bool iterating{false};  //!< flag indicating that the min dependency is iterating

  public:
    GlobalTimeCoordinator() = default;

    /** compute updates to time values
    and send an update if needed
    */
    virtual void updateTimeFactors() override;

  private:
    void transmitTimingMessagesUpstream(ActionMessage& msg) const;
    void transmitTimingMessagesDownstream(ActionMessage& msg,
                                          GlobalFederateId skipFed = GlobalFederateId{}) const;

  public:
    /** check if entry to the executing state can be granted*/
    virtual MessageProcessingResult checkExecEntry() override;

    /** generate a string with the current time status*/
    virtual std::string printTimeStatus() const override;
    /** generate debugging time information*/
    virtual void generateDebuggingTimeInfo(Json::Value& base) const override;

    /** get the current next time*/
    Time getNextTime() const { return currentMinTime; }

};
}  // namespace helics