/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "ActionMessage.h"
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
    : action_ (startingAction), index (dest_handle), processingComplete (iterationComplete), name (payload)
{
    if (hasInfo (startingAction))
    {
        info_ = std::make_unique<AdditionalInfo> ();
    }
}

ActionMessage::ActionMessage (ActionMessage &&act) noexcept
    : action_ (act.action_), source_id (act.source_id), source_handle (act.source_handle), dest_id (act.dest_id),
      dest_handle (act.dest_handle), index (dest_handle), iterationComplete (act.iterationComplete),
      processingComplete (iterationComplete), required (act.required), error (act.error), flag (act.flag),
      actionTime (act.actionTime), payload (std::move (act.payload)), name (payload), info_ (std::move (act.info_))
{
}

ActionMessage::ActionMessage (const ActionMessage &act)
    : action_ (act.action_), source_id (act.source_id), source_handle (act.source_handle), dest_id (act.dest_id),
      dest_handle (act.dest_handle), index (dest_handle), iterationComplete (act.iterationComplete),
      processingComplete (iterationComplete), required (act.required), error (act.error), flag (act.flag),
      actionTime (act.actionTime), payload (act.payload), name (payload)

{
    if (act.info_)
    {
        info_ = std::make_unique<AdditionalInfo> ((*act.info_));
    }
}

ActionMessage::ActionMessage (std::unique_ptr<Message> message)
    : action_ (CMD_SEND_MESSAGE), index (dest_handle), processingComplete (iterationComplete),
      actionTime (message->time), payload (std::move (message->data.m_data)), name (payload)
{
    info_ = std::make_unique<AdditionalInfo> ();
    info_->source = std::move (message->src);
    info_->orig_source = std::move (message->origsrc);
    info_->target = std::move (message->dest);
}

ActionMessage::ActionMessage (const std::string &bytes) : ActionMessage () { from_string (bytes); }

ActionMessage::ActionMessage (const std::vector<char> &bytes) : ActionMessage () { from_vector (bytes); }

ActionMessage::ActionMessage (const char *data, size_t size) : ActionMessage () { fromByteArray (data, size); }

ActionMessage::~ActionMessage () = default;

ActionMessage &ActionMessage::operator= (const ActionMessage &act)
{
    action_ = act.action_;
    source_id = act.source_id;
    source_handle = act.source_handle;
    dest_id = act.dest_id;
    dest_handle = act.dest_handle;
    iterationComplete = act.iterationComplete;
    required = act.required;
    actionTime = act.actionTime;
    payload = act.payload;

    if (act.info_)
    {
        info_ = std::make_unique<AdditionalInfo> ((*act.info_));
    }
    return *this;
}

ActionMessage &ActionMessage::operator= (ActionMessage &&act) noexcept
{
    action_ = act.action_;
    source_id = act.source_id;
    source_handle = act.source_handle;
    dest_id = act.dest_id;
    dest_handle = act.dest_handle;
    iterationComplete = act.iterationComplete;
    required = act.required;
    actionTime = act.actionTime;
    payload = std::move (act.payload);
    info_ = std::move (act.info_);
    return *this;
}

void ActionMessage::moveInfo (std::unique_ptr<Message> message)
{
    action_ = CMD_SEND_MESSAGE;
    payload = std::move (message->data.m_data);
    actionTime = message->time;
    if (!info_)
    {
        info_ = std::make_unique<AdditionalInfo> ();
    }
    info_->source = std::move (message->src);
    info_->orig_source = std::move (message->origsrc);
    info_->target = std::move (message->dest);
}

void ActionMessage::setAction (action_message_def::action_t newAction)
{
    if (hasInfo (newAction))
    {
        if (!info_)
        {
            info_ = std::make_unique<AdditionalInfo> ();
        }
    }
    action_ = newAction;
}

ActionMessage::AdditionalInfo &ActionMessage::info ()
{
    if (!info_)
    {
        info_ = std::make_unique<AdditionalInfo> ();
    }
    return *info_;
}

const ActionMessage::AdditionalInfo emptyAddInfo;

const ActionMessage::AdditionalInfo &ActionMessage::info () const
{
    if (info_)
    {
        return *info_;
    }
    return emptyAddInfo;
}

using archiver = cereal::PortableBinaryOutputArchive;

using retriever = cereal::PortableBinaryInputArchive;

