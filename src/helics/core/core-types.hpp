/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#ifndef HELICS_CORE_TYPES_HEADER
#    define HELICS_CORE_TYPES_HEADER

#    include "../helics_enums.h"

#    include <string>

/** @file
@details definitions of types an enumerations used in helics
*/

namespace helics {
/** enumeration of the possible federate states*/
enum federate_state {
    HELICS_CREATED, /*!> state upon creation, all registration calls are allowed*/
    HELICS_INITIALIZING,  //!< the federation has entered initialization state and initial values
                          //!< can be published
    HELICS_EXECUTING,  //!< the federation has entered execution state and it now advancing in time
    HELICS_TERMINATING,  //!< the federate is in the process of shutting down
    HELICS_ERROR,  //!< the federation has encountered an error
    HELICS_FINISHED,  //!< the federation has finished its execution
    HELICS_UNKNOWN,  //!< unknown state
};

/** convert the state into a human readable string*/
const std::string& fedStateString(federate_state state);

/** the type of the cores that are available */
enum class core_type : int {
    DEFAULT = helics_core_type_default,  //!< pick a core type depending on compile configuration
                                         //!< usually either
    //!< ZMQ if available or UDP
    ZMQ = helics_core_type_zmq,  //!< use the Zero MQ networking protocol
    MPI = helics_core_type_mpi,  //!< use MPI for operation on a parallel cluster
    TEST = helics_core_type_test,  //!< use the Test core if all federates are in the same process
    INTERPROCESS = helics_core_type_interprocess,  //!< interprocess uses memory mapped files to
                                                   //!< transfer data (for
    //!< use when all federates are on the same machine
    IPC = helics_core_type_ipc,  //!< same as INTERPROCESS
    TCP = helics_core_type_tcp,  //!< use a generic TCP protocol message stream to send messages
    TCP_SS = helics_core_type_tcp_ss,  //!< a single socket version of the TCP core for more easily
                                       //!< handling firewalls
    UDP = helics_core_type_udp,  //!< use UDP packets to send the data
    NNG = helics_core_type_nng,  //!< reserved for future Nanomsg implementation
    ZMQ_SS = helics_core_type_zmq_test,  //!< single socket version of ZMQ core for better
                                         //!< scalability performance
    HTTP = helics_core_type_http,  //!< core/broker using web traffic
    WEBSOCKET = helics_core_type_websocket,  //!< core/broker using web sockets
    INPROC = helics_core_type_inproc,  //!< core/broker using a stripped down in process core type
    NULLCORE = helics_core_type_null,  //!< explicit core type that doesn't exist
    UNRECOGNIZED = 22,  //!< unknown
    MULTI = 45  //!< use the multi-broker

};

/** enumeration of the possible message processing results*/
enum class message_processing_result : signed char {
    continue_processing = -2,  //!< the current loop should continue
    delay_message = -1,  //!< delay the current message and continue processing
    next_step = 0,  //!< indicator that the iterations have completed
    error = 1,  //!< indicator that an error has occurred
    halted = 2,  //!< indicator that the simulation has been halted
    iterating = 3,  //!< indicator that the iterations need to continue
    reprocess_message = 8  // indicator that the message needs to be processed again
};
/** function to check if the message processing result should be returned or processing continued*/
inline bool returnableResult(message_processing_result result)
{
    return (result >= message_processing_result::next_step);
}

/** enumeration of the possible states of iteration*/
enum class iteration_result : signed char {
    next_step = 0,  //!< indicator that the iterations have completed and the federate has moved to
                    //!< the next step
    error = 1,  //!< indicator that an error has occurred
    halted = 2,  //!< indicator that the simulation has been halted
    iterating = 3,  //!< indicator that the iterations need to continue
};

/** enumeration of the possible iteration requests by a federate*/
enum class iteration_request : signed char {
    no_iterations = 0,  //!< indicator that the iterations have completed
    force_iteration = 1,  //!< force an iteration whether it is needed or not
    iterate_if_needed = 2,  //!< indicator that the iterations need to continue
};

}  // namespace helics

constexpr auto ITERATION_COMPLETE =
    helics::iteration_request::no_iterations;  //!< simplified alias to indicate that iterations
                                               //!< have concluded
constexpr auto NO_ITERATION =
    helics::iteration_request::no_iterations;  //!< simplified alias to indicate that no iterations
                                               //!< are needed
constexpr auto FORCE_ITERATION =
    helics::iteration_request::force_iteration;  //!< simplified alias to force an iteration
constexpr auto ITERATE_IF_NEEDED =
    helics::iteration_request::iterate_if_needed;  //!< simplified alias to indicate that helics
                                                   //!< should iterate if
//!< warranted

// #TOBEDEPRECTATED The use of the core-types header for the functions contained in
// ../application_api/typeOperations.hpp is deprecated and will be removed in HELICS 3.0 please use
// ../application_api/typeOperations.hpp directory for those functions.
// This next section should be removed in HELICS 3.0 but is needed to prevent breaking changes
#    if defined HELICS_SHARED_LIBRARY || !defined HELICS_STATIC_CORE_LIBRARY
#        include "../application_api/typeOperations.hpp"
#    endif
#endif
