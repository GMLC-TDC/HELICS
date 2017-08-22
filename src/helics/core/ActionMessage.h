/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef ACTION_MESSAGE_H_
#define ACTION_MESSAGE_H_
#pragma once

#include "core-types.h"
#include "core.h"
#include <cereal/types/memory.hpp>
#include <memory>
#include <string>

namespace helics
{
const int32_t cmd_info_basis = 65536;

class ActionMessage
{
  public:
    // class for containing possibly larger additional information for certain message types
    class AdditionalInfo
    {
      public:
        Time Te = timeZero;  //!< event time
        Time Tdemin = timeZero;  //!< min dependent event time;
        std::string source;  //!< name of a registration
        std::string &type;  //!< alias source to type for registration
        std::string target;  //!< target or destination
        std::string &units;  //!< alias type to target for registration
        std::string orig_source;  //!< the original source
		/** constructor*/
        AdditionalInfo () noexcept : type (source), units (target){};
		/** copy constructor*/
        AdditionalInfo (const AdditionalInfo &ai)
            : Te (ai.Te), Tdemin (ai.Tdemin), source (ai.source), type (source), target (ai.target),
              units (target), orig_source (ai.orig_source){};
		/** move constructor*/
        AdditionalInfo (AdditionalInfo &&ai) noexcept
            : Te (ai.Te), Tdemin (ai.Tdemin), source (std::move (ai.source)), type (source),
              target (std::move (ai.target)), units (target), orig_source (std::move (ai.orig_source)){};
        template <class Archive>
        void save (Archive &ar) const
        {
            auto Tebase = Te.getBaseTimeCode ();
            auto Tdeminbase = Tdemin.getBaseTimeCode ();
			ar(Tebase, Tdeminbase);
			ar(source, target, orig_source);
        }

        template <class Archive>
        void load (Archive &ar)
        {
            decltype (Te.getBaseTimeCode ()) Tebase;
			decltype (Tdemin.getBaseTimeCode()) Tdeminbase;
			ar(Tebase, Tdeminbase);
            Te.setBaseTimeCode (Tebase);
            Tdemin.setBaseTimeCode (Tdeminbase);
			ar(source, target, orig_source);
        }
    };
    /** enumeration of globally recognized commands
    @details they are explicitly numbered for debugging and to ensure the enumeration is constant
    across different compilers*/
	enum action_t : int32_t
	{
		cmd_ignore = 0,

		cmd_disconnect = 3,  //!< command to disconnect a broker from a higher level broker
		cmd_init = 5,  //!< request entry to init mode
		cmd_init_grant = 7,  //!< grant entry to initialization mode
		cmd_init_not_ready = 8,  //!< retract an init ready command
		cmd_exec_request = 9,  //!< request an iteration or entry to execution mode
		cmd_exec_grant = 10,  //!< grant entry to exec mode or iterate
		cmd_exec_check = 12,  //!< command to run a check on execution entry
		cmd_register_route = 15,  //!< instructions to create a direct route to another federate
		cmd_stop = 20,  //!< halt execution
		cmd_fed_ack = 25,  //!<a reply with the global id or an error if the fed registration failed
		cmd_broker_ack = 27,  // a reply to the connect command with a global route id
		cmd_add_route = 32,  //!< command to define a route
		cmd_time_grant = 35,  //!< grant a time or iteration
		cmd_time_check = 36,  //!< command to run a check on whether time can be granted
		cmd_pub = 45,  //!< publish a value
		cmd_bye = 2000,  //!< message stating this is the last communication from a federate
		cmd_log = 55,  //!< log a message with the root broker
		cmd_warning = 9990, //!< indicate some sort of warning 
		cmd_error = 10000,  //!< indicate an error with a federate

		cmd_send_route = 75,  //!< command to define a route information
		cmd_subscriber = 85,  // !< command to send a subscriber
		cmd_send_dependency = 95,  //!< command to send a federate dependency information

		cmd_reg_fed = 105,  //!< register a federate
		// commands that require the extra info allocation have numbers greater than cmd_info_basis
		cmd_time_request = cmd_info_basis + 10,  //!< request a time or iteration
		cmd_send_message = cmd_info_basis + 20,  //!< send a message
		cmd_send_for_filter = cmd_info_basis + 30,  //!< send a message to be filtered

