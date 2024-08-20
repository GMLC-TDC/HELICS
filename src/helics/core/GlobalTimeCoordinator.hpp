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

/** class implementing a global time coordination in a non distributed fashion
 */
class GlobalTimeCoordinator: public BaseTimeCoordinator {
  private:
    // the variables for time coordination
    Time currentMinTime{Time::minVal()};
    TimeState currentTimeState{TimeState::initialized};
    Time nextEvent{Time::maxVal()};
    static constexpr std::int32_t mSequenceIncrement{100};

  protected:
    bool iterating{false};  //!< flag indicating that the min dependency is iterating
    bool mNewRequest{
        true};  //!< flag indicating a new request has been received since the last sequence Update
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
    virtual TimeProcessingResult processTimeMessage(const ActionMessage& cmd) override;
    virtual MessageProcessingResult
        checkExecEntry(GlobalFederateId triggerFed = GlobalFederateId{}) override;

    virtual std::string printTimeStatus() const override;
    virtual void generateDebuggingTimeInfo(nlohmann::json& base) const override;

    virtual Time getNextTime() const override { return currentMinTime; }
};
}  // namespace helics
