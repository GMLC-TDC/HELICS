/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once
#include <cstdint>

namespace helics
{
/** flag definitions for the action Message Flag field*/
enum action_message_flags : uint16_t
{
    iteration_requested_flag = 0,  //!< indicator that an iteration has been requested
    processing_complete_flag = 1,  //!< indicator that processing has been completed
    required_flag = 2,  //!< flag indicating that an action or match is required

    error_flag = 4,  //!< flag indicating an error condition associated with the command
    indicator_flag = 5,  //!< flag used for setting values
    extra_flag1 = 7,  //!< extra flag
    forwarding_coordinator =8, //!< flag indicating that a dependency is a forwarding coordinator
    clone_flag = 9,  //!< flag indicating the filter is a clone filter
    destination_processing_flag=11, //!< flag indicating the message is for destination processing
    broker_min_time_flag=12, //!< flag indicating that a broker the time constraint
};
/** namespace for message definitions*/
namespace action_message_def
{
const int32_t cmd_info_basis = 0x10000000;

/** enumeration of globally recognized commands
@details they are explicitly numbered for debugging and to ensure the enumeration is constant
across different compilers*/
enum class action_t : int32_t
{
    // priority commands ALL priority commands have a negative code
    cmd_priority_disconnect = -3,  //!< command to disconnect a broker from a higher level broker
    cmd_fed_ack = -25,  //!< a reply with the global id or an error if the fed registration failed

    cmd_broker_ack = -27,  // a reply to the connect command with a global route id
    cmd_add_route = -32,  //!< command to define a route
    cmd_route_ack = -16,  //!< acknowledge reply to a route registration
    cmd_register_route = -15,  //!< instructions to create a direct route to another federate
    cmd_reg_fed = -105,  //!< register a federate
    cmd_priority_ack = -254,  //!< priority commands usually have an ack this is an ack that doesn't do anything
    cmd_query = -cmd_info_basis - 37,  //!< send a query this is a priority command
    cmd_query_reply = -cmd_info_basis - 38,  //!< response to a query
    cmd_reg_broker = -cmd_info_basis - 40,  //!< for a broker to connect with a higher level broker

    cmd_ignore = 0,  //!< null command
    cmd_tick = 1,  //!< command for a timer tick
    cmd_disconnect = 3,  //!< disconnect command
    cmd_disconnect_name = 4,  //!< disconnect a broker or core by name vs id
    cmd_ping = 6,  //!< request for an Echo response
    cmd_ping_reply = 7,  //!< response to a ping request

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
    cmd_request_current_time = 38,  //!< command to request the current time status of a federate

    cmd_time_block=40,   //!< prevent a federate from granting time until the block is cleared
    cmd_time_unblock=41,  //!< clear a time block

    cmd_pub = 45,  //!< publish a value
    cmd_bye = 2000,  //!< message stating this is the last communication from a federate
    cmd_log = 55,  //!< log a message with the root broker
    cmd_warning = 9990,  //!< indicate some sort of warning
    cmd_error = 10000,  //!< indicate an error with a federate
    cmd_invalid = 1010101,  //!< indicates that command has generated an invalid state
    cmd_send_route = 75,  //!< command to define a route information
    cmd_subscriber = 85,  // !< command to send a subscriber
    cmd_search_dependency=134, //!< command to add a dependency by name
    cmd_add_dependency = 140,  //!< command to send a federate dependency information
    cmd_remove_dependency = 141,  //!< command to remove a dependency
    cmd_add_dependent = 144,  //!< command to add a dependent to a federate
    cmd_remove_dependent = 145,  //!< command to remove a dependent from a federates consideration
    cmd_add_interdependency = 148,  //!< command to add a federate as both dependent and a dependency
    cmd_remove_interdependency = 149,  //!< command to remove a federate as both dependent and a dependency

