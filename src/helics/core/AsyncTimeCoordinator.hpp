/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "ActionMessage.hpp"
#include "BaseTimeCoordinator.hpp"
#include "CoreFederateInfo.hpp"
#include "TimeDependencies.hpp"
#include "nlohmann/json_fwd.hpp"

#include <atomic>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace helics {

/** class implementing a time coordinator that explicitly allows asynchronous operation of the
federates -- that is, the federates are not time synchronized and are allowed to operate ahead of
others. Potential uses are: 1) the use of an external time coordination mechanism, 2) a purely
command driven system, 3) all federates are operated in real time mode, 4) you don't care about
the data and just want to see how fast all the federates go on their own.

This time coordinator does only minimal time keeping for entry to execution mode and allows
asynchronous operation of the federation
 */
class AsyncTimeCoordinator: public BaseTimeCoordinator {
  private:
    // the variables for time coordination
    Time currentMinTime{Time::minVal()};
    TimeState currentTimeState{TimeState::initialized};
    Time nextEvent{Time::maxVal()};

  protected:
    bool iterating{false};  //!< flag indicating that the min dependency is iterating

  public:
    AsyncTimeCoordinator() = default;

    /** compute updates to time values
    and send an update if needed
    */
    virtual bool updateTimeFactors() override;

  private:
    void transmitTimingMessagesUpstream(ActionMessage& msg) const;
    void transmitTimingMessagesDownstream(ActionMessage& msg,
                                          GlobalFederateId skipFed = GlobalFederateId{}) const;

  public:
    /** check if entry to the executing state can be granted*/
    virtual MessageProcessingResult
        checkExecEntry(GlobalFederateId triggerFed = GlobalFederateId{}) override;

    /** generate a string with the current time status*/
    virtual std::string printTimeStatus() const override;
    /** generate debugging time information*/
    virtual void generateDebuggingTimeInfo(nlohmann::json& base) const override;

    /** get the current next time*/
    virtual Time getNextTime() const override { return currentMinTime; }
};
}  // namespace helics
