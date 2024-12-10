/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../helics_enums.h"

#include <cstdint>
#include <string>
#include <string_view>

/** @file
@details definitions of types an enumerations used in helics
*/

namespace helics {
/** enumeration of the possible federate states*/
enum class FederateStates : std::uint8_t {
    CREATED, /*!> state upon creation, all registration calls are allowed*/
    INITIALIZING,  //!< the federation has entered initialization state and initial values
                   //!< can be published
    EXECUTING,  //!< the federation has entered execution state and it now advancing in time
    TERMINATING,  //!< the federate is in the process of shutting down
    ERRORED,  //!< the federation has encountered an error
    FINISHED,  //!< the federation has finished its execution
    UNKNOWN,  //!< unknown state
};

/** enumeration of the threading options for a core*/
enum class CoreThreading : std::uint8_t {
    /** choose the default based on federate options*/
    DEFAULT = HELICS_CORE_TYPE_DEFAULT,
    /** specify the core should be multithreaded*/
    MULTI = HELICS_FLAG_MULTI_THREAD_CORE,
    /** specify a single threaded core*/
    SINGLE = HELICS_FLAG_SINGLE_THREAD_CORE
};

/** convert the state into a human readable string*/
const std::string& fedStateString(FederateStates state);

/** the type of the cores that are available */
enum class CoreType : int {
    DEFAULT = HELICS_CORE_TYPE_DEFAULT,  //!< pick a core type depending on compile configuration
                                         //!< usually either
    //!< ZMQ if available or UDP
    ZMQ = HELICS_CORE_TYPE_ZMQ,  //!< use the Zero MQ networking protocol
    MPI = HELICS_CORE_TYPE_MPI,  //!< use MPI for operation on a parallel cluster
    TEST = HELICS_CORE_TYPE_TEST,  //!< use the Test core if all federates are in the same process
    INTERPROCESS = HELICS_CORE_TYPE_INTERPROCESS,  //!< interprocess uses memory mapped files to
                                                   //!< transfer data (for
    //!< use when all federates are on the same machine
    IPC = HELICS_CORE_TYPE_IPC,  //!< same as INTERPROCESS
    TCP = HELICS_CORE_TYPE_TCP,  //!< use a generic TCP protocol message stream to send messages
    TCP_SS = HELICS_CORE_TYPE_TCP_SS,  //!< a single socket version of the TCP core for more easily
                                       //!< handling firewalls
    UDP = HELICS_CORE_TYPE_UDP,  //!< use UDP packets to send the data
    NNG = HELICS_CORE_TYPE_NNG,  //!< reserved for future Nanomsg implementation
    ZMQ_SS = HELICS_CORE_TYPE_ZMQ_SS,  //!< single socket version of ZMQ core for better
                                       //!< scalability performance
    HTTP = HELICS_CORE_TYPE_HTTP,  //!< core/broker using web traffic
    WEBSOCKET = HELICS_CORE_TYPE_WEBSOCKET,  //!< core/broker using web sockets
    INPROC = HELICS_CORE_TYPE_INPROC,  //!< core/broker using a stripped down in process core type
    NULLCORE = HELICS_CORE_TYPE_NULL,  //!< explicit core type that doesn't exist
    EMPTY = HELICS_CORE_TYPE_EMPTY,  //!< core type that does nothing and can't communicate
    UNRECOGNIZED = 22,  //!< unknown
    MULTI = 45,  //!< use the multi-broker
    EXTRACT = HELICS_CORE_TYPE_EXTRACT  //!< extract core type from later arguments/files
};

/** enumeration of the possible message processing results*/
enum class MessageProcessingResult : std::int8_t {

    CONTINUE_PROCESSING = -2,  //!< the current loop should continue
    DELAY_MESSAGE = -1,  //!< delay the current message and continue processing
    NEXT_STEP = 0,  //!< indicator that the iterations have completed
    ITERATING = 2,  //!< indicator that the iterations need to continue
    HALTED = 3,  //!< indicator that the simulation has been halted
    /// indicator that there was a return request but no other conditions or issues
    USER_RETURN = 5,
    ERROR_RESULT = 7,  //!< indicator that an error has occurred
    REPROCESS_MESSAGE = 8,  // indicator that the message needs to be processed again
    BUSY = 10,  // indicator that processing could not be done since the resource was busy
};
/** function to check if the message processing result should be returned or processing continued*/
inline bool returnableResult(MessageProcessingResult result)
{
    return (result >= MessageProcessingResult::NEXT_STEP);
}
/** enumeration of the possible states of iteration*/
enum class IterationResult : signed char {
    NEXT_STEP = 0,  //!< indicator that the iterations have completed and the federate has moved to
                    //!< the next step
    ITERATING = 2,  //!< indicator that the iterations need to continue
    HALTED = 3,  //!< indicator that the simulation has been halted
    ERROR_RESULT = 7  //!< indicator that an error has occurred
};

/** enumeration of the possible iteration requests by a federate*/
enum class IterationRequest : signed char {
    NO_ITERATIONS = 0,  //!< indicator that the iterations have completed
    FORCE_ITERATION = 1,  //!< force an iteration whether it is needed or not
    ITERATE_IF_NEEDED = 2,  //!< indicator that the iterations need to continue
    HALT_OPERATIONS = 3,  //!< indicator that the federate should halt
    ERROR_CONDITION = 7  //!< indicator that the federate has errored and co-simulation should stop
};

/** define the type of the handle*/
enum class InterfaceType : char {
    UNKNOWN = 'u',
    PUBLICATION = 'p',  //!< handle to output interface
    INPUT = 'i',  //!< handle to a input interface
    ENDPOINT = 'e',  //!< handle to an endpoint
    FILTER = 'f',  //!< handle to a filter
    TRANSLATOR = 't',  //!< handle to a translator object
    SINK = 's'  //!< handle to a data sink
};

std::string_view interfaceTypeName(InterfaceType type) noexcept;

}  // namespace helics
/// simplified alias to indicate that iterations have concluded
constexpr auto ITERATION_COMPLETE = helics::IterationRequest::NO_ITERATIONS;
/// simplified alias to indicate that no iterations are needed
constexpr auto NO_ITERATION = helics::IterationRequest::NO_ITERATIONS;
/// simplified alias to force an iteration
constexpr auto FORCE_ITERATION = helics::IterationRequest::FORCE_ITERATION;
/// simplified alias to indicate that helics should iterate if warranted
constexpr auto ITERATE_IF_NEEDED = helics::IterationRequest::ITERATE_IF_NEEDED;
