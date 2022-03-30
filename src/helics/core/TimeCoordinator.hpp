/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../common/GuardedTypes.hpp"
#include "ActionMessage.hpp"
#include "TimeDependencies.hpp"

#include "json/forwards.h"
#include <atomic>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace helics {

/** enumeration of possible processing results*/
enum class message_process_result {
    no_effect = 0,  //!< the message did not result in an update
    processed,  //!< the message was used to update the current state
    delay_processing,  //!< the message should be delayed and reprocessed later
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
    /** have the shown event time match dependency events for use with federates
    that trigger on events but don't have internal generated events*/
    bool event_triggered = false;
    int maxIterations = 50;
};

/** class managing the coordination of time in HELICS
the time coordinator manages dependencies and computes whether time can advance or enter execution
mode
*/
class TimeCoordinator {
  protected:
    /// the variables for time coordination
    TimeData upstream;
    TimeData total;
    mutable TimeData lastSend;
    // the variables for time coordination
    Time time_granted{Time::minVal()};  //!< the most recent time granted
    Time time_requested{Time::maxVal()};  //!< the most recent time requested
    Time time_next{timeZero};  //!< the next possible internal event time
    Time time_minminDe{timeZero};  //!< the minimum  of the minimum dependency event Time
    Time time_minDe{timeZero};  //!< the minimum event time of the dependencies
    Time time_allow{Time::minVal()};  //!< the current allowable time
    Time time_exec{Time::maxVal()};  //!< the time of the next targeted execution
    Time time_message{Time::maxVal()};  //!< the time of the earliest message event
    Time time_value{Time::maxVal()};  //!< the time of the earliest value event
    /** time to use as a basis for calculating the next grantable
    time(usually time granted unless values are changing) */
    Time time_grantBase{Time::minVal()};
    /// a blocking time to not grant time >= the specified time
    Time time_block{Time::maxVal()};
    Time maxTime{Time::maxVal()};
    /// these are to maintain an accessible record of dependent federates
    shared_guarded_m<std::vector<GlobalFederateId>> dependent_federates;
    /// these are to maintain an accessible record of dependency federates
    shared_guarded_m<std::vector<GlobalFederateId>> dependency_federates;
    /// federates which this Federate is temporally dependent on
    TimeDependencies dependencies;
    /// blocks for a particular timeblocking link
    std::vector<std::pair<Time, int32_t>> timeBlocks;
    /// basic time control information
    tcoptions info;
    std::uint8_t currentRestrictionLevel{0};
    /// callback used to send the messages
    std::function<void(const ActionMessage&)> sendMessageFunction;

  public:
    /// the identifier for inserting into the source id field of any generated messages;
    GlobalFederateId source_id{0};
    /// indicator that the coordinator should be iteratingif need be
    IterationRequest iterating{IterationRequest::NO_ITERATIONS};
    /// flag indicating that the coordinator is trying to enter the exec mode
    bool checkingExec{false};
    /// flag that the coordinator has entered the execution Mode
    bool executionMode{false};
    /// flag indicating that a value or message was received during initialization stage
    bool hasInitUpdates{false};
    /// flag indicating that we need to send updates to all dependencies on receipt of addition
    /// request
    bool needSendAll{false};

  protected:
    bool disconnected{false};
    /// specify that the timeCoordinator should not grant times and instead operate in a continuous
    /// manner until completion
    bool nonGranting{false};
    /// if set to true the time coordinator is joining an ongoing co-simulation
    bool dynamicJoining{false};

    std::atomic<int32_t> iteration{0};  //!< current number of iterations
    int32_t sequenceCounter{0};  //!< sequence counter for tracking responses

  public:
    /** default constructor*/
    TimeCoordinator();
    /** construct from a federate info and message send function*/
    explicit TimeCoordinator(std::function<void(const ActionMessage&)> userSendMessageFunction);

    /** set a timeProperty for a the coordinator*/
    void setProperty(int timeProperty, Time propertyVal);
    /** set a timeProperty for a the coordinator*/
    void setProperty(int intProperty, int propertyVal);
    /** set an option Flag for a the coordinator*/
    void setOptionFlag(int optionFlag, bool value);
    /** set dynamic joining flag to true*/
    void setDynamicJoining() { dynamicJoining = true; }

    /** get a time Property*/
    Time getTimeProperty(int timeProperty) const;
    /** get an option flag value*/
    bool getOptionFlag(int optionFlag) const;
    /** get an option flag value*/
    int getIntegerProperty(int intProperty) const;
    /** set the callback function used for the sending messages*/
    void setMessageSender(std::function<void(const ActionMessage&)> userSendMessageFunction);