void ActionMessage::toByteArray (char *data, size_t buffer_size) const
{
    boost::iostreams::basic_array_sink<char> sr (data, buffer_size);
    boost::iostreams::stream<boost::iostreams::basic_array_sink<char>> s (sr);

    archiver oa (s);

    save (oa);
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
    boost::iostreams::basic_array_source<char> device (data, buffer_size);
    boost::iostreams::stream<boost::iostreams::basic_array_source<char>> s (device);
    retriever ia (s);
    load (ia);
}

void ActionMessage::from_string (const std::string &data) { fromByteArray (data.data (), data.size ()); }

void ActionMessage::from_vector (const std::vector<char> &data) { fromByteArray (data.data (), data.size ()); }

std::unique_ptr<Message> createMessage (const ActionMessage &cmd)
{
    auto msg = std::make_unique<Message> ();
    msg->origsrc = cmd.info ().orig_source;
    msg->dest = cmd.info ().target;
    msg->data = cmd.payload;
    msg->time = cmd.actionTime;
    msg->src = cmd.info ().source;

    return msg;
}

std::unique_ptr<Message> createMessage (ActionMessage &&cmd)
{
    auto msg = std::make_unique<Message> ();
    msg->origsrc = std::move (cmd.info ().orig_source);
    msg->dest = std::move (cmd.info ().target);
    msg->data = std::move (cmd.payload);
    msg->time = cmd.actionTime;
    msg->src = std::move (cmd.info ().source);

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

  {action_message_def::action_t::null_info_command, "null_info"},
  {action_message_def::action_t::priority_null_info_command, "priority_null_info"},
  {action_message_def::action_t::cmd_time_request, "time_request"},
  {action_message_def::action_t::cmd_send_message, "send_message"},
  {action_message_def::action_t::cmd_send_for_filter, "send_for_filter"},
  {action_message_def::action_t::cmd_send_for_filter_op, "send_for_filter_op"},

  {action_message_def::action_t::cmd_reg_pub, "reg_pub"},
  {action_message_def::action_t::cmd_notify_pub, "notify_pub"},
  {action_message_def::action_t::cmd_reg_dst_filter, "reg_dst_filter"},
  {action_message_def::action_t::cmd_notify_dst_filter, "notify_dst_filter"},
  {action_message_def::action_t::cmd_reg_sub, "reg_sub"},
  {action_message_def::action_t::cmd_notify_sub, "notify_sub"},
  {action_message_def::action_t::cmd_reg_src_filter, "reg_src_filter"},
  {action_message_def::action_t::cmd_notify_src_filter, "notify_src_filter"},
  {action_message_def::action_t::cmd_src_filter_has_operator, "src_filter_has_operator"},
  {action_message_def::action_t::cmd_reg_end, "reg_end"},
  {action_message_def::action_t::cmd_notify_end, "notify_end"},

  {action_message_def::action_t::cmd_has_operator, "has_operator"},
  // protocol messages are meant for the communication standard and are not used in the Cores/Brokers
  {action_message_def::action_t::cmd_protocol_priority, "protocol_priority"},
  {action_message_def::action_t::cmd_protocol, "protocol"},
  {action_message_def::action_t::cmd_protocol_big, "protocol_big"}};

using actionPair = std::pair<action_message_def::action_t, const char *>;
constexpr size_t actEnd = sizeof (actionStrings) / sizeof (actionPair);

std::string actionMessageType (action_message_def::action_t action)
{
    auto pptr = static_cast<const actionPair *> (actionStrings);
    auto res = std::find_if (pptr, pptr + actEnd, [action](const auto &pt) { return (pt.first == action); });
    if (res != pptr + actEnd)
    {
        return std::string (res->second);
    }
    return std::string (static_cast<const char *> (nullStr));
}

std::string prettyPrintString (const ActionMessage &command)
{
    std::string ret = actionMessageType (command.action ());
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
        if (command.error)
        {
            ret.append ("error");
        }
        else
        {
            ret.append (std::to_string (command.dest_id));
        }
        break;
    case CMD_REG_BROKER:
        ret.push_back (':');
        ret.append (command.name);
        break;
    case CMD_TIME_REQUEST:
        ret.push_back (':');
        ret.append ((boost::format ("From (%d) Time(%f, %f, %f)") % command.source_id %
                     static_cast<double> (command.actionTime) % static_cast<double> (command.info ().Te) %
                     static_cast<double> (command.info ().Tdemin))
                      .str ());
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