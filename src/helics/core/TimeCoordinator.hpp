/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "ActionMessage.hpp"
#include "CoreFederateInfo.hpp"
#include "TimeDependencies.hpp"
#include <atomic>
#include <functional>

namespace helics
{
/** class managing the coordination of time in HELICS
the time coordinator manages dependencies and computes whether time can advance or enter execution mode
*/
class TimeCoordinator
{
  private:
	// the variables for time coordination
    Time time_granted = Time::minVal ();  //!< the most recent time granted
    Time time_requested = Time::maxVal ();  //!< the most recent time requested
	Time time_next = timeZero;  //!< the next possible internal event time
	Time time_minminDe = timeZero;  //!< the minimum  of the minimum dependency event Time
	Time time_minDe = timeZero;  //!< the minimum event time of the dependencies
    Time time_allow = Time::minVal ();  //!< the current allowable time
    Time time_exec = Time::maxVal ();  //!< the time of the next targeted execution
    Time time_message = Time::maxVal ();  //!< the time of the earliest message event
    Time time_value = Time::maxVal ();  //!< the time of the earliest value event
    Time time_grantBase = Time::minVal();  //!< time to use as a basis for calculating the next grantable time(usually time granted unless values are changing)
	TimeDependencies dependencies;  //!< federates which this Federate is temporally dependent on
	std::vector<Core::federate_id_t> dependents;  //!< federates which temporally depend on this federate

	CoreFederateInfo info;  //!< basic federate info the core uses
	std::function<void(const ActionMessage &)> sendMessageFunction;  //!< callback used to send the messages

  public:
    Core::federate_id_t
      source_id;  //!< the identifier for inserting into the source id field of any generated messages;
    bool iterating = false;  //!< indicator that the coordinator should be iterating if need be
	bool checkingExec = false; //!< flag indicating that the coordinator is trying to enter the exec mode
	bool executionMode = false;	//!< flag that the coordinator has entered the execution Mode
    bool hasInitUpdates =
      false;  //!< flag indicating that a value or message was received during initialization stage
  private:
    std::atomic<int32_t> iteration{0};  //!< iteration counter
public:
    bool forwarding = false; //indicator that the time coordinator is a forwarding coordinator
  public:
    TimeCoordinator () = default;
    explicit TimeCoordinator (const CoreFederateInfo &info_);

	/* get the federate info used by the Core that affects timing*/
	CoreFederateInfo &getFedInfo()
	{
		return info;
	}
	/** get the core federate info in const setting*/
    const CoreFederateInfo &getFedInfo () const { return info; }
	/** set the core information using for timing as a block*/
    void setInfo (const CoreFederateInfo &info_) { info = info_; }
	/** set the callback function used for the sending messages*/
	void setMessageSender(std::function<void(const ActionMessage &)> sendMessageFunction_)
	{
		sendMessageFunction = std::move(sendMessageFunction_);
	}

	/** get the current granted time*/
	Time getGrantedTime() const
	{
		return time_granted;
	}
	/** get a list of actual dependencies*/
    std::vector < Core::federate_id_t> getDependencies() const;
    /** get a reference to the dependents vector*/
    const std::vector<Core::federate_id_t> &getDependents () const { return dependents; }
	/** get the current iteration counter for an iterative call
	@details this will work properly even when a federate is processing
	*/
    int32_t getCurrentIteration () const { return iteration; }
	/** compute updates to time values
	@return true if they have been modified
	*/
    bool updateTimeFactors ();
	/** update the time_value variable with a new value if needed
	*/
    void updateValueTime (Time valueUpdateTime);
	/** update the time_message variable with a new value if needed
	*/
    void updateMessageTime (Time messageUpdateTime);

	/** take a global id and get a pointer to the dependencyInfo for the other fed
	will be nullptr if it doesn't exist
	*/
    DependencyInfo *getDependencyInfo (Core::federate_id_t ofed);
	/** check whether a federate is a dependency*/
    bool isDependency (Core::federate_id_t ofed) const;

  private:
	/** helper function for computing the next event time*/
    bool updateNextExecutionTime ();
	/** helper function for computing the next possible time to generate an external event
	*/
    void updateNextPossibleEventTime ();
    /** get the next possible time that a time coordinator could grant*/
    Time getNextPossibleTime() const;
    Time generateAllowedTime(Time testTime) const;

    void sendTimeRequest() const;
    void updateTimeGrant();
  public:
	/** process a message related to time
	@return true if it did anything
	*/
    bool processTimeMessage (const ActionMessage &cmd);

    /** process a message related to configuration
    @param cmd the update command
    @param initMode set to true to allow initialization mode only updates
    */
    void processConfigUpdateMessage (const ActionMessage &cmd, bool initMode = false);
    /** process a dependency update message*/
    void processDependencyUpdateMessage (const ActionMessage &cmd);
	/** add a federate dependency
	@return true if it was actually added, false if the federate was already present
	*/
    bool addDependency (Core::federate_id_t fedID);
	/** add a dependent federate
	@return true if it was actually added, false if the federate was already present
	*/
    bool addDependent (Core::federate_id_t fedID);
	/** remove a dependency
	@param fedID the identifier of the federate to remove*/
	void removeDependency(Core::federate_id_t fedID);
	/** remove a dependent
	@param fedID the identifier of the federate to remove*/
	void removeDependent(Core::federate_id_t fedID);

	/** check if entry to the executing state can be granted*/
    iteration_state checkExecEntry ();
	/** request a time
	@param nextTime the new requested time
	@param iterate the mode of iteration to use (no_iteration, force_iteration, iterate_if_needed)
	@param newValueTime  the time of the next value
	@param newMessageTime the time of the next message
	*/
	void timeRequest(Time nextTime, helics_iteration_request iterate, Time newValueTime, Time newMessageTime);
	/** function to enter the exec Mode
	@param mode the mode of iteration_request (no_iteration, force_iteration, iterate_if_needed)
	*/
	void enteringExecMode(helics_iteration_request mode);
	/** check if it is valid to grant a time*/
    iteration_state checkTimeGrant ();
    /** generate a string with the current time status*/
    std::string printTimeStatus () const;
    /** return true if there are active dependencies*/
    bool hasActiveTimeDependencies() const;
};
} //namespace helics

