/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "basic_core_types.hpp"

#include "json/forwards.h"
#include <vector>

namespace helics {
class ActionMessage;

/**enumeration of possible states for a federate to be in regards to time request*/
enum class time_state_t : uint8_t {
    initialized = 0,
    exec_requested_iterative = 1,
    exec_requested = 2,
    time_granted = 3,
    time_requested_iterative = 4,
    time_requested = 5,
    error = 7
};

enum class ConnectionType : uint8_t {
    independent = 0,
    parent = 1,
    child = 2,
    self = 3,
    none = 4,
};
// helper class containing the basic timeData
class TimeData {
  public:
    Time next{negEpsilon};  //!< next possible message or value
    Time Te{timeZero};  //!< the next currently scheduled event
    Time minDe{timeZero};  //!< min dependency event time
    Time TeAlt{timeZero};  //!< the second min event
    global_federate_id minFed{};  //!< identifier for the min dependency
    global_federate_id minFedActual{};  //!< the actual forwarded minimum federate object
    time_state_t time_state{time_state_t::initialized};

    TimeData() = default;
    explicit TimeData(Time start): next{start}, Te{start}, minDe{start}, TeAlt{start} {};
    /** check if there is an update to the current dependency info and assign*/
    bool update(const TimeData& update);
};

/** data class containing information about inter-federate dependencies*/
class DependencyInfo: public TimeData {
  public:
    global_federate_id fedID{};  //!< identifier for the dependency

    bool cyclic{false};  //!< indicator that the dependency is cyclic and should be reset more
                         //!< completely on grant
    ConnectionType connection{ConnectionType::independent};
    bool dependent{false};  //!< indicator the dependency is a dependent object
    bool dependency{false};  //!< indicator that the dependency is an actual dependency
    bool forwarding{false};  //!< indicator that the dependency is a forwarding time coordinator
    bool nonGranting{false};  //!< indicator that the dependency is a non granting time coordinator
    // Time forwardEvent{Time::maxVal()};  //!< a predicted event
    /** default constructor*/
    DependencyInfo() = default;
    /** construct from a federate id*/
    explicit DependencyInfo(global_federate_id id): fedID(id), forwarding{id.isBroker()} {}

    explicit DependencyInfo(Time start): TimeData(start) {}
};

/** class for managing a set of dependencies*/
class TimeDependencies {
  private:
    std::vector<DependencyInfo> dependencies;  //!< container
  public:
    /** default constructor*/
    TimeDependencies() = default;
    /** return true if the given federate is already a dependency*/
    bool isDependency(global_federate_id ofed) const;
    /** return true if the given federate is already a dependent*/
    bool isDependent(global_federate_id ofed) const;
    /** insert a dependency into the structure
    @return true if the dependency was added, false if it existed already
    */
    bool addDependency(global_federate_id id);
    /** remove  dependency from consideration*/
    void removeDependency(global_federate_id id);
    /** update the info about a dependency based on a message*/
    bool addDependent(global_federate_id id);
    /** remove  dependent from consideration*/
    void removeDependent(global_federate_id id);
    /** remove an interdependency from consideration*/
    void removeInterdependence(global_federate_id id);
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

    /**  check if there are no dependencies*/
    bool empty() const { return dependencies.empty(); }
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
    /** get a count of the active dependencies*/
    int activeDependencyCount() const;
    /** get a count of the active dependencies*/
    global_federate_id getMinDependency() const;

    void setDependencyVector(const std::vector<DependencyInfo>& deps) { dependencies = deps; }
};

TimeData generateMinTimeUpstream(const TimeDependencies& dependencies,
                                 bool restricted,
                                 global_federate_id self,
                                 global_federate_id ignore = global_federate_id());

TimeData generateMinTimeDownstream(const TimeDependencies& dependencies,
                                   bool restricted,
                                   global_federate_id self,
                                   global_federate_id ignore = global_federate_id());

TimeData generateMinTimeTotal(const TimeDependencies& dependencies,
                              bool restricted,
                              global_federate_id self,
                              global_federate_id ignore = global_federate_id());

void generateJsonOutputTimeData(Json::Value& output,
                                const TimeData& dep,
                                bool includeAggregates = true);

void generateJsonOutputDependency(Json::Value& output, const DependencyInfo& dep);
}  // namespace helics
