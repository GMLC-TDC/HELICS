/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef TIME_DEPENDENCIES_H_
#define TIME_DEPENDENCIES_H_
#pragma once

#include "core.h"
#include "helics-time.h"

namespace helics
{
class ActionMessage;
/** data class containing information about interfederate dependencies*/
class DependencyInfo
{
  public:
    /**enumeration of possible states for a federate to be in regards to time request*/
    enum class time_state_t : int
    {
        initialized = 0,
        exec_requested_iterative = 1,
        exec_requested = 2,
        time_granted = 3,
        time_requested_iterative = 4,
        time_requested = 5,
    };
    Core::federate_id_t fedID = invalid_fed_id;  //!< identifier for the dependency
    time_state_t time_state = time_state_t::initialized;  //!< the current state of the dependency
    Time Tnext = timeZero;  //!<next possible message or value
    Time Te = timeZero;  //!< the next currently scheduled event
    Time Tdemin = timeZero;  //!< min dependency event time
    /** default constructor*/
    DependencyInfo () = default;
    /** construct from a federate id*/
    DependencyInfo (Core::federate_id_t id) : fedID (id){};

    bool ProcessMessage (const ActionMessage &m);
};

/** class for managing a set of dependencies*/
class TimeDependencies
{
  private:
    std::vector<DependencyInfo> dependencies;  //!< container
  public:
    /** default constructor*/
    TimeDependencies () = default;
    /** return true if the given federate is already a member*/
    bool isDependency (Core::federate_id_t ofed) const;
    /** insert a dependency into the structure
    @return true if the dependency was added, false if it existed already
    */
    bool addDependency (Core::federate_id_t id);
    /** remove  dependency from consideration*/
    void removeDependency (Core::federate_id_t id);
    /** update the info about a dependency based on a message*/
    bool updateTime (const ActionMessage &m);
    /** iterator to first dependency*/
    auto begin () { return dependencies.begin (); }
    /** iterator to end point*/
    auto end () { return dependencies.end (); }
    /**  const iterator to first dependency*/
    auto begin () const { return dependencies.cbegin (); }
    /** const iterator to end point*/
    auto end () const { return dependencies.cend (); }
    /**  const iterator to first dependency*/
    auto cbegin () const { return dependencies.cbegin (); }
    /**  const iterator to first dependency*/
    auto cend () const { return dependencies.cend (); }

    DependencyInfo *getDependencyInfo (Core::federate_id_t id);

    bool checkIfReadyForExecEntry (bool iterating) const;
    bool checkIfReadyForTimeGrant (bool iterating, Time desiredGrantTime) const;
    void ResetIteratingExecRequests ();
    void ResetIteratingTimeRequests (Time requestTime);
};
}
#endif  // DEPENDENCY_INFO_H_