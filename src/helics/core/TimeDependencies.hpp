/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "basic_CoreTypes.hpp"
#include "nlohmann/json_fwd.hpp"

#include <string>
#include <utility>
#include <vector>

namespace helics {
class ActionMessage;

/**enumeration of possible states for a federate to be in regards to time request*/
enum class TimeState : std::uint8_t {
    initialized = 0,
    exec_requested_require_iteration = 1,
    exec_requested_iterative = 2,
    exec_requested = 3,
    time_granted = 5,
    time_requested_require_iteration = 6,
    time_requested_iterative = 7,
    time_requested = 8,
    error = 10
};

/** enumeration of the different connection arrangements*/
enum class ConnectionType : std::uint8_t {
    INDEPENDENT = 0,
    PARENT = 1,
    CHILD = 2,
    SELF = 3,
    NONE = 4,
};

/** enumeration of the possible time message processing results*/
enum class TimeProcessingResult : std::uint8_t {
    NOT_PROCESSED = 0,  //!< the message did not result in an update
    PROCESSED = 1,  //!< the message was used to update the current state
    /// the message was used to update the current state and additional checks should be made
    PROCESSED_AND_CHECK = 2,
    /// the message was used to update the current state and a new request was made
    PROCESSED_NEW_REQUEST = 3,
    DELAY_PROCESSING = 5  //!< the message should be delayed and reprocessed later
};

/** enumeration of delay modes which affect whether a time is granted or not*/
enum class GrantDelayMode : std::uint8_t { NONE = 0, INTERRUPTED = 1, WAITING = 2 };

inline GrantDelayMode getDelayMode(bool waiting, bool interrupted)
{
    return waiting ? GrantDelayMode::WAITING :
                     (interrupted ? GrantDelayMode::INTERRUPTED : GrantDelayMode::NONE);
}

// helper class containing the basic timeData
class TimeData {
  public:
    Time next{negEpsilon};  //!< next possible message or value
    Time Te{timeZero};  //!< the next currently scheduled event
    Time minDe{timeZero};  //!< min dependency event time
    Time TeAlt{timeZero};  //!< the second min event
    Time lastGrant{timeZero};  //!< storage for the previous grant time
    GlobalFederateId minFed{};  //!< identifier for the min dependency
    GlobalFederateId minFedActual{};  //!< the actual forwarded minimum federate object
    TimeState mTimeState{TimeState::initialized};
    bool hasData{false};  //!< indicator that data was sent in the current interval
    bool interrupted{false};  //!< indicator that the federates next event is a timing interruption
    bool delayedTiming{false};  //!< indicator that the dependency is using delayed timing
    std::int8_t timingVersion{-2};  //!< version indicator
    std::uint8_t restrictionLevel{0};  //!< timing restriction level

    std::int32_t timeoutCount{0};  //!< counter for timeout checking
    std::int32_t sequenceCounter{0};  //!< the sequence Counter of the request
    std::int32_t responseSequenceCounter{0};  //!< the iteration count of the min federate
    /// the iteration of the dependency when the local iteration was granted
    std::int32_t grantedIteration{0};
    TimeData() = default;
    TimeData(const TimeData&) = default;
    TimeData& operator=(const TimeData&) = default;
    TimeData(TimeData&&) = default;
    TimeData& operator=(TimeData&&) = default;
    explicit TimeData(Time start,
                      TimeState startState = TimeState::initialized,
                      std::uint8_t resLevel = 0U):
        next{start}, Te{start}, minDe{start}, TeAlt{start}, mTimeState{startState},
        restrictionLevel{resLevel} {};
    /** check if there is an update to the current dependency info and assign*/
    bool update(const TimeData& update);
};

/** data class containing information about inter-federate dependencies*/
class DependencyInfo: public TimeData {
  public:
    GlobalFederateId fedID{};  //!< identifier for the dependency

    bool cyclic{false};  //!< indicator that the dependency is cyclic and should be reset more
                         //!< completely on grant
    ConnectionType connection{ConnectionType::INDEPENDENT};
    bool dependent{false};  //!< indicator the dependency is a dependent object
    bool dependency{false};  //!< indicator that the dependency is an actual dependency
    bool forwarding{false};  //!< indicator that the dependency is a forwarding time coordinator
    bool nonGranting{false};  //!< indicator that the dependency is a non granting time coordinator
    bool triggered{false};  //!< indicator that the dependency has been triggered in some way
    bool updateRequested{false};  //!< indicator that an update request is in process
    // Time forwardEvent{Time::maxVal()};  //!< a predicted event
    /** default constructor*/
    DependencyInfo() = default;
    DependencyInfo(const DependencyInfo&) = default;
    DependencyInfo& operator=(const DependencyInfo&) = default;
    DependencyInfo(DependencyInfo&&) = default;
    DependencyInfo& operator=(DependencyInfo&&) = default;
    /** construct from a federate id*/
    explicit DependencyInfo(GlobalFederateId id): fedID(id), forwarding{id.isBroker()} {}

