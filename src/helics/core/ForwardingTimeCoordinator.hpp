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

#include <atomic>
#include <functional>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <utility>
#include <vector>

namespace helics {

/** class managing the coordination of time in HELICS for forwarding object (cores, brokers)
the time coordinator manages dependencies and computes whether time can advance or enter execution
mode
*/
class ForwardingTimeCoordinator: public BaseTimeCoordinator {
  private:
    // the variables for time coordination
    DependencyInfo upstream;
    DependencyInfo downstream;
    static constexpr std::int32_t mSequenceIncrement{100};

  protected:
    bool iterating{false};  //!< flag indicating that the min dependency is iterating
    bool ignoreMinFed{false};  //!< flag indicating that minFed Controls should not be used
    std::int32_t sequenceModifier{0};

  public:
    ForwardingTimeCoordinator() = default;

    /** compute updates to time values
    and send an update if needed
    */
    virtual bool updateTimeFactors() override;

  private:
    void transmitTimingMessagesUpstream(ActionMessage& msg) const;
    void transmitTimingMessagesDownstream(ActionMessage& msg,
                                          GlobalFederateId skipFed = GlobalFederateId{}) const;

  public:
    virtual TimeProcessingResult processTimeMessage(const ActionMessage& cmd) override;
    /** check if entry to the executing state can be granted*/
    virtual MessageProcessingResult
        checkExecEntry(GlobalFederateId triggerFed = GlobalFederateId{}) override;

    /** generate a string with the current time status*/
    virtual std::string printTimeStatus() const override;
    /** generate debugging time information*/
    virtual void generateDebuggingTimeInfo(nlohmann::json& base) const override;

    /** get the current next time*/
    virtual Time getNextTime() const override { return downstream.next; }
};
}  // namespace helics
