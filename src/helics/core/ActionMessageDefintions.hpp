/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../helics_enums.h"

#include <cstdint>
namespace helics {
/** namespace for message definitions*/
namespace action_message_def {
    constexpr int32_t cmd_info_basis = 0x10000000;

    /** enumeration of globally recognized commands
@details they are explicitly numbered for debugging and to ensure the enumeration is constant
across different compilers*/
    enum class action_t : int32_t {
        // priority commands ALL priority commands have a negative code
        cmd_priority_disconnect =
            -3,  //!< command to disconnect a broker from a higher level broker
        cmd_fed_ack =
            -25,  //!< a reply with the global id or an error if the fed registration failed

        cmd_broker_ack = -27,  // a reply to the connect command with a global route id
        cmd_add_route = -32,  //!< command to define a route
        cmd_route_ack = -16,  //!< acknowledge reply to a route registration
        cmd_register_route = -15,  //!< instructions to create a direct route to another federate
        cmd_reg_fed = -105,  //!< register a federate
        cmd_priority_ack =
            -254,  //!< priority commands usually do not have an ack this is an ack that
        //!< doesn't do anything
        cmd_query = -cmd_info_basis - 37,  //!< send a query this is a priority command
        cmd_query_ordered =
            937,  //!< send a query along the synchronous paths instead of priority channels
        cmd_set_global = -cmd_info_basis - 55,  //!< set a global value
        cmd_broker_query = -37,  //!< send a query to a core
        cmd_broker_query_ordered = 939,  //!< send a query to a core
        cmd_query_reply = -cmd_info_basis - 38,  //!< response to a query
        cmd_query_reply_ordered = 942,  //!< response to a query on normal paths
        cmd_reg_broker =
            -cmd_info_basis - 40,  //!< for a broker to connect with a higher level broker
        cmd_broker_location = cmd_info_basis - 57,  //!< command to define a new broker location
        cmd_broker_setup = -1,  //!< command to load the setup information for a broker
        cmd_ignore = 0,  //!< null command
        cmd_tick = 1,  //!< command for a timer tick
        cmd_user_disconnect = 2,  //!< command specifying that a user has issued a disconnect signal
        cmd_disconnect = 3,  //!< disconnect command
        cmd_disconnect_name = 4,  //!< disconnect a broker or core by name vs id
        cmd_disconnect_check = 5,  //!< check for a disconnect
        cmd_disconnect_fed = 6,  //!< disconnect a federate
        cmd_broadcast_disconnect = 7,  //!< a broadcast disconnect message
        cmd_disconnect_core = 8,  //!< disconnect a core
        cmd_disconnect_broker = 9,  //!< disconnect a broker
        cmd_disconnect_fed_ack = 1006,  //!< federate disconnect ack
        cmd_disconnect_core_ack = 1008,  //!< ack for core disconnect
        cmd_disconnect_broker_ack = 1009,  //!< ack for broker disconnect
        cmd_check_connections = 297,  //!< command to check for any connections
        cmd_ping = 298,  //!< request for an Echo response
        cmd_ping_priority = -298,  //!< request for an Echo response on priority channel
        cmd_ping_reply = 299,  //!< response to a ping request
        cmd_broker_ping = 306,  //!< ping to send to a broker to also ping subbrokers and cores
        cmd_init = 10,  //!< request entry to init mode
        cmd_init_grant = 11,  //!< grant entry to initialization mode
        cmd_init_not_ready = 12,  //!< retract an init ready command
        cmd_exec_request = 20,  //!< request an iteration or entry to execution mode
        cmd_exec_grant = 22,  //!< grant entry to exec mode or iterate
        cmd_exec_check = 24,  //!< command to run a check on execution entry
        cmd_ack = 254,  //!< acknowledge command to for various purposes

        cmd_stop = 30,  //!< halt execution
        cmd_terminate_immediately = 31,  //!< immediate halt no-disconnect;

        cmd_time_grant = 35,  //!< grant a time or iteration
        cmd_time_check = 36,  //!< command to run a check on whether time can be granted
        cmd_request_current_time =
            38,  //!< command to request the current time status of a federate

        cmd_time_block = 40,  //!< prevent a federate from granting time until the block is cleared
        cmd_time_unblock = 41,  //!< clear a time block
        cmd_time_barrier_request = 42,  //!< request a time barrier
        cmd_time_barrier = 43,  //!< setup a global time barrier
        cmd_time_barrier_clear = 44,  //!< clear a global time barrier

