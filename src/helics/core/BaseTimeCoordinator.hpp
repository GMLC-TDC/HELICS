/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "ActionMessage.hpp"
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
class BaseTimeCoordinator {
  protected:
    // Core::local_federate_id parent = invalid_fed_id;  //!< the id for the parent object which
    // should also be a ForwardingTimeCoordinator
    TimeDependencies dependencies;  //!< federates which this Federate is temporally dependent on
    /// callback used to send the messages
    std::function<void(const ActionMessage&)> sendMessageFunction;
    /// the identifier for inserting into the source id field of any generated messages;

    GlobalFederateId mSourceId{0};
    bool noParent{true};  //!< indicator that the coordinator does not have parents
    bool federatesOnly{false};  //!< indicator that the forwarder only operates with federates
    bool checkingExec{false};
    bool executionMode{false};  //!< flag that the coordinator has entered the execution Mode
    /// flag indicating that a restrictive time policy should be used
    bool restrictive_time_policy{false};

  public:
    BaseTimeCoordinator() = default;
    virtual ~BaseTimeCoordinator() = default;
    /** set the callback function used for the sending messages*/
    void setMessageSender(std::function<void(const ActionMessage&)> userSendMessageFunction)
    {
        sendMessageFunction = std::move(userSendMessageFunction);
    }
    void setRestrictivePolicy(bool policy) { restrictive_time_policy = policy; }
    /** get a list of actual dependencies*/
    std::vector<GlobalFederateId> getDependencies() const;
    /** get a reference to the dependents vector*/
    std::vector<GlobalFederateId> getDependents() const;

    void setSourceId(GlobalFederateId sourceId) { mSourceId = sourceId; }
    GlobalFederateId sourceId() const { return mSourceId; }
    /** compute updates to time values
    and send an update if needed
    */
    virtual void updateTimeFactors() = 0;

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
    ActionMessage generateTimeRequest(const DependencyInfo& dep, GlobalFederateId fed) const;

  public:
    /** process a message related to time
    @return a message_process_result if it did anything
    */
    bool processTimeMessage(const ActionMessage& cmd);

    /** process a dependency update message*/
    void processDependencyUpdateMessage(const ActionMessage& cmd);
    /** add a federate dependency
    @return true if it was actually added, false if the federate was already present
    */
    bool addDependency(GlobalFederateId fedID);
    /** add a dependent federate
    @return true if it was actually added, false if the federate was already present
    */
    bool addDependent(GlobalFederateId fedID);
    /** remove a dependency
    @param fedID the identifier of the federate to remove*/
    void removeDependency(GlobalFederateId fedID);
    /** remove a dependent
    @param fedID the identifier of the federate to remove*/
    void removeDependent(GlobalFederateId fedID);

    void setAsChild(GlobalFederateId fedID);
    void setAsParent(GlobalFederateId fedID);

    /** disconnect*/
    void disconnect();
    /** check if entry to the executing state can be granted*/
    virtual MessageProcessingResult checkExecEntry() = 0;

    /** function to enter the exec Mode
     */
    void enteringExecMode();

    /** generate a string with the current time status*/
    virtual std::string printTimeStatus() const = 0;
    /** generate debugging time information*/
    virtual void generateDebuggingTimeInfo(Json::Value& base) const;

    /** check if there are any active Time dependencies*/
    bool hasActiveTimeDependencies() const;

    /** get a count of the active dependencies*/
    int dependencyCount() const;
    /** get a count of the active dependencies*/
    GlobalFederateId getMinDependency() const;

    /** grant timeout check
    @return a json Object with debugging info if empty nothing is logged*/
    Json::Value grantTimeoutCheck(const ActionMessage& cmd);
    /** get the current next time*/
    virtual Time getNextTime() const = 0;
};
}  // namespace helics