    template<class... Args>
    explicit DependencyInfo(Time start, Args&&... args):
        TimeData(start, std::forward<Args>(args)...)
    {
    }
};

/** class for managing a set of dependencies*/
class TimeDependencies {
  private:
    std::vector<DependencyInfo> dependencies;  //!< container
    mutable GlobalFederateId mDelayedDependency{};

  public:
    /** default constructor*/
    TimeDependencies() = default;
    /** return true if the given federate is already a dependency*/
    bool isDependency(GlobalFederateId ofed) const;
    /** return true if the given federate is already a dependent*/
    bool isDependent(GlobalFederateId ofed) const;
    /** insert a dependency into the structure
    @return true if the dependency was added, false if it existed already
    */
    bool addDependency(GlobalFederateId gid);
    /** remove  dependency from consideration*/
    void removeDependency(GlobalFederateId gid);
    /** update the info about a dependency based on a message*/
    bool addDependent(GlobalFederateId gid);
    /** remove  dependent from consideration*/
    void removeDependent(GlobalFederateId gid);
    /** reset a dependency for reentrant connection*/
    void resetDependency(GlobalFederateId id);
    /** remove an interdependency from consideration*/
    void removeInterdependence(GlobalFederateId gid);
    /** update the info about a dependency based on a message*/
    TimeProcessingResult updateTime(const ActionMessage& cmd);
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
    const DependencyInfo* getDependencyInfo(GlobalFederateId gid) const;

    /** get a pointer to the dependency information for a particular object*/
    DependencyInfo* getDependencyInfo(GlobalFederateId gid);

    /** check if the dependencies would allow entry to exec mode*/
    bool checkIfReadyForExecEntry(bool iterating, bool waiting) const;
    /** check if all dependencies have passed exec mode and are requesting time*/
    bool checkIfAllDependenciesArePastExec(bool iterating) const;
    /** check if the dependencies would allow a grant of the time
    @param iterating true if the object is iterating
    @param desiredGrantTime  the time to check for granting
    @return true if the object is ready
    */
    bool checkIfReadyForTimeGrant(bool iterating,
                                  Time desiredGrantTime,
                                  GrantDelayMode delayMode) const;

    /** reset the iterative exec requests to prepare for the next iteration*/
    void resetIteratingExecRequests();
    /** reset iterative time requests to prepare for next iteration
    @param requestTime  the time that is being iterated*/
    void resetIteratingTimeRequests(Time requestTime);
    /** reset the tdeMin */
    void resetDependentEvents(Time grantTime);
    /** check if there are active dependencies*/
    bool hasActiveTimeDependencies() const;
    /** verify that all the sequence Counters match*/
    bool verifySequenceCounter(Time tmin, std::int32_t sequenceCount);
    /** get a count of the active dependencies*/
    int activeDependencyCount() const;
    /** get a count of the active dependencies*/
    GlobalFederateId getMinDependency() const;

    void setDependencyVector(const std::vector<DependencyInfo>& deps) { dependencies = deps; }
    /** check the dependency set for any issues
    @return an error code and string containing an error description */
    std::pair<int, std::string> checkForIssues(bool waiting) const;

    bool hasDelayedDependency() const { return mDelayedDependency.isValid(); }
    GlobalFederateId delayedDependency() const { return mDelayedDependency; }
};

inline bool checkSequenceCounter(const DependencyInfo& dep, Time tmin, std::int32_t sq)
{
    return (!dep.dependency || !dep.dependent || dep.timingVersion <= 0 || dep.next > tmin ||
            dep.next >= cBigTime || dep.responseSequenceCounter == sq);
}

const DependencyInfo& getExecEntryMinFederate(const TimeDependencies& dependencies,
                                              GlobalFederateId self,
                                              ConnectionType ignoreType = ConnectionType::NONE,
                                              GlobalFederateId ignore = GlobalFederateId{});
static constexpr GlobalFederateId NoIgnoredFederates{};

TimeData generateMinTimeUpstream(const TimeDependencies& dependencies,
                                 bool restricted,
                                 GlobalFederateId self,
                                 GlobalFederateId ignore,
                                 std::int32_t responseCode);

TimeData generateMinTimeDownstream(const TimeDependencies& dependencies,
                                   bool restricted,
                                   GlobalFederateId self,
                                   GlobalFederateId ignore,
                                   std::int32_t responseCode);

TimeData generateMinTimeTotal(const TimeDependencies& dependencies,
                              bool restricted,
                              GlobalFederateId self,
                              GlobalFederateId ignore,
                              std::int32_t responseCode);

void generateJsonOutputTimeData(nlohmann::json& output,
                                const TimeData& dep,
                                bool includeAggregates = true);

void addTimeState(nlohmann::json& output, const TimeState state);

void generateJsonOutputDependency(nlohmann::json& output, const DependencyInfo& dep);
}  // namespace helics
