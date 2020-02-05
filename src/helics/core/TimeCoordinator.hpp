/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../common/GuardedTypes.hpp"
#include "ActionMessage.hpp"
#include "TimeDependencies.hpp"

#include <atomic>
#include <deque>
#include <functional>

namespace helics {
/** enumeration of possible processing results*/
enum class message_process_result {
    no_effect = 0, //!< the message did not result in an update
    processed, //!< the message was used to update the current state
    delay_processing, //!< the message should be delayed and reprocessed later
};

/** class for the controlling fields and options for a time coordinator*/
class tcoptions {
  public:
    Time timeDelta = Time::epsilon();
    Time inputDelay = timeZero;
    Time outputDelay = timeZero;
    Time offset = timeZero;
    Time period = timeZero;
    // Time rtLag = timeZero;
    // Time rtLead = timeZero;
    // bool observer = false;
    // bool realtime = false;
    // bool source_only = false;
    bool wait_for_current_time_updates = false;
    bool uninterruptible = false;
    bool restrictive_time_policy = false;
    // 1 byte gap
    int maxIterations = 50;
};

/** class managing the coordination of time in HELICS
the time coordinator manages dependencies and computes whether time can advance or enter execution mode
*/
class TimeCoordinator {
  private:
    // the variables for time coordination
    Time time_granted = Time::minVal(); //!< the most recent time granted
    Time time_requested = Time::maxVal(); //!< the most recent time requested
    Time time_next = timeZero; //!< the next possible internal event time
    Time time_minminDe = timeZero; //!< the minimum  of the minimum dependency event Time
    Time time_minDe = timeZero; //!< the minimum event time of the dependencies
    Time time_allow = Time::minVal(); //!< the current allowable time
    Time time_exec = Time::maxVal(); //!< the time of the next targeted execution
    Time time_message = Time::maxVal(); //!< the time of the earliest message event
    Time time_value = Time::maxVal(); //!< the time of the earliest value event
    Time time_grantBase =
        Time::minVal(); //!< time to use as a basis for calculating the next grantable
    //!< time(usually time granted unless values are changing)
    Time time_block = Time::maxVal(); //!< a blocking time to not grant time >= the specified time
    shared_guarded_m<std::vector<global_federate_id>>
        dependent_federates; //!< these are to maintain an accessible record of dependent federates
    shared_guarded_m<std::vector<global_federate_id>>
        dependency_federates; //!< these are to maintain an accessible record of dependency federates
    TimeDependencies dependencies; //!< federates which this Federate is temporally dependent on
    std::vector<global_federate_id>
        dependents; //!< federates which temporally depend on this federate
    std::deque<std::pair<Time, int32_t>> timeBlocks; //!< blocks for a particular timeblocking link
    tcoptions info; //!< basic time control information
    std::function<void(const ActionMessage&)>
        sendMessageFunction; //!< callback used to send the messages

  public:
    global_federate_id source_id{
        0}; //!< the identifier for inserting into the source id field of any generated messages;
    bool iterating{false}; //!< indicator that the coordinator should be iterating if need be
    bool checkingExec{
        false}; //!< flag indicating that the coordinator is trying to enter the exec mode
    bool executionMode{false}; //!< flag that the coordinator has entered the execution Mode
    bool hasInitUpdates{
        false}; //!< flag indicating that a value or message was received during initialization stage
  private:
    std::atomic<int32_t> iteration{0}; //!< iteration counter
    bool disconnected{false};

  public:
    /** default constructor*/
    TimeCoordinator();
    /** construct from a federate info and message send function*/
    explicit TimeCoordinator(std::function<void(const ActionMessage&)> sendMessageFunction_);

    /** set a timeProperty for a the coordinator*/
    void setProperty(int timeProperty, Time propertyVal);
    /** set a timeProperty for a the coordinator*/
    void setProperty(int intProperty, int propertyVal);
    /** set an option Flag for a the coordinator*/
    void setOptionFlag(int optionFlag, bool value);
    /** get a time Property*/
    Time getTimeProperty(int timeProperty) const;
    /** get an option flag value*/
    bool getOptionFlag(int optionFlag) const;
    /** get an option flag value*/
    int getIntegerProperty(int intProperty) const;
    /** set the callback function used for the sending messages*/
    void setMessageSender(std::function<void(const ActionMessage&)> sendMessageFunction_);

    /** get the current granted time*/
    Time getGrantedTime() const { return time_granted; }
    /** get the current granted time*/
    Time allowedSendTime() const { return time_granted + info.outputDelay; }
    /** get a list of actual dependencies*/
    std::vector<global_federate_id> getDependencies() const;
    /** get a reference to the dependents vector*/
    std::vector<global_federate_id> getDependents() const
    {
        return *dependent_federates.lock_shared();
    }
    /** get the current iteration counter for an iterative call
    @details this will work properly even when a federate is processing
    */
    int32_t getCurrentIteration() const { return iteration.load(); }
    /** compute updates to time values
    @return true if they have been modified
    */
    bool updateTimeFactors();
    /** update the time_value variable with a new value if needed
     */
    void updateValueTime(Time valueUpdateTime);
    /** update the time_message variable with a new value if needed
     */
    void updateMessageTime(Time messageUpdateTime);

  private:
    /** take a global id and get a pointer to the dependencyInfo for the other fed
    will be nullptr if it doesn't exist
    */
    DependencyInfo* getDependencyInfo(global_federate_id ofed);
    /** check whether a federate is a dependency*/
    bool isDependency(global_federate_id ofed) const;

  private:
    /** helper function for computing the next event time*/
    bool updateNextExecutionTime();
    /** helper function for computing the next possible time to generate an external event
     */
    void updateNextPossibleEventTime();
    /** get the next possible time that a time coordinator could grant*/
    Time getNextPossibleTime() const;
    Time generateAllowedTime(Time testTime) const;

    void sendTimeRequest() const;
    void updateTimeGrant();
    void transmitTimingMessage(ActionMessage& msg) const;

    message_process_result processTimeBlockMessage(const ActionMessage& cmd);

  public:
    /** process a message related to time
    @return the result of processing the message
    */
    message_process_result processTimeMessage(const ActionMessage& cmd);

    /** process a message related to configuration
    @param cmd the update command
    */
    void processConfigUpdateMessage(const ActionMessage& cmd);
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

    /** check if entry to the executing state can be granted*/
    message_processing_result checkExecEntry();
    /** request a time
    @param nextTime the new requested time
    @param iterate the mode of iteration to use (no_iteration, force_iteration, iterate_if_needed)
    @param newValueTime  the time of the next value
    @param newMessageTime the time of the next message
    */
    void timeRequest(
        Time nextTime,
        iteration_request iterate,
        Time newValueTime,
        Time newMessageTime);
    /** function to enter the exec Mode
    @param mode the mode of iteration_request (no_iteration, force_iteration, iterate_if_needed)
    */
    void enteringExecMode(iteration_request mode);
    /** check if it is valid to grant a time*/
    message_processing_result checkTimeGrant();
    /** disconnect*/
    void disconnect();
    /** generate a string with the current time status*/
    std::string printTimeStatus() const;
    /** return true if there are active dependencies*/
    bool hasActiveTimeDependencies() const;
    /** generate a configuration string(JSON)*/
    std::string generateConfig() const;
};
} // namespace helics