        cmd_pub = 52,  //!< publish a value
        cmd_bye = 2000,  //!< message stating this is the last communication from a federate
        cmd_log = 55,  //!< log a message with the root broker
        cmd_warning = 9990,  //!< indicate some sort of warning
        cmd_error = 10000,  //!< indicate an error with a federate
        cmd_local_error = 10003,  //!< indicate a local error within a federate/core/broker
        cmd_global_error = 10005,  //!< indicate a global error with a federate/core/broker and the
                                   //!< federation should terminate
        cmd_error_check = 10001,  //!< check some status for error and error timeouts
        cmd_invalid = 1010101,  //!< indicates that command has generated an invalid state
        cmd_send_route = 75,  //!< command to define a route information
        cmd_search_dependency = 1464,  //!< command to add a dependency by name
        cmd_add_dependency = 140,  //!< command to send a federate dependency information
        cmd_remove_dependency = 141,  //!< command to remove a dependency
        cmd_add_dependent = 144,  //!< command to add a dependent to a federate
        cmd_remove_dependent =
            145,  //!< command to remove a dependent from a federates consideration
        cmd_add_interdependency =
            148,  //!< command to add a federate as both dependent and a dependency
        cmd_remove_interdependency =
            149,  //!< command to remove a federate as both dependent and a dependency

        cmd_data_link =
            cmd_info_basis + 707,  //!< command to connect a publication with an endpoint
        cmd_filter_link = cmd_info_basis + 709,  //!< command to add a target to a filter

        cmd_fed_configure_time =
            202,  //!< command to update the configuration of a federate a time parameter
        cmd_fed_configure_int =
            203,  //!< command to update the configuration of a federate an int parameter
        cmd_fed_configure_flag =
            204,  //!< command to update the configuration of a federate a flag parameter
        cmd_core_configure = 207,  //!< command to update the configuration of a core
        cmd_interface_configure = 209,  //!< command to update the configuration of an interface
        cmd_broker_configure = 211,  //!< command to update the configuration of a broker
        cmd_base_configure = 213,  //!< command to update the configuration of a broker/core base

        cmd_update_filter_op =
            10427,  //!< command to update a filter op [should only used internal to a core]
        null_info_command =
            cmd_info_basis - 1,  //!< biggest command that doesn't have the info structure
        /** the biggest negative priority command*/
        priority_null_info_command = -cmd_info_basis - 1,
        // commands that require the extra info allocation have numbers greater than cmd_info_basis
        cmd_time_request = 500,  //!< request a time or iteration
        cmd_force_time_grant =
            525,  //!< command to force grant a time regardless of other considerations
        cmd_send_message = cmd_info_basis + 20,  //!< send a message
        cmd_null_message = 726,  //!< used when a filter drops a message but it needs to return
        cmd_null_dest_message = 730,  //!< used when a destination filter drops a message
        cmd_send_for_filter = cmd_info_basis +
            30,  //!< send a message to be filtered and forward on to the destination
        cmd_send_for_dest_filter_return =
            cmd_info_basis + 31,  //!< send a message to a destination filter for processing
        cmd_send_for_filter_return =
            cmd_info_basis + 35,  //!< send a message back to its originator after filtering
        cmd_filter_result =
            cmd_info_basis + 40,  //!< the results of a filter message going back to its originator
        cmd_dest_filter_result = cmd_info_basis +
            41,  //!< the result of a destination filter going back to its originator
        cmd_reg_pub = cmd_info_basis + 50,  //!< register a publication
        cmd_add_publisher = 50,  //!< notify of a publication
        cmd_reg_filter = cmd_info_basis + 60,  //!< register a destination filter
        cmd_add_filter = 62,  //!< notify of a destination filter
        cmd_reg_input = cmd_info_basis + 70,  //!< register an input interface
        cmd_add_subscriber = 70,  //!< notify of a subscription
        cmd_reg_end = cmd_info_basis + 90,  //!< register an endpoint
        cmd_add_endpoint = 90,  //!< notify of a source endpoint

        cmd_add_named_input = 104,  //!< command to add a named input as a target
        cmd_add_named_filter = 105,  //!< command to add named filter as a target
        cmd_add_named_publication = 106,  //!< command to add a named publication as a target
        cmd_add_named_endpoint = 107,  //!< command to add a named endpoint as a target
        cmd_remove_named_input = 124,  //!< cmd to remove a target from connection by name
        cmd_remove_named_filter = 125,  //!< cmd to remove a filter from connection by name
        cmd_remove_named_publication =
            126,  //!< cmd to remove a publication from connection by name
        cmd_remove_named_endpoint = 127,  //!< cmd to remove an endpoint

