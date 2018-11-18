/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "ActionMessage.hpp"
#include "flagOperations.hpp"
#include <cereal/archives/portable_binary.hpp>
#include <complex>
//#include <cereal/archives/binary.hpp>
#include "../common/fmt_format.h"
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>

#include <algorithm>

namespace helics
{
ActionMessage::ActionMessage (action_message_def::action_t startingAction)
    : messageAction (startingAction), name (payload)
{
}

ActionMessage::ActionMessage (action_message_def::action_t startingAction,
                              global_federate_id_t sourceId,
                              global_federate_id_t destId)
    : ActionMessage (startingAction)
{
    source_id = sourceId;
    dest_id = destId;
}

ActionMessage::ActionMessage (ActionMessage &&act) noexcept
    : messageAction (act.messageAction), messageID (act.messageID), source_id (act.source_id),
      source_handle (act.source_handle), dest_id (act.dest_id), dest_handle (act.dest_handle),
      counter (act.counter), flags (act.flags), actionTime (act.actionTime), Te (act.Te), Tdemin (act.Tdemin),
      payload (std::move (act.payload)), name (payload), stringData (std::move (act.stringData))
{
}

ActionMessage::ActionMessage (const ActionMessage &act)
    : messageAction (act.messageAction), messageID (act.messageID), source_id (act.source_id),
      source_handle (act.source_handle), dest_id (act.dest_id), dest_handle (act.dest_handle),
      counter (act.counter), flags (act.flags), actionTime (act.actionTime), Te (act.Te), Tdemin (act.Tdemin),
      payload (act.payload), name (payload), stringData (act.stringData)

{
}

ActionMessage::ActionMessage (std::unique_ptr<Message> message)
    : messageAction (CMD_SEND_MESSAGE), messageID (message->messageID), actionTime (message->time),
      payload (std::move (message->data.m_data)), name (payload),
      stringData ({std::move (message->dest), std::move (message->source), std::move (message->original_source),
                   std::move (message->original_dest)})
{
}

ActionMessage::ActionMessage (const std::string &bytes) : ActionMessage () { from_string (bytes); }

ActionMessage::ActionMessage (const std::vector<char> &bytes) : ActionMessage () { from_vector (bytes); }

ActionMessage::ActionMessage (const char *data, size_t size) : ActionMessage () { fromByteArray (data, size); }

ActionMessage::~ActionMessage () = default;

ActionMessage &ActionMessage::operator= (const ActionMessage &act)
{
    messageAction = act.messageAction;
    messageID = act.messageID;
    source_id = act.source_id;
    source_handle = act.source_handle;
    dest_id = act.dest_id;
    dest_handle = act.dest_handle;
    counter = act.counter;
    flags = act.flags;
    actionTime = act.actionTime;
    Te = act.Te;
    Tdemin = act.Tdemin;
    payload = act.payload;
    stringData = act.stringData;
    return *this;
}

ActionMessage &ActionMessage::operator= (ActionMessage &&act) noexcept
{
    messageAction = act.messageAction;
    messageID = act.messageID;
    source_id = act.source_id;
    source_handle = act.source_handle;
    dest_id = act.dest_id;
    dest_handle = act.dest_handle;
    counter = act.counter;
    flags = act.flags;
    actionTime = act.actionTime;
    Te = act.Te;
    Tdemin = act.Tdemin;
    payload = std::move (act.payload);
    stringData = std::move (act.stringData);
    return *this;
}

void ActionMessage::moveInfo (std::unique_ptr<Message> message)
{
    messageAction = CMD_SEND_MESSAGE;
    messageID = message->messageID;
    payload = std::move (message->data.m_data);
    actionTime = message->time;
    stringData = {std::move (message->dest), std::move (message->source), std::move (message->original_source),
                  std::move (message->original_dest)};
}

void ActionMessage::setAction (action_message_def::action_t newAction) { messageAction = newAction; }

static const std::string emptyStr;
const std::string &ActionMessage::getString (int index) const
{
    if (isValidIndex (index, stringData))
    {
        return stringData[index];
    }
    return emptyStr;
}

void ActionMessage::setString (int index, const std::string &str)
{
    if (index >= 0)
    {
        if (index >= static_cast<int> (stringData.size ()))
        {
            stringData.resize (index + 1);
        }
        stringData[index] = str;
    }
}
using archiver = cereal::PortableBinaryOutputArchive;

using retriever = cereal::PortableBinaryInputArchive;

int ActionMessage::toByteArray (char *data, size_t buffer_size) const
{
    if ((data == nullptr) || (buffer_size == 0))
    {
        return -1;
    }
    boost::iostreams::basic_array_sink<char> sr (data, buffer_size);
    boost::iostreams::stream<boost::iostreams::basic_array_sink<char>> s (sr);

    archiver oa (s);
    try
    {
        save (oa);
        return static_cast<int> (boost::iostreams::seek (s, 0, std::ios_base::cur));
    }
    catch (const std::ios_base::failure &)
    {
        return -1;
    }
}

std::string ActionMessage::to_string () const
{
    std::string data;
    boost::iostreams::back_insert_device<std::string> inserter (data);
    boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> s (inserter);
    archiver oa (s);

    save (oa);

    // don't forget to flush the stream to finish writing into the buffer
    s.flush ();
    return data;
}

constexpr auto LEADING_CHAR = '\xF3';
constexpr auto TAIL_CHAR1 = '\xFA';
constexpr auto TAIL_CHAR2 = '\xFC';

std::string ActionMessage::packetize () const
{
    std::string data;
    data.push_back (LEADING_CHAR);
    data.resize (4);
    boost::iostreams::back_insert_device<std::string> inserter (data);
    boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> s (inserter);
    archiver oa (s);

    save (oa);

    // don't forget to flush the stream to finish writing into the buffer
    s.flush ();
    // now generate a length header
    auto sz = static_cast<int32_t> (data.size ());
    data[1] = static_cast<char> (((sz >> 16) & 0xFF));
    data[2] = static_cast<char> (((sz >> 8) & 0xFF));
    data[3] = static_cast<char> (sz & 0xFF);
    data.push_back (TAIL_CHAR1);
    data.push_back (TAIL_CHAR2);
    return data;
}

std::vector<char> ActionMessage::to_vector () const
{
    std::vector<char> data;
    boost::iostreams::back_insert_device<std::vector<char>> inserter (data);
    boost::iostreams::stream<boost::iostreams::back_insert_device<std::vector<char>>> s (inserter);
    archiver oa (s);

    save (oa);

    // don't forget to flush the stream to finish writing into the buffer
    s.flush ();
    return data;
}

void ActionMessage::to_vector (std::vector<char> &data) const
{
    data.clear ();
    boost::iostreams::back_insert_device<std::vector<char>> inserter (data);
    boost::iostreams::stream<boost::iostreams::back_insert_device<std::vector<char>>> s (inserter);
    archiver oa (s);

    save (oa);

    // don't forget to flush the stream to finish writing into the buffer
    s.flush ();
}

void ActionMessage::to_string (std::string &data) const
{
    data.clear ();

    boost::iostreams::back_insert_device<std::string> inserter (data);
    boost::iostreams::stream<boost::iostreams::back_insert_device<std::string>> s (inserter);
    archiver oa (s);

    save (oa);

    // don't forget to flush the stream to finish writing into the buffer
    s.flush ();
}

void ActionMessage::fromByteArray (const char *data, size_t buffer_size)
{
    if (data[0] == LEADING_CHAR)
    {
        auto res = depacketize (data, buffer_size);
        if (res > 0)
        {
            return;
        }
    }
    boost::iostreams::basic_array_source<char> device (data, buffer_size);
    boost::iostreams::stream<boost::iostreams::basic_array_source<char>> s (device);
    retriever ia (s);
    try
    {
        load (ia);
    }
    catch (const cereal::Exception &ce)
    {
        messageAction = CMD_INVALID;
    }
}

size_t ActionMessage::depacketize (const char *data, size_t buffer_size)
{
    if (data[0] != LEADING_CHAR)
    {
        return 0;
    }
    if (buffer_size < 6)
    {
        return 0;
    }
    size_t message_size = static_cast<unsigned char> (data[1]);
    message_size <<= 8;
    message_size += static_cast<unsigned char> (data[2]);
    message_size <<= 8;
    message_size += static_cast<unsigned char> (data[3]);
    if (buffer_size < message_size + 2)
    {
        return 0;
    }
    if (data[message_size] != TAIL_CHAR1)
    {
        return 0;
    }
    if (data[message_size + 1] != TAIL_CHAR2)
    {
        return 0;
    }
    boost::iostreams::basic_array_source<char> device (data + 4, message_size);
    boost::iostreams::stream<boost::iostreams::basic_array_source<char>> s (device);
    retriever ia (s);
    try
    {
        load (ia);
        return message_size + 2;
    }
    catch (const cereal::Exception &ce)
    {
        messageAction = CMD_INVALID;
        return 0;
    }
}

void ActionMessage::from_string (const std::string &data) { fromByteArray (data.data (), data.size ()); }

void ActionMessage::from_vector (const std::vector<char> &data) { fromByteArray (data.data (), data.size ()); }

std::unique_ptr<Message> createMessageFromCommand (const ActionMessage &cmd)
{
    auto msg = std::make_unique<Message> ();
    switch (cmd.stringData.size ())
    {
    case 0:
        break;
    case 1:
        msg->dest = cmd.stringData[0];
        break;
    case 2:
        msg->dest = cmd.stringData[0];
        msg->source = cmd.stringData[1];
        break;
    case 3:
        msg->dest = cmd.stringData[0];
        msg->source = cmd.stringData[1];
        msg->original_source = cmd.stringData[2];
        break;
    default:
        msg->dest = cmd.stringData[0];
        msg->source = cmd.stringData[1];
        msg->original_source = cmd.stringData[2];
        msg->original_dest = cmd.stringData[3];
        break;
    }
    msg->data = cmd.payload;
    msg->time = cmd.actionTime;
    msg->messageID = cmd.messageID;

    return msg;
}

std::unique_ptr<Message> createMessageFromCommand (ActionMessage &&cmd)
{
    auto msg = std::make_unique<Message> ();
    switch (cmd.stringData.size ())
    {
    case 0:
        break;
    case 1:
        msg->dest = std::move (cmd.stringData[0]);
        break;
    case 2:
        msg->dest = std::move (cmd.stringData[0]);
        msg->source = std::move (cmd.stringData[1]);
        break;
    case 3:
        msg->dest = std::move (cmd.stringData[0]);
        msg->source = std::move (cmd.stringData[1]);
        msg->original_source = std::move (cmd.stringData[2]);
        break;
    default:
        msg->dest = std::move (cmd.stringData[0]);
        msg->source = std::move (cmd.stringData[1]);
        msg->original_source = std::move (cmd.stringData[2]);
        msg->original_dest = std::move (cmd.stringData[3]);
        break;
    }
    msg->data = std::move (cmd.payload);
    msg->time = cmd.actionTime;
    msg->messageID = cmd.messageID;
    return msg;
}

static constexpr char unknownStr[] = "unknown";

// done in this screwy way because this can be called after things have started to be deconstructed so static
// consts can cause seg faults

static constexpr std::pair<action_message_def::action_t, const char *> actionStrings[] = {
  // priority commands
  {action_message_def::action_t::cmd_priority_disconnect, "priority_disconnect"},
  {action_message_def::action_t::cmd_disconnect, "disconnect"},
  {action_message_def::action_t::cmd_disconnect_name, "disconnect by name"},
{ action_message_def::action_t::cmd_user_disconnect, "disconnect from user" },
  {action_message_def::action_t::cmd_fed_ack, "fed_ack"},

  {action_message_def::action_t::cmd_broker_ack, "broker_ack"},
  {action_message_def::action_t::cmd_add_route, "add_route"},
  {action_message_def::action_t::cmd_route_ack, "route_ack"},
  {action_message_def::action_t::cmd_register_route, "register_route"},
  {action_message_def::action_t::cmd_reg_fed, "reg_fed"},
  {action_message_def::action_t::cmd_priority_ack, "priority_ack"},
  {action_message_def::action_t::cmd_query, "query"},
  {action_message_def::action_t::cmd_query_reply, "query_reply"},
  {action_message_def::action_t::cmd_reg_broker, "reg_broker"},

  {action_message_def::action_t::cmd_ignore, "ignore"},
  {action_message_def::action_t::cmd_tick, "tick"},
  {action_message_def::action_t::cmd_ping, "ping"},
  {action_message_def::action_t::cmd_ping_reply, "ping reply"},
  {action_message_def::action_t::cmd_fed_configure_time, "fed_configure_time"},
  {action_message_def::action_t::cmd_fed_configure_int, "fed_configure_int"},
  {action_message_def::action_t::cmd_fed_configure_flag, "fed_configure_flag"},
  {action_message_def::action_t::cmd_init, "init"},
  {action_message_def::action_t::cmd_init_grant, "init_grant"},
  {action_message_def::action_t::cmd_init_not_ready, "init_not_ready"},
  {action_message_def::action_t::cmd_exec_request, "exec_request"},
  {action_message_def::action_t::cmd_exec_grant, "exec_grant"},
  {action_message_def::action_t::cmd_exec_check, "exec_check"},
  {action_message_def::action_t::cmd_ack, "ack"},

  {action_message_def::action_t::cmd_stop, "stop"},
  {action_message_def::action_t::cmd_terminate_immediately, "terminate_immediately"},

  {action_message_def::action_t::cmd_time_grant, "time_grant"},
  {action_message_def::action_t::cmd_time_check, "time_check"},
  {action_message_def::action_t::cmd_time_block, "time_block"},
  {action_message_def::action_t::cmd_time_unblock, "time_unblock"},
  {action_message_def::action_t::cmd_pub, "pub"},
  {action_message_def::action_t::cmd_bye, "bye"},
  {action_message_def::action_t::cmd_log, "log"},
  {action_message_def::action_t::cmd_warning, "warning"},
  {action_message_def::action_t::cmd_error, "error"},

  {action_message_def::action_t::cmd_send_route, "send_route"},
  {action_message_def::action_t::cmd_add_dependency, "add_dependency"},
  {action_message_def::action_t::cmd_remove_dependency, "remove_dependency"},
  {action_message_def::action_t::cmd_add_dependent, "add_dependent"},
  {action_message_def::action_t::cmd_remove_dependent, "remove_dependent"},
  {action_message_def::action_t::cmd_add_interdependency, "add_interdependency"},
  {action_message_def::action_t::cmd_remove_interdependency, "remove_interdependency"},

  {action_message_def::action_t::null_info_command, "null_info"},
  {action_message_def::action_t::priority_null_info_command, "priority_null_info"},
  {action_message_def::action_t::cmd_time_request, "time_request"},
  {action_message_def::action_t::cmd_send_message, "send_message"},
  {action_message_def::action_t::cmd_send_for_filter, "send_for_filter"},
  {action_message_def::action_t::cmd_filter_result, "result from running a filter"},
  {action_message_def::action_t::cmd_send_for_filter_return, "send_for_filter_return"},
  {action_message_def::action_t::cmd_null_message, "null message"},

  {action_message_def::action_t::cmd_reg_pub, "reg_pub"},
  {action_message_def::action_t::cmd_add_publisher, "add publisher"},
  {action_message_def::action_t::cmd_reg_filter, "reg_filter"},
  {action_message_def::action_t::cmd_add_filter, "add_filter"},
  {action_message_def::action_t::cmd_reg_input, "reg_input"},
  {action_message_def::action_t::cmd_add_subscriber, "add_subscriber"},
  {action_message_def::action_t::cmd_reg_end, "reg_end"},
  {action_message_def::action_t::cmd_resend, "reg_resend"},
  {action_message_def::action_t::cmd_add_endpoint, "add_endpoint"},
  {action_message_def::action_t::cmd_add_named_endpoint, "add_named_endpoint"},
  {action_message_def::action_t::cmd_add_named_input, "add_named_input"},
  {action_message_def::action_t::cmd_add_named_publication, "add_named_publication"},
  {action_message_def::action_t::cmd_add_named_filter, "add_named_filter"},
  {action_message_def::action_t::cmd_remove_target, "remove_target"},
  {action_message_def::action_t::cmd_multi_message, "multi message"},
  // protocol messages are meant for the communication standard and are not used in the Cores/Brokers
  {action_message_def::action_t::cmd_protocol_priority, "protocol_priority"},
  {action_message_def::action_t::cmd_protocol, "protocol"},
  {action_message_def::action_t::cmd_protocol_big, "protocol_big"}};

using actionPair = std::pair<action_message_def::action_t, const char *>;
static constexpr size_t actEnd = sizeof (actionStrings) / sizeof (actionPair);
// this was done this way to keep the string array as a constexpr otherwise it could be deleted as this function
// can (in actuality) be used as the program is shutting down
const char *actionMessageType (action_message_def::action_t action)
{
    auto pptr = static_cast<const actionPair *> (actionStrings);
    auto res = std::find_if (pptr, pptr + actEnd, [action](const auto &pt) { return (pt.first == action); });
    if (res != pptr + actEnd)
    {
        return res->second;
    }  
    return static_cast<const char *> (unknownStr);
}

static constexpr std::pair<int, const char *> errorStrings[] = {
  // priority commands
  {-5, "lost connection with server"},
  {5, "already in initialization mode"},
  {6, "duplicate federate name detected"},
{ 7, "duplicate broker name detected" }
};

using errorPair = std::pair<int, const char *>;
static constexpr size_t errEnd = sizeof (errorStrings) / sizeof (errorPair);

// this was done this way to keep the string array as a constexpr otherwise it could be deleted as this function
// can (in actuality) be used as the program is shutting down
const char *commandErrorString (int errorcode)
{
    auto pptr = static_cast<const errorPair *> (errorStrings);
    auto res = std::find_if (pptr, pptr + errEnd, [errorcode](const auto &pt) { return (pt.first == errorcode); });
    if (res != pptr + errEnd)
    {
        return res->second;
    }
    return static_cast<const char *> (unknownStr);
}

std::string prettyPrintString (const ActionMessage &command)
{
    std::string ret (actionMessageType (command.action ()));
	if (ret == unknownStr)
	{
        ret += " " + std::to_string (static_cast<int> (command.action ()));
        return ret;
	}
    switch (command.action ())
    {
    case CMD_REG_FED:
        ret.push_back (':');
        ret.append (command.name);
        break;
    case CMD_FED_ACK:
        ret.push_back (':');
        ret.append (command.name);
        ret.append ("--");
        if (checkActionFlag (command, error_flag))
        {
            ret.append ("error");
        }
        else
        {
            ret.append (std::to_string (command.dest_id.baseValue ()));
        }
        break;
    case CMD_PUB:
        ret.push_back (':');
        ret.append (fmt::format ("From ({}) handle({}) size {} at {} to {}", command.source_id.baseValue (),
                                 command.dest_handle.baseValue (), command.payload.size (),
                                 static_cast<double> (command.actionTime), command.dest_id.baseValue ()));
        break;
    case CMD_REG_BROKER:
        ret.push_back (':');
        ret.append (command.name);
        break;
    case CMD_TIME_GRANT:
        ret.push_back (':');
        ret.append (fmt::format ("From ({}) Granted Time({}) to ({})", command.source_id.baseValue (),
                                 static_cast<double> (command.actionTime), command.dest_id.baseValue ()));
        break;
    case CMD_TIME_REQUEST:
        ret.push_back (':');
        ret.append (fmt::format ("From ({}) Time({}, {}, {}) to ({})", command.source_id.baseValue (),
                                 static_cast<double> (command.actionTime), static_cast<double> (command.Te),
                                 static_cast<double> (command.Tdemin), command.dest_id.baseValue ()));
        break;
    case CMD_FED_CONFIGURE_TIME:
    case CMD_FED_CONFIGURE_INT:
    case CMD_FED_CONFIGURE_FLAG:
        break;
    case CMD_SEND_MESSAGE:
        ret.push_back (':');
        ret.append (fmt::format ("From ({})({}:{}) To {} size {} at {}", command.getString (origSourceStringLoc),
                                 command.source_id.baseValue (), command.source_handle.baseValue (),
                                 command.getString (targetStringLoc), command.payload.size (),
                                 static_cast<double> (command.actionTime)));
        break;
    default:
        ret.append (fmt::format (":From {}", command.source_id.baseValue ()));
        break;
    }
    return ret;
}

std::ostream &operator<< (std::ostream &os, const ActionMessage &command)
{
    os << prettyPrintString (command);
    return os;
}

int appendMessage (ActionMessage &m, const ActionMessage &newMessage)
{
    if (m.action () == CMD_MULTI_MESSAGE)
    {
        m.setString (m.counter++, newMessage.to_string ());
        return m.counter;
    }
    return (-1);
}
}  // namespace helics
