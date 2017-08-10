/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "ActionMessage.h"

namespace helics
{
ActionMessage::ActionMessage (action_t action) : action_ (action),name(payload)
{
	if (action >= cmd_info_basis)
	{
		info_ = std::make_unique<AdditionalInfo>();
	}
}

ActionMessage::ActionMessage (ActionMessage &&act) noexcept
    : action_ (act.action_), source_id (act.source_id), source_handle (act.source_handle), dest_id (act.dest_id),
      dest_handle (act.dest_handle), iterationComplete (act.iterationComplete), required (act.required),
	  error(act.error), flag(act.flag), actionTime(act.actionTime), payload(std::move(act.payload)), name(payload),
      info_(std::move(act.info_))
{
}

ActionMessage::ActionMessage (const ActionMessage &act)
    : action_ (act.action_), source_id (act.source_id), source_handle (act.source_handle), dest_id (act.dest_id),
      dest_handle (act.dest_handle), iterationComplete (act.iterationComplete), required (act.required),
	  error(act.error), flag(act.flag), actionTime(act.actionTime), payload(act.payload), name(payload)

{
    if (act.info_)
    {
        info_ = std::make_unique<AdditionalInfo> ((*act.info_));
    }
}

ActionMessage::ActionMessage(std::unique_ptr<Message> message):action_(action_t::cmd_send_message), actionTime(message->time), payload(std::move(message->data.to_string())),name(payload)
{
	info_ = std::make_unique<AdditionalInfo>();
	info_->source = std::move(message->src);
	info_->orig_source = std::move(message->origsrc);
	info_->target = std::move(message->dest);

}

ActionMessage::~ActionMessage() = default;

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
    act.info_ = std::move (act.info_);
    return *this;
}

void ActionMessage::moveInfo(std::unique_ptr<Message> message)
{
	
	action_ = action_t::cmd_send_message;
	payload = std::move(message->data.to_string());
	actionTime = message->time;
	if (!info_)
	{
		info_ = std::make_unique<AdditionalInfo>();
	}
	info_->source = std::move(message->src);
	info_->orig_source = std::move(message->origsrc);
	info_->target = std::move(message->dest);

}

void ActionMessage::setAction(action_t action)
{
	if (action >= cmd_info_basis)
	{
		if (!info_)
		{
			info_ = std::make_unique<AdditionalInfo>();
		}
	}
	action_ = action;
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

const ActionMessage::AdditionalInfo &ActionMessage::info() const
{
	if (info_)
	{
		return *info_;
	}
	return emptyAddInfo;
}

std::unique_ptr<Message> createMessage (const ActionMessage &cmd)
{
	auto msg = std::make_unique<Message>();
	msg->origsrc = cmd.info().orig_source;
	msg->dest = cmd.info().target;
	msg->data = cmd.payload;
	msg->time = cmd.actionTime;
	msg->src = cmd.info().source;
	
    return msg;
}

std::unique_ptr<Message> createMessage(ActionMessage &&cmd)
{
	auto msg = std::make_unique<Message>();
	msg->origsrc = std::move(cmd.info().orig_source);
	msg->dest = std::move(cmd.info().target);
	msg->data = std::move(cmd.payload);
	msg->time = cmd.actionTime;
	msg->src = std::move(cmd.info().source);

	return msg;
}

}