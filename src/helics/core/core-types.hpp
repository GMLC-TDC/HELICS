/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include <string>

/** @file
@details definitions of types an enumerations used in helics
*/

namespace helics
{
/** enumeration of the possible federate states*/
enum federate_state_t
{
    HELICS_CREATED, /*!> state upon creation, all registration calls are allowed*/
    HELICS_INITIALIZING,  //!< the federation has entered initialization state and initial values can be published
    HELICS_EXECUTING,  //!< the federation has entered execution state and it now advancing in time
    HELICS_ERROR,  //!< the federation has encountered an error
    HELICS_FINISHED,  //!< the federation has finished its execution
    HELICS_NONE,  //!< unknown state
};

/** the type of the cores that are available */
enum class core_type : int
{
    DEFAULT = 0,  //!< pick a core type depending on compile configuration usually either ZMQ if available or UDP
    ZMQ = 1,  //!< use the Zero MQ networking protocol
    MPI = 2,  //!< use MPI for operation on a parallel cluster
    TEST = 3,  //!< use the Test core if all federates are in the same process
    INTERPROCESS = 4,  //!< interprocess uses memory mapped files to transfer data (for use when all federates are
                       //!< on the same machine
    IPC = 5,  //!< same as INTERPROCESS
    TCP = 6,  //!< use a generic TCP protocol message stream to send messages
    UDP = 7,  //!< use UDP packets to send the data
    UNRECOGNIZED = 8,  //!< unknown

};



/** enumeration of the possible states of convergence*/
enum class iteration_state : signed char
{
    error = -5,  //!< indicator that an error has occurred
    continue_processing = -1,  //!< the current loop should continue
    next_step = 0,  //!< indicator that the iterations have completed
    iterating = 2,  //!< indicator that the iterations need to continue
    halted = 3,  //!< indicator that the simulation has been halted

};

/** enumeration of the possible states of iteration*/
enum class iteration_result : signed char
{
    error = -5,  //!< indicator that an error has occurred
    next_step = 0,  //!< indicator that the iterations have completed and the federate has moved to the next step
    iterating = 2,  //!< indicator that the iterations need to continue
    halted = 3,  //!< indicator that the simulation has been halted
};

/** enumeration of the possible iteration requests by a federate*/
enum class iteration_request : signed char
{
    no_iterations = 0,  //!< indicator that the iterations have completed
    force_iteration = 1,  //!< force an iteration whether it is needed or not
    iterate_if_needed = 2,  //!< indicator that the iterations need to continue
};

#define ITERATION_COMPLETE helics::iteration_request::no_iterations
#define NO_ITERATION helics::iteration_request::no_iterations
#define FORCE_ITERATION helics::iteration_request::force_iteration
#define ITERATE_IF_NEEDED helics::iteration_request::iterate_if_needed

/** defining some check modes for dealing with required or optional components*/
enum class handle_check_mode : char
{
    required = 0,  //!< indicator that the publication or endpoint is required to be there
    optional = 1,  //!< indicator that the publication or endpoint is optional
};

/**generate a string based on the core type*/
std::string to_string (core_type type);

/** generate a core type value from a std::string
@param a string describing the desired core type
@return a value of the helics_core_type enumeration
@throws invalid_argument if the string is not recognized
*/
core_type coreTypeFromString (std::string type);

/**
 * Returns true if core/broker type specified is available in current compilation.
 */
bool isCoreTypeAvailable (core_type type) noexcept;
} //namespace helics

#define PUBLICATION_REQUIRED helics::handle_check_mode::required
#define PUBLICATION_OPTIONAL helics::handle_check_mode::optional

