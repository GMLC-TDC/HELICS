/*
Copyright (c) 2017-2021,
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
class ForwardingTimeCoordinator {
  private:
    // the variables for time coordination
    DependencyInfo upstream;
    DependencyInfo downstream;

    // Core::local_federate_id parent = invalid_fed_id;  //!< the id for the parent object which
    // should also be a ForwardingTimeCoordinator
    TimeDependencies dependencies;  //!< federates which this Federate is temporally dependent on
    /// callback used to send the messages
    std::function<void(const ActionMessage&)> sendMessageFunction;

  public:
    /// the identifier for inserting into the source id field of any generated messages;
    global_federate_id source_id{0};
    /// flag indicating that the coordinator is trying to enter the exec mode
    bool checkingExec{false};
    bool executionMode{false};  //!< flag that the coordinator has entered the execution Mode
    bool iterating{false};  //!< flag indicating that the min dependency is iterating
    bool ignoreMinFed{false};  //!< flag indicating that minFed Controls should not be used
    /// flag indicating that a restrictive time policy should be used
    bool restrictive_time_policy{false};
    bool noParent{false};  //!< indicator that the coordinator does not have parents
  private:
    bool federatesOnly{false};  //!< indicator that the forwarder only operates with federates
  public:
    ForwardingTimeCoordinator() = default;

    /** set the callback function used for the sending messages*/
    void setMessageSender(std::function<void(const ActionMessage&)> userSendMessageFunction)
    {
        sendMessageFunction = std::move(userSendMessageFunction);
    }

    /** get a list of actual dependencies*/
    std::vector<global_federate_id> getDependencies() const;
    /** get a reference to the dependents vector*/
    std::vector<global_federate_id> getDependents() const;

    /** compute updates to time values
    and send an update if needed
    */
    void updateTimeFactors();

    /** take a global id and get a pointer to the dependencyInfo for the other fed
    will be nullptr if it doesn't exist
    */
    const DependencyInfo* getDependencyInfo(global_federate_id ofed) const;
    /** check whether a federate is a dependency*/
    bool isDependency(global_federate_id ofed) const;
    /** check whether a timeCoordinator has any dependencies or dependents*/
    bool empty() const { return dependencies.empty(); }

  private:
    void transmitTimingMessagesUpstream(ActionMessage& msg) const;
    void transmitTimingMessagesDownstream(ActionMessage& msg) const;
    /** generate a timeRequest message based on the dependency info data*/
    ActionMessage generateTimeRequest(const DependencyInfo& dep, global_federate_id fed) const;

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
    bool addDependency(global_federate_id fedID);
    /** add a dependent federate
    @return true if it was actually added, false if the federate was already present
    */
    bool addDependent(global_federate_id fedID);
    /** remove a dependency
    @param fedID the identifier of the federate to remove*/
    void removeDependency(global_federate_id fedID);
    /** remove a dependent
    @param fedID the identifier of the federate to remove*/
    void removeDependent(global_federate_id fedID);

    void setAsChild(global_federate_id fedID);
    void setAsParent(global_federate_id fedID);

    /** disconnect*/
    void disconnect();
    /** check if entry to the executing state can be granted*/
    message_processing_result checkExecEntry();

    /** function to enter the exec Mode
     */
    void enteringExecMode();

    /** generate a string with the current time status*/
    std::string printTimeStatus() const;
    /** generate debugging time information*/
    void generateDebuggingTimeInfo(Json::Value& base) const;

    /** check if there are any active Time dependencies*/
    bool hasActiveTimeDependencies() const;
    /** get the current next time*/
    Time getNextTime() const { return downstream.next; }
    /** get a count of the active dependencies*/
    int dependencyCount() const;
    /** get a count of the active dependencies*/
    global_federate_id getMinDependency() const;
};
}  // namespace helics
