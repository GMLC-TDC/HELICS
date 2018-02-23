/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/
#pragma once

#include "ActionMessage.hpp"
#include "CoreFederateInfo.hpp"
#include "TimeDependencies.hpp"
#include <atomic>
#include <functional>

namespace helics
{
/** class managing the coordination of time in HELICS for forwarding object (cores, brokers)
the time coordinator manages dependencies and computes whether time can advance or enter execution mode
*/
class ForwardingTimeCoordinator
{
private:
    // the variables for time coordination
    Time time_next = timeZero;  //!< the next possible internal event time
    Time time_minminDe = timeZero;  //!< the minimum  of the minimum dependency event Time
    Time time_minDe = timeZero;  //!< the minimum event time of the dependencies
    DependencyInfo::time_state_t time_state = DependencyInfo::time_state_t::time_requested; //!< the current forwarding time state
    Core::federate_id_t lastMinFed = invalid_fed_id; //!< the latest minimum fed
    TimeDependencies dependencies;  //!< federates which this Federate is temporally dependent on
    std::vector<Core::federate_id_t> dependents;  //!< federates which temporally depend on this federate

    std::function<void(const ActionMessage &)> sendMessageFunction;  //!< callback used to send the messages

public:
    Core::federate_id_t
        source_id;  //!< the identifier for inserting into the source id field of any generated messages;
    bool checkingExec = false; //!< flag indicating that the coordinator is trying to enter the exec mode
    bool executionMode = false;	//!< flag that the coordinator has entered the execution Mode
    bool iterating = false; //!< flag indicating that the min dependency is iterating
public:
    ForwardingTimeCoordinator() = default;

    /** set the callback function used for the sending messages*/
    void setMessageSender(std::function<void(const ActionMessage &)> sendMessageFunction_)
    {
        sendMessageFunction = std::move(sendMessageFunction_);
    }

    /** get a list of actual dependencies*/
    std::vector < Core::federate_id_t> getDependencies() const;
    /** get a reference to the dependents vector*/
    const std::vector<Core::federate_id_t> &getDependents() const { return dependents; }

    /** compute updates to time values
    and send an update if needed
    */
    void updateTimeFactors();

    /** take a global id and get a pointer to the dependencyInfo for the other fed
    will be nullptr if it doesn't exist
    */
    DependencyInfo *getDependencyInfo(Core::federate_id_t ofed);
    /** check whether a federate is a dependency*/
    bool isDependency(Core::federate_id_t ofed) const;

private:
    /**send out the latest time request command*/
    void sendTimeRequest() const;
public:
    /** process a message related to time
    @return true if it did anything
    */
    bool processTimeMessage(const ActionMessage &cmd);


    /** process a dependency update message*/
    void processDependencyUpdateMessage(const ActionMessage &cmd);
    /** add a federate dependency
    @return true if it was actually added, false if the federate was already present
    */
    bool addDependency(Core::federate_id_t fedID);
    /** add a dependent federate
    @return true if it was actually added, false if the federate was already present
    */
    bool addDependent(Core::federate_id_t fedID);
    /** remove a dependency
    @param fedID the identifier of the federate to remove*/
    void removeDependency(Core::federate_id_t fedID);
    /** remove a dependent
    @param fedID the identifier of the federate to remove*/
    void removeDependent(Core::federate_id_t fedID);

    /** check if entry to the executing state can be granted*/
    iteration_state checkExecEntry();
  
    /** function to enter the exec Mode
    */
    void enteringExecMode();
    
    /** generate a string with the current time status*/
    std::string printTimeStatus() const;
};
} //namespace helics