    cmd_fed_configure = 205,  //!< command to update the configuration of a federate
    cmd_core_configure = 207,  //!< command to update the configuration of a core

    cmd_update_filter_op= 10427,  //!< command to update a filter op [should only used internal to a core]
    null_info_command = cmd_info_basis - 1,  //!< biggest command that doesn't have the info structure
    /** the biggest negative priority command*/
    priority_null_info_command = -cmd_info_basis - 1,
    // commands that require the extra info allocation have numbers greater than cmd_info_basis
    cmd_time_request = 500,  //!< request a time or iteration
    cmd_send_message = cmd_info_basis + 20,  //!< send a message
    cmd_null_message = 726,  //!< used when a filter drops a message but it needs to return
    cmd_null_dest_message=730, //!< used when a destination filter drops a message
    cmd_send_for_filter =
      cmd_info_basis + 30,  //!< send a message to be filtered and forward on to the destination
    cmd_send_for_dest_filter_return = cmd_info_basis+31, //!< send a message to a destination filter for processing
    cmd_send_for_filter_return = cmd_info_basis + 35,  //!< send a message back to its originator after filtering
    cmd_filter_result = cmd_info_basis + 40,  //!< the results of a filter message going back to its originator
    cmd_dest_filter_result=cmd_info_basis+41,  //!< the result of a destination filter going back to its originator
    cmd_reg_pub = cmd_info_basis + 50,  //!< register a publication
    cmd_notify_pub = 50,  //!< notify of a publication
    cmd_reg_dst_filter = cmd_info_basis + 60,  //!< register a destination filter
    cmd_notify_dst_filter = cmd_info_basis + 62,  //!< notify of a destination filter
    cmd_reg_sub = cmd_info_basis + 70,  //!< register a subscription
    cmd_notify_sub = 70,  //!< notify of a subscription
    cmd_reg_src_filter = cmd_info_basis + 80,  //!< register a source filter
    cmd_notify_src_filter = cmd_info_basis + 82,  //!< notify of a source
    cmd_reg_end = cmd_info_basis + 90,  //!< register an endpoint
    cmd_notify_end = 90,  //!< notify of an endpoint

