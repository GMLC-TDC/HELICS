/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "ActionMessage.h"
#include <complex>
#include <cereal/archives/portable_binary.hpp>
//#include <cereal/archives/binary.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

namespace helics
{
ActionMessage::ActionMessage (action_message_def::action_t action) : action_ (action), index(dest_handle), processingComplete(iterationComplete), name(payload)
{
	if (hasInfo(action))
	{
		info_ = std::make_unique<AdditionalInfo>();
	}
}

ActionMessage::ActionMessage (ActionMessage &&act) noexcept
    : action_ (act.action_), source_id (act.source_id), source_handle (act.source_handle), dest_id (act.dest_id),
      dest_handle (act.dest_handle), index(dest_handle), iterationComplete (act.iterationComplete), processingComplete(iterationComplete), required (act.required),
	  error(act.error), flag(act.flag), actionTime(act.actionTime), payload(std::move(act.payload)), name(payload),
      info_(std::move(act.info_))
{
}

ActionMessage::ActionMessage (const ActionMessage &act)
    : action_ (act.action_), source_id (act.source_id), source_handle (act.source_handle), dest_id (act.dest_id),
      dest_handle (act.dest_handle), index(dest_handle), iterationComplete (act.iterationComplete), processingComplete(iterationComplete), required (act.required),
	  error(act.error), flag(act.flag), actionTime(act.actionTime), payload(act.payload), name(payload)

{
    if (act.info_)
    {
        info_ = std::make_unique<AdditionalInfo> ((*act.info_));
    }
}

ActionMessage::ActionMessage(std::unique_ptr<Message> message):action_(CMD_SEND_MESSAGE), index(dest_handle), processingComplete(iterationComplete), actionTime(message->time), payload(std::move(message->data.to_string())),name(payload)
{
	info_ = std::make_unique<AdditionalInfo>();
	info_->source = std::move(message->src);
	info_->orig_source = std::move(message->origsrc);
	info_->target = std::move(message->dest);

}


ActionMessage::ActionMessage(const std::string &bytes):ActionMessage()
{
	from_string(bytes);
}

ActionMessage::ActionMessage(const std::vector<char> &bytes): ActionMessage()
{
	from_vector(bytes);
}

ActionMessage::ActionMessage(const char *data, size_t size): ActionMessage()
{
	fromByteArray(data, size);
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
    info_ = std::move (act.info_);
    return *this;
}

void ActionMessage::moveInfo(std::unique_ptr<Message> message)
{
	
	action_ = CMD_SEND_MESSAGE;
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

void ActionMessage::setAction(action_message_def::action_t action)
{
	if (hasInfo(action))
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

using archiver = cereal::PortableBinaryOutputArchive;

using retriever = cereal::PortableBinaryInputArchive;

void ActionMessage::toByteArray(char *data, size_t buffer_size) const
{
	boost::iostreams::basic_array_sink<char> sr(data, buffer_size);
	boost::iostreams::stream< boost::iostreams::basic_array_sink<char> > s(sr);

	archiver oa(s);

	save(oa);
}

std::string ActionMessage::to_string() const
{
	std::string data;
	boost::iostreams::back_insert_device<std::string> inserter(data);
	boost::iostreams::stream<boost::iostreams::back_insert_device<std::string> > s(inserter);
	archiver oa(s);

	save(oa);

	// don't forget to flush the stream to finish writing into the buffer
	s.flush();
	return data;
}

std::vector<char> ActionMessage::to_vector() const
{
	std::vector<char> data;
	boost::iostreams::back_insert_device<std::vector<char>> inserter(data);
	boost::iostreams::stream<boost::iostreams::back_insert_device<std::vector<char>> > s(inserter);
	archiver oa(s);

	save(oa);

	// don't forget to flush the stream to finish writing into the buffer
	s.flush();
	return data;
}

void ActionMessage::to_vector(std::vector<char> &data) const
{
	data.clear();
	boost::iostreams::back_insert_device<std::vector<char>> inserter(data);
	boost::iostreams::stream<boost::iostreams::back_insert_device<std::vector<char>> > s(inserter);
	archiver oa(s);

	save(oa);

	// don't forget to flush the stream to finish writing into the buffer
	s.flush();
}

void ActionMessage::to_string(std::string &data) const
{
	data.clear();

	boost::iostreams::back_insert_device<std::string> inserter(data);
	boost::iostreams::stream<boost::iostreams::back_insert_device<std::string> > s(inserter);
	archiver oa(s);

	save(oa);

	// don't forget to flush the stream to finish writing into the buffer
	s.flush();

}

void ActionMessage::fromByteArray(const char *data, size_t buffer_size)
{
	boost::iostreams::basic_array_source<char> device(data, buffer_size);
	boost::iostreams::stream<boost::iostreams::basic_array_source<char> > s(device);
	retriever ia(s);
	load(ia);
}

void ActionMessage::from_string(const std::string &data)
{
	fromByteArray(data.data(), data.size());
}

void ActionMessage::from_vector(const std::vector<char> &data)
{
	fromByteArray(data.data(), data.size());
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


bool isPriorityCommand(const ActionMessage &command)
{
	return (command.action()<action_message_def::action_t::cmd_ignore);
}
}