		cmd_reg_broker = cmd_info_basis + 40,  //!< for a broker to connect with a higher level broker
		cmd_reg_pub = cmd_info_basis + 50,  //!< register a publication
		cmd_notify_pub = 50,	//!< notify of a publication
		cmd_reg_dst = cmd_info_basis + 60,  //!< register a destination filter
		cmd_notify_dst = 60, //!< notify of a destination filter
		cmd_reg_sub = cmd_info_basis + 70,  //!< register a subscription
		cmd_notify_sub = 70,	//!< notify of a subscription
		cmd_reg_src = cmd_info_basis + 80,  //!< register a source filter
		cmd_notify_src = 80,	//!< notify of a source
		cmd_reg_end = cmd_info_basis + 90,  //!< register an endpoint
		cmd_notify_end = 90,	//!< notify of an endpoint

		cmd_protocol = 60000, //!< cmd used in the protocol stacks and ignored by the core
		cmd_protocol_big=0x0F00+60000 //!< cmd used in the protocol stacks with the additional info
    };
    // need to make sure this object is under 64 bytes in size to fit in cache lines
  private:
    int32_t action_ = action_t::cmd_ignore;  // 4 -- command
  public:
    int32_t source_id = 0;  // 8 -- for federate_id or route_id
    int32_t source_handle = 0;  // 12 -- for local handle or local code
    int32_t dest_id = 0;  // 16 fed_id for a targeted message
    int32_t dest_handle = 0;  // 20 local handle for a targetted message
	int32_t &index;			//alias to dest_handle 
    bool iterationComplete = false;  // 24
	bool &processingComplete;  //Alias to iterationComplete
    bool required = false;  //!< flag indicating a publication is required
    bool error = false;  //!< flag indicating an error condition associated with the command
    bool flag = false;  //!< general flag for many purposes
    Time actionTime = timeZero;  //!< the time an action took place or will take place	//32
    std::string payload;  //!< string containing the data	//64 std::string is 32 bytes on most platforms (except libc++)
    std::string &name;  //!<alias payload to a name reference for registration functions
  private:
    std::unique_ptr<AdditionalInfo> info_;  //!< pointer to an additional info structure with more data if required
  public:
    /** default constructor*/
    ActionMessage () noexcept : index(dest_handle), processingComplete(iterationComplete), name (payload) {};
    /** construct from an action type*/
    ActionMessage (action_t action);
    /** move constructor*/
    ActionMessage (ActionMessage &&act) noexcept;
    /** build an action message from a message*/
    ActionMessage (std::unique_ptr<Message> message);
	/** construct from a string*/
	explicit ActionMessage(const std::string &bytes);
	/** construct from a data vector*/
	explicit ActionMessage(const std::vector<char> &bytes);
	/** construct from a data pointer and size*/
	explicit ActionMessage(const char *data, size_t size);
    /** destructor*/
    ~ActionMessage ();
    /** copy constructor*/
    ActionMessage (const ActionMessage &act);
    /** copy operator*/
    ActionMessage &operator= (const ActionMessage &);
    /** move assignment*/
    ActionMessage &operator= (ActionMessage &&act) noexcept;
    /** get the action of the message*/
    int32_t action () const noexcept { return action_; }
    /** set the action*/
    void setAction (action_t action);
    /** get a reference to the additional info structure*/
    AdditionalInfo &info ();
    /** get a const ref to the info structure*/
    const AdditionalInfo &info () const;

    void moveInfo (std::unique_ptr<Message> message);

    template <class Archive>
    void save (Archive &ar) const
    {
        ar (action_, source_id, source_handle, dest_id, dest_handle);
        ar (iterationComplete, required, error, flag);

        auto btc = actionTime.getBaseTimeCode ();
        ar (btc, payload);
        if (action_ >= cmd_info_basis)
        {
            ar (info_);
        }
    }

