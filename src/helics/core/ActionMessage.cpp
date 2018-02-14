/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "ActionMessage.hpp"
#include <cereal/archives/portable_binary.hpp>
#include <complex>
//#include <cereal/archives/binary.hpp>
#include <boost/format.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>

#include <algorithm>

namespace helics
{
ActionMessage::ActionMessage (action_message_def::action_t startingAction)
    : messageAction (startingAction), index (dest_handle), name (payload)
{
    if (hasInfo (startingAction))
    {
        extraInfo = std::make_unique<AdditionalInfo> ();
    }
}

ActionMessage::ActionMessage (action_message_def::action_t startingAction, int32_t sourceId, int32_t destId)
    : ActionMessage (startingAction)
{
    source_id = sourceId;
    dest_id = destId;
}

ActionMessage::ActionMessage (ActionMessage &&act) noexcept
    : messageAction (act.messageAction), source_id (act.source_id), source_handle (act.source_handle), dest_id (act.dest_id),
      dest_handle (act.dest_handle), index (dest_handle), counter (act.counter), flags (act.flags),
      actionTime (act.actionTime), Te (act.Te), Tdemin (act.Tdemin), payload (std::move (act.payload)),
      name (payload), extraInfo (std::move (act.extraInfo))
{
}

ActionMessage::ActionMessage (const ActionMessage &act)
    : messageAction (act.messageAction), source_id (act.source_id), source_handle (act.source_handle), dest_id (act.dest_id),
      dest_handle (act.dest_handle), index (dest_handle), counter (act.counter), flags (act.flags),
      actionTime (act.actionTime), Te (act.Te), Tdemin (act.Tdemin), payload (act.payload), name (payload)

{
    if (act.extraInfo)
    {
        extraInfo = std::make_unique<AdditionalInfo> ((*act.extraInfo));
    }
}

ActionMessage::ActionMessage (std::unique_ptr<Message> message)
    : messageAction (CMD_SEND_MESSAGE), index (dest_handle), actionTime (message->time),
      payload (std::move (message->data.m_data)), name (payload)
{
    extraInfo = std::make_unique<AdditionalInfo> ();
    extraInfo->source = std::move (message->source);
    extraInfo->orig_source = std::move (message->original_source);
    extraInfo->original_dest = std::move (message->original_dest);
    extraInfo->target = std::move (message->dest);
    extraInfo->messageID = message->messageID;
}

ActionMessage::ActionMessage (const std::string &bytes) : ActionMessage () { from_string (bytes); }

ActionMessage::ActionMessage (const std::vector<char> &bytes) : ActionMessage () { from_vector (bytes); }

ActionMessage::ActionMessage (const char *data, size_t size) : ActionMessage () { fromByteArray (data, size); }

ActionMessage::~ActionMessage () = default;

ActionMessage &ActionMessage::operator= (const ActionMessage &act)
{
    messageAction = act.messageAction;
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

    if (act.extraInfo)
    {
        extraInfo = std::make_unique<AdditionalInfo> ((*act.extraInfo));
    }
    return *this;
}

ActionMessage &ActionMessage::operator= (ActionMessage &&act) noexcept
{
    messageAction = act.messageAction;
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
    extraInfo = std::move (act.extraInfo);
    return *this;
}

void ActionMessage::moveInfo (std::unique_ptr<Message> message)
{
    messageAction = CMD_SEND_MESSAGE;
    payload = std::move (message->data.m_data);
    actionTime = message->time;
    if (!extraInfo)
    {
        extraInfo = std::make_unique<AdditionalInfo> ();
    }
    extraInfo->source = std::move (message->source);
    extraInfo->orig_source = std::move (message->original_source);
    extraInfo->original_dest = std::move (message->original_dest);
    extraInfo->target = std::move (message->dest);
    extraInfo->messageID = message->messageID;
}

void ActionMessage::setAction (action_message_def::action_t newAction)
{
    if (hasInfo (newAction))
    {
        if (!extraInfo)
        {
            extraInfo = std::make_unique<AdditionalInfo> ();
        }
    }
    messageAction = newAction;
}

ActionMessage::AdditionalInfo &ActionMessage::info ()
{
    if (!extraInfo)
    {
        extraInfo = std::make_unique<AdditionalInfo> ();
    }
    return *extraInfo;
}

const ActionMessage::AdditionalInfo emptyAddInfo;

const ActionMessage::AdditionalInfo &ActionMessage::info () const
{
    if (extraInfo)
    {
        return *extraInfo;
    }
    return emptyAddInfo;
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
    msg->original_source = cmd.info ().orig_source;
    msg->original_dest = cmd.info ().original_dest;
    msg->dest = cmd.info ().target;
    msg->data = cmd.payload;
    msg->time = cmd.actionTime;
    msg->messageID = cmd.info().messageID;
    msg->source = cmd.info ().source;

    return msg;
}

std::unique_ptr<Message> createMessageFromCommand (ActionMessage &&cmd)
{
    auto msg = std::make_unique<Message> ();
    msg->original_source = std::move (cmd.info ().orig_source);
    msg->original_dest = std::move (cmd.info ().original_dest);
    msg->dest = std::move (cmd.info ().target);
    msg->data = std::move (cmd.payload);
    msg->time = cmd.actionTime;
    msg->source = std::move (cmd.info ().source);
    msg->messageID = cmd.info().messageID;
    return msg;
}

constexpr char nullStr[] = "unknown";

constexpr std::pair<action_message_def::action_t, const char *> actionStrings[] = {
  // priority commands
  {action_message_def::action_t::cmd_priority_disconnect, "priority_disconnect"},
  {action_message_def::action_t::cmd_disconnect, "disconnect"},
  {action_message_def::action_t::cmd_disconnect_name, "disconnect by name"},
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
  {action_message_def::action_t::cmd_fed_configure, "fed_configure"},
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
  {action_message_def::action_t::cmd_pub, "pub"},
  {action_message_def::action_t::cmd_bye, "bye"},
  {action_message_def::action_t::cmd_log, "log"},
  {action_message_def::action_t::cmd_warning, "warning"},
  {action_message_def::action_t::cmd_error, "error"},

  {action_message_def::action_t::cmd_send_route, "send_route"},
  {action_message_def::action_t::cmd_subscriber, "subscriber"},
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
  {action_message_def::action_t::cmd_null_message, "null message" },

  {action_message_def::action_t::cmd_reg_pub, "reg_pub"},
  {action_message_def::action_t::cmd_notify_pub, "notify_pub"},
  {action_message_def::action_t::cmd_reg_dst_filter, "reg_dst_filter"},
  {action_message_def::action_t::cmd_notify_dst_filter, "notify_dst_filter"},
  {action_message_def::action_t::cmd_reg_sub, "reg_sub"},
  {action_message_def::action_t::cmd_notify_sub, "notify_sub"},
  {action_message_def::action_t::cmd_reg_src_filter, "reg_src_filter"},
  {action_message_def::action_t::cmd_notify_src_filter, "notify_src_filter"},
  {action_message_def::action_t::cmd_reg_end, "reg_end"},
  {action_message_def::action_t::cmd_notify_end, "notify_end"},

  {action_message_def::action_t::cmd_has_operator, "has_operator"},
  // protocol messages are meant for the communication standard and are not used in the Cores/Brokers
  {action_message_def::action_t::cmd_protocol_priority, "protocol_priority"},
  {action_message_def::action_t::cmd_protocol, "protocol"},
  {action_message_def::action_t::cmd_protocol_big, "protocol_big"}};

using actionPair = std::pair<action_message_def::action_t, const char *>;
constexpr size_t actEnd = sizeof (actionStrings) / sizeof (actionPair);

const char *actionMessageType (action_message_def::action_t action)
{
    auto pptr = static_cast<const actionPair *> (actionStrings);
    auto res = std::find_if (pptr, pptr + actEnd, [action](const auto &pt) { return (pt.first == action); });
    if (res != pptr + actEnd)
    {
        return res->second;
    }
    return static_cast<const char *> (nullStr);
}

std::string prettyPrintString (const ActionMessage &command)
{
    std::string ret (actionMessageType (command.action ()));
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
            ret.append (std::to_string (command.dest_id));
        }
        break;
    case CMD_PUB:
        ret.push_back (':');
        ret.append ((boost::format ("From (%d) handle(%d) size %d at %f") % command.source_id %
                     command.dest_handle % command.payload.size () % static_cast<double> (command.actionTime))
                      .str ());
        break;
    case CMD_REG_BROKER:
        ret.push_back (':');
        ret.append (command.name);
        break;
    case CMD_TIME_GRANT:
        ret.push_back (':');
        ret.append ((boost::format ("From (%d) Granted Time(%f)") % command.source_id %
                     static_cast<double> (command.actionTime))
                      .str ());
        break;
    case CMD_TIME_REQUEST:
        ret.push_back (':');
        ret.append ((boost::format ("From (%d) Time(%f, %f, %f)") % command.source_id %
                     static_cast<double> (command.actionTime) % static_cast<double> (command.Te) %
                     static_cast<double> (command.Tdemin))
                      .str ());
        break;
    case CMD_FED_CONFIGURE:
        break;
    case CMD_SEND_MESSAGE:
        ret.push_back(':');
        ret.append((boost::format("From (%s)(%d:%d) To %s size %d at %f") % command.info().orig_source%command.source_id % command.source_handle %
            command.info().target % command.payload.size() % static_cast<double> (command.actionTime))
            .str());
        break;
    default:
        break;
    }
    return ret;
}

std::ostream &operator<< (std::ostream &os, const ActionMessage &command)
{
    os << prettyPrintString (command);
    return os;
}
}  // namespace helics
