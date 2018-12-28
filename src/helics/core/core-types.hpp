/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once
#include "../helics_enums.h"
#include "federate_id.hpp"
#include <string>

/** @file
@details definitions of types an enumerations used in helics
*/

namespace helics
{
/** enumeration of the possible federate states*/
enum federate_state
{
    HELICS_CREATED, /*!> state upon creation, all registration calls are allowed*/
    HELICS_INITIALIZING,  //!< the federation has entered initialization state and initial values can be published
    HELICS_EXECUTING,  //!< the federation has entered execution state and it now advancing in time
    HELICS_TERMINATING,  //!< the federate is in the process of shutting down
    HELICS_ERROR,  //!< the federation has encountered an error
    HELICS_FINISHED,  //!< the federation has finished its execution
    HELICS_UNKNOWN,  //!< unknown state
};

/** the type of the cores that are available */
enum class core_type : int
{
    DEFAULT = helics_core_type_default,  //!< pick a core type depending on compile configuration usually either
                                         //!< ZMQ if available or UDP
    ZMQ = helics_core_type_zmq,  //!< use the Zero MQ networking protocol
    MPI = helics_core_type_mpi,  //!< use MPI for operation on a parallel cluster
    TEST = helics_core_type_test,  //!< use the Test core if all federates are in the same process
    INTERPROCESS = helics_core_type_interprocess,  //!< interprocess uses memory mapped files to transfer data (for
                                                   //!< use when all federates are on the same machine
    IPC = helics_core_type_ipc,  //!< same as INTERPROCESS
    TCP = helics_core_type_tcp,  //!< use a generic TCP protocol message stream to send messages
    TCP_SS =
      helics_core_type_tcp_ss,  //!< a single socket version of the TCP core for more easily handling firewalls
    UDP = helics_core_type_udp,  //!< use UDP packets to send the data
    NNG = helics_core_type_nng,  //!< reserved for future Nanomsg implementation
    ZMQ_SS = helics_core_type_zmq_test,  //!< single socket version of ZMQ core for better scalability performance
    HTTP = helics_core_type_http,  //!< core/broker using web traffic
    UNRECOGNIZED = 22,  //!< unknown

};

/** enumeration of the possible message processing results*/
enum class message_processing_result : signed char
{

    continue_processing = -2,  //!< the current loop should continue
    delay_message = -1,  //!< delay the current message and continue processing
    next_step = 0,  //!< indicator that the iterations have completed
    iterating = 2,  //!< indicator that the iterations need to continue
    halted = 3,  //!< indicator that the simulation has been halted
    error = 7,  //!< indicator that an error has occurred
};

inline bool returnableResult (message_processing_result result)
{
    return (result >= message_processing_result::next_step);
}
/** enumeration of the possible states of iteration*/
enum class iteration_result : signed char
{
    next_step = 0,  //!< indicator that the iterations have completed and the federate has moved to the next step
    iterating = 2,  //!< indicator that the iterations need to continue
    halted = 3,  //!< indicator that the simulation has been halted
    error = 7,  //!< indicator that an error has occurred
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

/**generate a string based on the core type*/
std::string to_string (core_type type);

/** generate a core type value from a std::string
@param a string describing the desired core type
@return a value of the helics_core_type enumeration
helics::core_type::unrecognized if the type is not valid
*/
core_type coreTypeFromString (std::string type) noexcept;

/**
 * Returns true if core/broker type specified is available in current compilation.
 */
bool isCoreTypeAvailable (core_type type) noexcept;

}  // namespace helics