    cmd_has_operator = 92,  //!< notify that a filter has an operator
    cmd_protocol_priority = -60000,  //!< priority command used by protocol stacks and ignored by core
    cmd_protocol = 60000,  //!< command used in the protocol stacks and ignored by the core
    cmd_protocol_big = cmd_info_basis + 60000  //!< command used in the protocol stacks with the additional info

};

}  // namespace action_message_def

#define CMD_IGNORE action_message_def::action_t::cmd_ignore
#define CMD_INVALID action_message_def::action_t::cmd_invalid
#define CMD_TICK action_message_def::action_t::cmd_tick
#define CMD_REG_BROKER action_message_def::action_t::cmd_reg_broker
#define CMD_PRIORITY_DISCONNECT action_message_def::action_t::cmd_priority_disconnect
#define CMD_DISCONNECT action_message_def::action_t::cmd_disconnect
#define CMD_DISCONNECT_NAME action_message_def::action_t::cmd_disconnect_name
#define CMD_PING action_message_def::action_t::cmd_ping
#define CMD_PING_REPLY action_message_def::action_t::cmd_ping_reply

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
#define CMD_TIME_CHECK action_message_def::action_t::cmd_time_check
#define CMD_REQUEST_CURRENT_TIME action_message_def::action_t::cmd_request_current_time

#define CMD_TIME_BLOCK action_message_def::action_t::cmd_time_block
#define CMD_TIME_UNBLOCK action_message_def::action_t::cmd_time_unblock

#define CMD_SEND_MESSAGE action_message_def::action_t::cmd_send_message
#define CMD_SEND_FOR_FILTER action_message_def::action_t::cmd_send_for_filter
#define CMD_SEND_FOR_FILTER_AND_RETURN action_message_def::action_t::cmd_send_for_filter_return
#define CMD_SEND_FOR_DEST_FILTER_AND_RETURN action_message_def::action_t::cmd_send_for_dest_filter_return
#define CMD_NULL_MESSAGE action_message_def::action_t::cmd_null_message
#define CMD_NULL_DEST_MESSAGE action_message_def::action_t::cmd_null_dest_message
#define CMD_FILTER_RESULT action_message_def::action_t::cmd_filter_result
#define CMD_DEST_FILTER_RESULT action_message_def::action_t::cmd_dest_filter_result

#define CMD_PUB action_message_def::action_t::cmd_pub
#define CMD_LOG action_message_def::action_t::cmd_log
#define CMD_WARNING action_message_def::action_t::cmd_warning
#define CMD_ERROR action_message_def::action_t::cmd_error
#define CMD_REG_PUB action_message_def::action_t::cmd_reg_pub
#define CMD_NOTIFY_PUB action_message_def::action_t::cmd_notify_pub

#define CMD_REG_SUB action_message_def::action_t::cmd_reg_sub
#define CMD_NOTIFY_SUB action_message_def::action_t::cmd_notify_sub
#define CMD_REG_END action_message_def::action_t::cmd_reg_end
#define CMD_NOTIFY_END action_message_def::action_t::cmd_notify_end
#define CMD_REG_DST_FILTER action_message_def::action_t::cmd_reg_dst_filter
#define CMD_NOTIFY_DST_FILTER action_message_def::action_t::cmd_notify_dst_filter

#define CMD_REG_SRC_FILTER action_message_def::action_t::cmd_reg_src_filter
#define CMD_NOTIFY_SRC_FILTER action_message_def::action_t::cmd_notify_src_filter
#define CMD_UPDATE_FILTER_OP action_message_def::action_t::cmd_update_filter_op

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

#define CMD_FED_CONFIGURE action_message_def::action_t::cmd_fed_configure
#define CMD_CORE_CONFIGURE action_message_def::action_t::cmd_core_configure

#define CMD_ACK action_message_def::action_t::cmd_ack
#define CMD_PRIORITY_ACK action_message_def::action_t::cmd_priority_ack

#define CMD_QUERY action_message_def::action_t::cmd_query
#define CMD_QUERY_REPLY action_message_def::action_t::cmd_query_reply

// definitions for the protocol options
#define PROTOCOL_PING 10
#define PROTOCOL_PONG 11
#define CLOSE_RECEIVER 23425215
#define NEW_ROUTE 233
#define DISCONNECT 2523
#define DISCONNECT_ERROR 2623

#define NAME_NOT_FOUND 2726
#define RECONNECT 1997
#define RECONNECT_RECEIVER 1999
// for requesting port definitions on a computer
#define PORT_DEFINITIONS 1451
#define QUERY_PORTS 1453
#define REQUEST_PORTS 1455
#define SET_USED_PORTS 1457
#define NULL_REPLY 0;

// definitions for FED_CONFIGURE_COMMAND
#define UPDATE_INPUT_DELAY 0
#define UPDATE_OUTPUT_DELAY 1
#define UPDATE_MINDELTA 2
#define UPDATE_PERIOD 3
#define UPDATE_OFFSET 4
#define UPDATE_MAX_ITERATION 5
#define UPDATE_LOG_LEVEL 6
#define UPDATE_FLAG 7

/** check if the action has an info structure associated with it*/
inline bool hasInfo (action_message_def::action_t action)
{
    return ((action > action_message_def::action_t::null_info_command) ||
            (action < action_message_def::action_t::priority_null_info_command));
}
/** return the name of the action
@param action The action to get the name for
@return a pointer to string with the name
*/
const char *actionMessageType (action_message_def::action_t action);
/** return a string associated with a particular error code
@param errorcode The error to get the string for
@return a pointer to string with the name
*/
const char *commandErrorString(int errorcode);

}  // namespace helics

