/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "basic_core_types.hpp"

#include <vector>

namespace helics {
class ActionMessage;

/** data class containing information about inter-federate dependencies*/
class DependencyInfo {
  public:
    /**enumeration of possible states for a federate to be in regards to time request*/
    enum class time_state_t : int16_t {
        initialized = 0,
        exec_requested_iterative = 1,
        exec_requested = 2,
        time_granted = 3,
        time_requested_iterative = 4,
        time_requested = 5,
        error = 7
    };

    global_federate_id fedID{}; //!< identifier for the dependency
    global_federate_id minFed{}; //!< identifier for the min dependency
    time_state_t time_state{time_state_t::initialized}; //!< the current state of the dependency
    bool cyclic{
        false}; //!< indicator that the dependency is cyclic and should be reset more completely on grant
    // 5 byte gap here
    Time Tnext{negEpsilon}; //!< next possible message or value
    Time Te{timeZero}; //!< the next currently scheduled event
    Time Tdemin{timeZero}; //!< min dependency event time
    Time forwardEvent{Time::maxVal()}; //!< a predicted event
    /** default constructor*/
    DependencyInfo() = default;
    /** construct from a federate id*/
    explicit DependencyInfo(global_federate_id id): fedID(id){};

    /** process a dependency related message
    @param m  a reference to an action message that contains some instructions for modifying dependencies
    @return the results of processing the message*/
    bool ProcessMessage(const ActionMessage& m);
};

/** class for managing a set of dependencies*/
class TimeDependencies {
  private:
    std::vector<DependencyInfo> dependencies; //!< container
  public:
    /** default constructor*/
    TimeDependencies() = default;
    /** return true if the given federate is already a member*/
    bool isDependency(global_federate_id ofed) const;
    /** insert a dependency into the structure
    @return true if the dependency was added, false if it existed already
    */
    bool addDependency(global_federate_id id);
    /** remove  dependency from consideration*/
    void removeDependency(global_federate_id id);
    /** update the info about a dependency based on a message*/
    bool updateTime(const ActionMessage& m);
    /** get the number of dependencies*/
    auto size() const { return dependencies.size(); }
    /** iterator to first dependency*/
    auto begin() { return dependencies.begin(); }
    /** iterator to end point*/
    auto end() { return dependencies.end(); }
    /**  const iterator to first dependency*/
    auto begin() const { return dependencies.cbegin(); }
    /** const iterator to end point*/
    auto end() const { return dependencies.cend(); }
    /**  const iterator to first dependency*/
    auto cbegin() const { return dependencies.cbegin(); }
    /**  const iterator to first dependency*/
    auto cend() const { return dependencies.cend(); }

    /** get a pointer to the dependency information for a particular object*/
    const DependencyInfo* getDependencyInfo(global_federate_id id) const;

    /** get a pointer to the dependency information for a particular object*/
    DependencyInfo* getDependencyInfo(global_federate_id id);

    /** check if the dependencies would allow entry to exec mode*/
    bool checkIfReadyForExecEntry(bool iterating) const;
    /** check if the dependencies would allow a grant of the time
    @param iterating true if the object is iterating
    @param desiredGrantTime  the time to check for granting
    @return true if the object is ready
    */
    bool checkIfReadyForTimeGrant(bool iterating, Time desiredGrantTime) const;
    /** reset the iterative exec requests to prepare for the next iteration*/
    void resetIteratingExecRequests();
    /** reset iterative time requests to prepare for next iteration
    @param requestTime  the time that is being iterated*/
    void resetIteratingTimeRequests(Time requestTime);
    /** reset the tdeMin */
    void resetDependentEvents(Time grantTime);
    /** check if there are active dependencies*/
    bool hasActiveTimeDependencies() const;
};
} // namespace helics
