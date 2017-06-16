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

ActionMessage::~ActionMessage () {}

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

message_t *createMessage (const ActionMessage &cmd)
{
    auto msg = new message_t;
    char *origSrcCopy = new char[cmd.info ().orig_source.size () + 1];

    strcpy (origSrcCopy, cmd.info ().orig_source.c_str ());

    char *dstCopy = new char[cmd.info ().target.size () + 1];
    strcpy (dstCopy, cmd.info ().target.c_str ());

    char *dataCopy = new char[cmd.payload.size () + 1];
    memcpy (dataCopy, cmd.payload.data (), cmd.payload.size ());

    msg->time = cmd.actionTime;
    msg->data = dataCopy;
    msg->len = cmd.payload.size ();
    msg->origsrc = origSrcCopy;
    if (cmd.info ().source == cmd.info ().orig_source)
    {
        msg->src = msg->origsrc;
    }
    else
    {
        char *srcCopy = new char[cmd.info ().source.size ()];
        strcpy (srcCopy, cmd.info ().source.c_str ());
        msg->src = srcCopy;
    }
    msg->dst = dstCopy;
    return msg;
}

message_t createTempMessage(ActionMessage &cmd)
{
	message_t tMessage;
	tMessage.origsrc = cmd.info().orig_source.c_str();
	if (cmd.info().source == cmd.info().orig_source)
	{
		tMessage.src = tMessage.origsrc;
	}
	else
	{
		tMessage.src = cmd.info().source.c_str();
	}
	tMessage.dst = cmd.info().target.c_str();
	tMessage.len = cmd.payload.size();
	tMessage.data = cmd.payload.data();
	tMessage.time = cmd.actionTime;
	return tMessage;
}
}