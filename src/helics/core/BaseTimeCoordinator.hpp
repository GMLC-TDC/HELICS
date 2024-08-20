/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "ActionMessage.hpp"
#include "CoreFederateInfo.hpp"
#include "TimeDependencies.hpp"
#include "nlohmann/json_fwd.hpp"

#include <atomic>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace helics {

/** a virtual class defining a time coordinator.  The base class implements some common data and
 * operations that are common to all the time coordinators
 */
class BaseTimeCoordinator {
  protected:
    TimeDependencies dependencies;  //!< federates which this Federate is temporally dependent on
    /// callback used to send the messages
    std::function<void(const ActionMessage&)> sendMessageFunction;
    /// the identifier for inserting into the source id field of any generated messages;

    GlobalFederateId mSourceId{0};
    std::int32_t sequenceCounter{0};  //!< storage for sequence counter
    bool noParent{true};  //!< indicator that the coordinator does not have parents
    bool federatesOnly{false};  //!< indicator that the forwarder only operates with federates
    bool checkingExec{false};
    bool executionMode{false};  //!< flag that the coordinator has entered the execution Mode
    /// flag indicating that a restrictive time policy should be used
    bool restrictive_time_policy{false};
    /// specify that the timeCoordinator should not grant times and instead operate in a continuous
    /// manner until completion
    bool nonGranting{false};
    bool delayedTiming{false};
    bool disconnected{false};

  public:
    static constexpr std::int32_t TIME_COORDINATOR_VERSION{1};
    BaseTimeCoordinator();
    explicit BaseTimeCoordinator(std::function<void(const ActionMessage&)> userSendMessageFunction);
    virtual ~BaseTimeCoordinator() = default;
    /** set the callback function used for the sending messages*/
    void setMessageSender(std::function<void(const ActionMessage&)> userSendMessageFunction);

    void setRestrictivePolicy(bool policy) { restrictive_time_policy = policy; }
    /** get a list of actual dependencies*/
    std::vector<GlobalFederateId> getDependencies() const;
    /** get a reference to the dependents vector*/
    std::vector<GlobalFederateId> getDependents() const;
    /** get the last time grant from a federate */
    Time getLastGrant(GlobalFederateId fedId) const;
    /** set the source id for the time coordinator*/
    void setSourceId(GlobalFederateId sourceId) { mSourceId = sourceId; }
    GlobalFederateId sourceId() const { return mSourceId; }
    /** compute updates to time values
    and send an update if needed
    */
    virtual bool updateTimeFactors() = 0;

    /** take a global id and get a pointer to the dependencyInfo for the other fed
    will be nullptr if it doesn't exist
    */
    const DependencyInfo* getDependencyInfo(GlobalFederateId ofed) const;
    /** check whether a federate is a dependency*/
    bool isDependency(GlobalFederateId ofed) const;
    /** check whether a timeCoordinator has any dependencies or dependents*/
    bool empty() const { return dependencies.empty(); }

  protected:
    /** generate a timeRequest message based on the dependency info data*/
    ActionMessage generateTimeRequest(const TimeData& dep,
                                      GlobalFederateId fed,
                                      std::int32_t responseCode) const;
    /** send the timing info to dependents*/
    void sendTimingInfo();

  public:
    /** process a message related to time
    @return a TimeProcessingResult if it did anything
    */
    virtual TimeProcessingResult processTimeMessage(const ActionMessage& cmd);

    /** process a dependency update message*/
    void processDependencyUpdateMessage(const ActionMessage& cmd);
    /** add a federate dependency
    @return true if it was actually added, false if the federate was already present
    */
    virtual bool addDependency(GlobalFederateId fedID);
    /** add a dependent federate
    @return true if it was actually added, false if the federate was already present
    */
    virtual bool addDependent(GlobalFederateId fedID);
    /** remove a dependency
    @param fedID the identifier of the federate to remove*/
    virtual void removeDependency(GlobalFederateId fedID);
    /** remove a dependent
    @param fedID the identifier of the federate to remove*/
    virtual void removeDependent(GlobalFederateId fedID);
    /** reset a dependency that has been reintroduced
    @param fedID the identifier of the federate to reset*/
    virtual void resetDependency(GlobalFederateId fedID);

    void setAsChild(GlobalFederateId fedID);
    void setAsParent(GlobalFederateId fedID);
    void setVersion(GlobalFederateId fedID, std::int8_t version);
    GlobalFederateId getParent() const;

    /** disconnect*/
    void disconnect();
    /** check if entry to the executing state can be granted*/
    virtual MessageProcessingResult
        checkExecEntry(GlobalFederateId triggerFed = GlobalFederateId{}) = 0;

    /** function to enter the exec Mode
     * @param mode the mode of iteration_request (NO_ITERATIONS, FORCE_ITERATION, ITERATE_IF_NEEDED)
     */
    virtual void enteringExecMode(IterationRequest mode = IterationRequest::NO_ITERATIONS);

    /** generate a string with the current time status*/
    virtual std::string printTimeStatus() const = 0;
    /** generate debugging time information*/
    virtual void generateDebuggingTimeInfo(nlohmann::json& base) const;

    /** check if there are any active Time dependencies*/
    bool hasActiveTimeDependencies() const;

    /** get a count of the active dependencies*/
    int dependencyCount() const;
    /** get a count of the active dependencies*/
    GlobalFederateId getMinDependency() const;

    /** grant timeout check
    @return a json Object with debugging info if empty nothing is logged*/
    nlohmann::json grantTimeoutCheck(const ActionMessage& cmd);
    /** get the current next time*/
    virtual Time getNextTime() const = 0;
};
}  // namespace helics