        cmd_remove_subscriber = 134,  //!< cmd to remove a target from connection
        cmd_remove_filter = 135,  //!< cmd to remove a filter from connection
        cmd_remove_publication = 136,  //!< cmd to remove a publication from connection
        cmd_remove_endpoint = 137,  //!< cmd to remove an endpoint

        cmd_close_interface = 133,  //!< cmd to close all communications from an interface
        cmd_multi_message = 1037,  //!< cmd that encapsulates a bunch of messages in its payload

        cmd_connection_error = 2034,  //!< cmd indicating a connection error with a broker/federate

        cmd_protocol_priority =
            -60000,  //!< priority command used by protocol stacks and ignored by core
        cmd_protocol = 60000,  //!< command used in the protocol stacks and ignored by the core
        cmd_protocol_big = cmd_info_basis +
            60000,  //!< command used in the protocol stacks with the additional info
        cmd_resend = 121212  //!< command to resend some data
    };

}  // namespace action_message_def

#define CMD_IGNORE action_message_def::action_t::cmd_ignore
#define CMD_INVALID action_message_def::action_t::cmd_invalid
#define CMD_TICK action_message_def::action_t::cmd_tick
#define CMD_REG_BROKER action_message_def::action_t::cmd_reg_broker
#define CMD_PRIORITY_DISCONNECT action_message_def::action_t::cmd_priority_disconnect
#define CMD_USER_DISCONNECT action_message_def::action_t::cmd_user_disconnect
#define CMD_DISCONNECT action_message_def::action_t::cmd_disconnect
#define CMD_DISCONNECT_NAME action_message_def::action_t::cmd_disconnect_name
#define CMD_DISCONNECT_CHECK action_message_def::action_t::cmd_disconnect_check
#define CMD_DISCONNECT_FED action_message_def::action_t::cmd_disconnect_fed
#define CMD_BROADCAST_DISCONNECT action_message_def::action_t::cmd_broadcast_disconnect

#define CMD_DISCONNECT_CORE action_message_def::action_t::cmd_disconnect_core
#define CMD_DISCONNECT_BROKER action_message_def::action_t::cmd_disconnect_broker
#define CMD_DISCONNECT_FED_ACK action_message_def::action_t::cmd_disconnect_fed_ack
#define CMD_DISCONNECT_CORE_ACK action_message_def::action_t::cmd_disconnect_core_ack
#define CMD_DISCONNECT_BROKER_ACK action_message_def::action_t::cmd_disconnect_broker_ack
#define CMD_CONNECTION_ERROR action_message_def::action_t::cmd_connection_error

#define CMD_CHECK_CONNECTIONS action_message_def::action_t::cmd_check_connections
#define CMD_PING action_message_def::action_t::cmd_ping
#define CMD_PING_PRIORITY action_message_def::action_t::cmd_ping_priority

#define CMD_BROKER_PING action_message_def::action_t::cmd_broker_setup
#define CMD_PING_REPLY action_message_def::action_t::cmd_ping_reply
#define CMD_BROKER_SETUP action_message_def::action_t::cmd_broker_setup
#define CMD_BROKER_LOCATION action_message_def::action_t::cmd_broker_location

#define CMD_INIT action_message_def::action_t::cmd_init
#define CMD_INIT_NOT_READY action_message_def::action_t::cmd_init_not_ready
#define CMD_INIT_GRANT action_message_def::action_t::cmd_init_grant
#define CMD_EXEC_REQUEST action_message_def::action_t::cmd_exec_request
#define CMD_EXEC_GRANT action_message_def::action_t::cmd_exec_grant
#define CMD_EXEC_CHECK action_message_def::action_t::cmd_exec_check
#define CMD_REG_ROUTE action_message_def::action_t::cmd_register_route
#define CMD_ROUTE_ACK action_message_def::action_t::cmd_route_ack
#define CMD_STOP action_message_def::action_t::cmd_stop
#define CMD_TERMINATE_IMMEDIATELY action_message_def::action_t::cmd_terminate_immediately
#define CMD_TIME_REQUEST action_message_def::action_t::cmd_time_request
#define CMD_TIME_GRANT action_message_def::action_t::cmd_time_grant
#define CMD_FORCE_TIME_GRANT action_message_def::action_t::cmd_force_time_grant
#define CMD_TIME_CHECK action_message_def::action_t::cmd_time_check
#define CMD_REQUEST_CURRENT_TIME action_message_def::action_t::cmd_request_current_time

#define CMD_TIME_BLOCK action_message_def::action_t::cmd_time_block
#define CMD_TIME_UNBLOCK action_message_def::action_t::cmd_time_unblock

#define CMD_TIME_BARRIER_REQUEST action_message_def::action_t::cmd_time_barrier_request
#define CMD_TIME_BARRIER action_message_def::action_t::cmd_time_barrier
#define CMD_TIME_BARRIER_CLEAR action_message_def::action_t::cmd_time_barrier_clear

#define CMD_SEND_MESSAGE action_message_def::action_t::cmd_send_message
#define CMD_SEND_FOR_FILTER action_message_def::action_t::cmd_send_for_filter
#define CMD_SEND_FOR_FILTER_AND_RETURN action_message_def::action_t::cmd_send_for_filter_return
#define CMD_SEND_FOR_DEST_FILTER_AND_RETURN                                                        \
    action_message_def::action_t::cmd_send_for_dest_filter_return
#define CMD_NULL_MESSAGE action_message_def::action_t::cmd_null_message
#define CMD_NULL_DEST_MESSAGE action_message_def::action_t::cmd_null_dest_message
#define CMD_FILTER_RESULT action_message_def::action_t::cmd_filter_result
#define CMD_DEST_FILTER_RESULT action_message_def::action_t::cmd_dest_filter_result

#define CMD_PUB action_message_def::action_t::cmd_pub
#define CMD_LOG action_message_def::action_t::cmd_log
#define CMD_WARNING action_message_def::action_t::cmd_warning
#define CMD_ERROR action_message_def::action_t::cmd_error
#define CMD_GLOBAL_ERROR action_message_def::action_t::cmd_global_error
#define CMD_LOCAL_ERROR action_message_def::action_t::cmd_local_error
#define CMD_ERROR_CHECK action_message_def::action_t::cmd_error_check

#define CMD_RESEND action_message_def::action_t::cmd_resend

#define CMD_REG_PUB action_message_def::action_t::cmd_reg_pub
#define CMD_ADD_PUBLISHER action_message_def::action_t::cmd_add_publisher
#define CMD_REG_INPUT action_message_def::action_t::cmd_reg_input
#define CMD_ADD_SUBSCRIBER action_message_def::action_t::cmd_add_subscriber

#define CMD_ADD_NAMED_ENDPOINT action_message_def::action_t::cmd_add_named_endpoint
#define CMD_ADD_NAMED_FILTER action_message_def::action_t::cmd_add_named_filter
#define CMD_ADD_NAMED_PUBLICATION action_message_def::action_t::cmd_add_named_publication
#define CMD_ADD_NAMED_INPUT action_message_def::action_t::cmd_add_named_input

#define CMD_REMOVE_NAMED_ENDPOINT action_message_def::action_t::cmd_remove_named_endpoint
#define CMD_REMOVE_NAMED_FILTER action_message_def::action_t::cmd_remove_named_filter
#define CMD_REMOVE_NAMED_PUBLICATION action_message_def::action_t::cmd_remove_named_publication
#define CMD_REMOVE_NAMED_INPUT action_message_def::action_t::cmd_remove_named_input

#define CMD_REMOVE_ENDPOINT action_message_def::action_t::cmd_remove_endpoint
#define CMD_REMOVE_FILTER action_message_def::action_t::cmd_remove_filter
#define CMD_REMOVE_PUBLICATION action_message_def::action_t::cmd_remove_publication
#define CMD_REMOVE_SUBSCRIBER action_message_def::action_t::cmd_remove_subscriber

#define CMD_CLOSE_INTERFACE action_message_def::action_t::cmd_close_interface

#define CMD_DATA_LINK action_message_def::action_t::cmd_data_link
#define CMD_FILTER_LINK action_message_def::action_t::cmd_filter_link

#define CMD_REMOVE_NAMED_TARGET action_message_def::action_t::cmd_remove_named_target
#define CMD_REMOVE_TARGET action_message_def::action_t::cmd_remove_target

#define CMD_REG_ENDPOINT action_message_def::action_t::cmd_reg_end
#define CMD_ADD_ENDPOINT action_message_def::action_t::cmd_add_endpoint

#define CMD_REG_FILTER action_message_def::action_t::cmd_reg_filter
#define CMD_ADD_FILTER action_message_def::action_t::cmd_add_filter

#define CMD_SEARCH_DEPENDENCY action_message_def::action_t::cmd_search_dependency
#define CMD_ADD_DEPENDENCY action_message_def::action_t::cmd_add_dependency
#define CMD_REMOVE_DEPENDENCY action_message_def::action_t::cmd_remove_dependency
#define CMD_ADD_DEPENDENT action_message_def::action_t::cmd_add_dependent
#define CMD_REMOVE_DEPENDENT action_message_def::action_t::cmd_remove_dependent
#define CMD_ADD_INTERDEPENDENCY action_message_def::action_t::cmd_add_interdependency
#define CMD_REMOVE_INTERDEPENDENCY action_message_def::action_t::cmd_remove_interdependency

#define CMD_REG_FED action_message_def::action_t::cmd_reg_fed
#define CMD_BROKER_ACK action_message_def::action_t::cmd_broker_ack
#define CMD_FED_ACK action_message_def::action_t::cmd_fed_ack
#define CMD_PROTOCOL_PRIORITY action_message_def::action_t::cmd_protocol_priority
#define CMD_PROTOCOL action_message_def::action_t::cmd_protocol
#define CMD_PROTOCOL_BIG action_message_def::action_t::cmd_protocol_big

#define CMD_FED_CONFIGURE_TIME action_message_def::action_t::cmd_fed_configure_time
#define CMD_FED_CONFIGURE_INT action_message_def::action_t::cmd_fed_configure_int
#define CMD_FED_CONFIGURE_FLAG action_message_def::action_t::cmd_fed_configure_flag
#define CMD_INTERFACE_CONFIGURE action_message_def::action_t::cmd_interface_configure

#define CMD_CORE_CONFIGURE action_message_def::action_t::cmd_core_configure
#define CMD_BROKER_CONFIGURE action_message_def::action_t::cmd_broker_configure
#define CMD_BASE_CONFIGURE action_message_def::action_t::cmd_base_configure

#define CMD_ACK action_message_def::action_t::cmd_ack
#define CMD_PRIORITY_ACK action_message_def::action_t::cmd_priority_ack

#define CMD_QUERY action_message_def::action_t::cmd_query
#define CMD_QUERY_ORDERED action_message_def::action_t::cmd_query_ordered
#define CMD_BROKER_QUERY action_message_def::action_t::cmd_broker_query
#define CMD_BROKER_QUERY_ORDERED action_message_def::action_t::cmd_broker_query_ordered
#define CMD_QUERY_REPLY action_message_def::action_t::cmd_query_reply
#define CMD_QUERY_REPLY_ORDERED action_message_def::action_t::cmd_query_reply_ordered
#define CMD_SET_GLOBAL action_message_def::action_t::cmd_set_global

#define CMD_MULTI_MESSAGE action_message_def::action_t::cmd_multi_message

// definitions for the protocol options
#define PROTOCOL_PING 10
#define PROTOCOL_PONG 11
#define CLOSE_RECEIVER 23425215
/// some definitions for test core debugging
#define PAUSE_TRANSMITTER 453623
#define ALLOW_MESSAGES 453624
#define UNPAUSE_TRANSMITTER 453625
// routing information
#define NEW_ROUTE 233
#define REMOVE_ROUTE 244
#define CONNECTION_INFORMATION 299
#define CONNECTION_REQUEST 301
#define CONNECTION_ACK 304
#define NEW_BROKER_INFORMATION 333
#define DISCONNECT 2523
#define DISCONNECT_ERROR 2623
#define DELAY_CONNECTION 3795

#define NAME_NOT_FOUND 2726
#define RECONNECT_TRANSMITTER 1997
#define RECONNECT_RECEIVER 1999
// for requesting port definitions on a computer
#define PORT_DEFINITIONS 1451
#define QUERY_PORTS 1453
#define REQUEST_PORTS 1455
#define SET_USED_PORTS 1457
#define NULL_REPLY 0;

// definitions related to Core Configure
#define UPDATE_FILTER_OPERATOR 572
#define UPDATE_QUERY_CALLBACK 581
#define UPDATE_LOGGING_CALLBACK 592
#define REQUEST_TICK_FORWARDING 607

/** return the name of the action
@param action The action to get the name for
@return a pointer to string with the name
*/
const char* actionMessageType(action_message_def::action_t action);

enum cmd_error_codes : int {
    lost_server_connection_code = -5,
    connection_error_code = -2,
    already_init_error_code = 5,
    duplicate_federate_name_error_code = 6,
    duplicate_broker_name_error_code = 7,
    mismatch_broker_key_error_code = 9,
    max_federate_count_exceeded = 11,
    max_broker_count_exceeded = 13
};

/** return a string associated with a particular error code
@param[in,out] errorcode The error to get the string for
@return a pointer to string with the name
*/
const char* commandErrorString(int errorcode);

}  // namespace helics
