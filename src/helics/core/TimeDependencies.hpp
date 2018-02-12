/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef TIME_DEPENDENCIES_H_
#define TIME_DEPENDENCIES_H_
#pragma once

#include "Core.hpp"
#include "helics-time.hpp"
#include <vector>

namespace helics
{
class ActionMessage;
/** data class containing information about inter-federate dependencies*/
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
    Time Tnext = negEpsilon;  //!< next possible message or value
    Time Te = timeZero;  //!< the next currently scheduled event
    Time Tdemin = timeZero;  //!< min dependency event time
    /** default constructor*/
    DependencyInfo () = default;
    /** construct from a federate id*/
    DependencyInfo (Core::federate_id_t id) : fedID (id){};

	/** process a dependency related message
	@param m  a reference to an action message that contains some instructions for modifying dependencies
	@return true if something was modified by the message*/
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

	/** get a pointer to the dependency information for a particular object*/
    DependencyInfo *getDependencyInfo (Core::federate_id_t id);

	/** check if the dependencies would allow entry to exec mode*/
    bool checkIfReadyForExecEntry (bool iterating) const;
	/** check if the dependencies would allow a grant of the time
	@param iterating true if the object is iterating
	@param desiredGrantTime  the time to check for granting
	@return true if the object is ready
	*/
    bool checkIfReadyForTimeGrant (bool iterating, Time desiredGrantTime) const;
    void resetIteratingExecRequests ();
    void resetIteratingTimeRequests (Time requestTime);
};
}
#endif  // DEPENDENCY_INFO_H_