    /** get the current granted time*/
    Time getGrantedTime() const { return time_granted; }
    /** get the current granted time*/
    Time allowedSendTime() const { return time_granted + info.outputDelay; }
    /** get a list of actual dependencies*/
    std::vector<GlobalFederateId> getDependencies() const;
    /** get a reference to the dependents vector*/
    std::vector<GlobalFederateId> getDependents() const
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
    if allowed it will send an updated time request message
     */
    void updateValueTime(Time valueUpdateTime, bool allowRequestSend);
    /** update the time_message variable with a new value if needed
    if allowed it will send an updated time request message
     */
    void updateMessageTime(Time messageUpdateTime, bool allowRequestSend);

    void specifyNonGranting(bool value = true) { nonGranting = value; }
    /** get the id of the federate that has the earliest time dependency*/
    GlobalFederateId getMinDependency() const;

  private:
    /** take a global id and get a pointer to the dependencyInfo for the other fed
    will be nullptr if it doesn't exist
    */
    DependencyInfo* getDependencyInfo(GlobalFederateId ofed);
    /** check whether a federate is a dependency*/
    bool isDependency(GlobalFederateId ofed) const;

  private:
    /** helper function for computing the next event time*/
    bool updateNextExecutionTime();
    /** helper function for computing the next possible time to generate an external event
     */
    void updateNextPossibleEventTime();
    /** get the next possible time that a time coordinator could grant*/
    Time getNextPossibleTime() const;
    Time generateAllowedTime(Time testTime) const;
    /* return true if the skip federate was detected*/
    bool checkAndSendTimeRequest(ActionMessage& upd, GlobalFederateId skip) const;

    void sendTimeRequest() const;
    void updateTimeGrant();
    /** transmit message to all federates except the skipFed,  return true if skipFed was used*/
    bool transmitTimingMessages(ActionMessage& msg,
                                GlobalFederateId skipFed = GlobalFederateId{}) const;

    message_process_result processTimeBlockMessage(const ActionMessage& cmd);

    Time updateTimeBlocks(int32_t blockId, Time newTime);

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
    /** set a federate to be a child in timekeeping hierarchy*/
    void setAsChild(GlobalFederateId fedID);
    /** set a federate/broker to be the parent in the timekeeping hierarchy*/
    void setAsParent(GlobalFederateId fedID);
    /** get the id of a federate acting as a parent for timekeeping*/
    GlobalFederateId getParent() const;

    /** check if entry to the executing state can be granted*/
    MessageProcessingResult checkExecEntry(GlobalFederateId triggerFed = GlobalFederateId{});

    /** send updated exec request to target or everyone if target is invalid*/
    void sendUpdatedExecRequest(GlobalFederateId target = GlobalFederateId{},
                                GlobalFederateId minFed = GlobalFederateId{},
                                std::int32_t responseSequenceCounter = 0);
    /** request a time
    @param nextTime the new requested time
    @param iterate the mode of iteration to use (no_iteration, FORCE_ITERATION, ITERATE_IF_NEEDED)
    @param newValueTime  the time of the next value
    @param newMessageTime the time of the next message
    */
    void timeRequest(Time nextTime,
                     IterationRequest iterate,
                     Time newValueTime,
                     Time newMessageTime);
    /** function to enter the exec Mode
    @param mode the mode of iteration_request (no_iteration, FORCE_ITERATION, ITERATE_IF_NEEDED)
    */
    void enteringExecMode(IterationRequest mode);
    /** check if it is valid to grant a time*/
    MessageProcessingResult checkTimeGrant();
    /** disconnect*/
    void disconnect();
    /** generate a local Error*/
    void localError();
    /** generate a string with the current time status*/
    std::string printTimeStatus() const;
    /** return true if there are active dependencies*/
    bool hasActiveTimeDependencies() const;
    /** generate a configuration string(JSON)*/
    void generateConfig(Json::Value& base) const;

    /** generate debugging time information*/
    void generateDebuggingTimeInfo(Json::Value& base) const;

    /** get a count of the active dependencies*/
    int dependencyCount() const;
    /** get a count of the active dependencies*/
    std::pair<GlobalFederateId, Time> getMinGrantedDependency() const;
    /** enter initialization*/
    void enterInitialization();
    /** request a resend of the time message for certain federates currently blocking*/
    void requestTimeCheck();
};
}  // namespace helics