    template <class Archive>
    void load (Archive &ar)
    {
        ar (action_, source_id, source_handle, dest_id, dest_handle);

        ar (iterationComplete, required, error, flag);

        decltype (actionTime.getBaseTimeCode ()) btc;
        ar (btc, payload);
        actionTime.setBaseTimeCode (btc);
        if (action_ >= cmd_info_basis)
        {
            if (!info_)
            {
                info_ = std::make_unique<AdditionalInfo> ();
            }
            ar (info_);
        }
    }

    /** functions that convert to and from a byte stream*/
    void toByteArray (char *data, size_t buffer_size) const;
    void to_string (std::string &data) const;
	std::string to_string() const;
	std::vector<char> to_vector() const;
    void fromByteArray (const char *data, size_t buffer_size);
    void from_string (const std::string &data);
	void from_vector(const std::vector<char> &data);

};


#define CMD_IGNORE ActionMessage::action_t::cmd_ignore
#define CMD_REG_BROKER ActionMessage::action_t::cmd_reg_broker
#define CMD_DISCONNECT ActionMessage::action_t::cmd_disconnect
#define CMD_INIT ActionMessage::action_t::cmd_init
#define CMD_INIT_NOT_READY ActionMessage::action_t::cmd_init_not_ready
#define CMD_INIT_GRANT ActionMessage::action_t::cmd_init_grant
#define CMD_EXEC_REQUEST ActionMessage::action_t::cmd_exec_request
#define CMD_EXEC_GRANT ActionMessage::action_t::cmd_exec_grant
#define CMD_EXEC_CHECK ActionMessage::action_t::cmd_exec_check
#define CMD_REG_ROUTE ActionMessage::action_t::cmd_register_route
#define CMD_STOP ActionMessage::action_t::cmd_stop
#define CMD_TIME_REQUEST ActionMessage::action_t::cmd_time_request
#define CMD_TIME_GRANT ActionMessage::action_t::cmd_time_grant
#define CMD_TIME_CHECK ActionMessage::action_t::cmd_time_check
#define CMD_SEND_MESSAGE ActionMessage::action_t::cmd_send_message
#define CMD_SEND_FOR_FILTER ActionMessage::action_t::cmd_send_for_filter
#define CMD_PUB ActionMessage::action_t::cmd_pub
#define CMD_LOG ActionMessage::action_t::cmd_log
#define CMD_WARNING ActionMessage::action_t::cmd_warning
#define CMD_ERROR ActionMessage::action_t::cmd_error
#define CMD_REG_PUB ActionMessage::action_t::cmd_reg_pub
#define CMD_NOTIFY_PUB ActionMessage::action_t::cmd_notify_pub
#define CMD_REG_SUB ActionMessage::action_t::cmd_reg_sub
#define CMD_NOTIFY_SUB ActionMessage::action_t::cmd_notify_sub
#define CMD_REG_END ActionMessage::action_t::cmd_reg_end
#define CMD_NOTIFY_END ActionMessage::action_t::cmd_notify_end
#define CMD_REG_DST_FILTER ActionMessage::action_t::cmd_reg_dst
#define CMD_NOTIFY_DST_FILTER ActionMessage::action_t::cmd_notify_dst
#define CMD_REG_SRC_FILTER ActionMessage::action_t::cmd_reg_src
#define CMD_NOTIFY_SRC_FILTER ActionMessage::action_t::cmd_notify_src
#define CMD_REG_FED ActionMessage::action_t::cmd_reg_fed
#define CMD_BROKER_ACK ActionMessage::action_t::cmd_broker_ack
#define CMD_FED_ACK ActionMessage::action_t::cmd_fed_ack
#define CMD_PROTOCOL ActionMessage::action_t::cmd_protocol
#define CMD_PROTOCOL_BIG ActionMessage::action_t::cmd_protocol_big

/** create a new message object that copies all the information from the cmd into newly allocated memory for the
 * message
 */
std::unique_ptr<Message> createMessage (const ActionMessage &cmd);

/** create a new message object that moves all the information from the cmd into newly allocated memory for the
 * message
 */
std::unique_ptr<Message> createMessage (ActionMessage &&cmd);

bool isPriorityCommand (const ActionMessage &command);


}  // namespace helics
#endif
