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
#include "ActionMessageDefintions.h"
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
        std::string source;  //!< name of a registration
        std::string &type;  //!< alias source to type for registration
        std::string target;  //!< target or destination
        std::string &units;  //!< alias type to target for registration
        std::string orig_source;  //!< the original source
		std::string &type_out;  //!< alias type_out to orig_source for filter
        std::string orig_dest;  //!< the original destination of a message
		/** constructor*/
        AdditionalInfo () noexcept : type (source), units (target),type_out(orig_source){};
		/** copy constructor*/
        AdditionalInfo (const AdditionalInfo &ai)
            : source (ai.source), type (source), target (ai.target),
              units (target), orig_source (ai.orig_source), type_out(orig_source),orig_dest(ai.orig_dest) {};
		/** move constructor*/
        AdditionalInfo (AdditionalInfo &&ai) noexcept
            : source (std::move (ai.source)), type (source),
              target (std::move (ai.target)), units (target), orig_source (std::move (ai.orig_source)), type_out(orig_source),orig_dest(std::move(ai.orig_dest)) {};
        template <class Archive>
        void save (Archive &ar) const
        {
			ar(source, target, orig_source,orig_dest);
        }

        template <class Archive>
        void load (Archive &ar)
        {
           
			ar(source, target, orig_source,orig_dest);
        }
    };
   
    // need to try to make sure this object is under 64 bytes in size to fit in cache lines NOT there yet
  private:
    action_message_def::action_t action_ = CMD_IGNORE;  // 4 -- command
  public:
    int32_t source_id = 0;  //!< 8 -- for federate_id or route_id
    int32_t source_handle = 0;  //!< 12 -- for local handle or local code
    int32_t dest_id = 0;  //!< 16 fed_id for a targeted message
    int32_t dest_handle = 0;  //!< 20 local handle for a targeted message
	int32_t &index;			//!<alias to dest_handle 
    uint16_t counter=0; //!< 22 counter for filter tracking
    uint16_t flags=0; //!<  24 set of messageFlags
   
    Time actionTime = timeZero;  //!< 32 the time an action took place or will take place	//32
    Time Te = timeZero;  //!< 40 event time
    Time Tdemin = timeZero;  //!< 48 min dependent event time;
    std::string payload;  //!< string containing the data	//64 std::string is 32 bytes on most platforms (except libc++)
    std::string &name;  //!<alias payload to a name reference for registration functions
  private:
    std::unique_ptr<AdditionalInfo> info_;  //!< pointer to an additional info structure with more data if required
  public:
    /** default constructor*/
    ActionMessage () noexcept : index(dest_handle), name (payload) {};
    /** construct from an action type 
    @details this is an implicit constructor
    */
    ActionMessage (action_message_def::action_t startingAction);
    /** construct from action, source and destination id's
    */
    ActionMessage(action_message_def::action_t startingAction, int32_t sourceId, int32_t destId);
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
    action_message_def::action_t action () const noexcept { return action_; }
    /** set the action*/
    void setAction (action_message_def::action_t newAction);
    /** get a reference to the additional info structure*/
    AdditionalInfo &info ();
    /** get a const ref to the info structure*/
    const AdditionalInfo &info () const;
	/** move a message data into the actionMessage
	@details take ownership of the message and move the contents out then destroy the message shell
	@param message the message to move.  
	*/
    void moveInfo (std::unique_ptr<Message> message);

    template <class Archive>
    void save (Archive &ar) const
    {
        ar (action_, source_id, source_handle, dest_id, dest_handle);
        ar (counter,flags);

        auto btc = actionTime.getBaseTimeCode ();
        auto Tebase = Te.getBaseTimeCode();
        auto Tdeminbase = Tdemin.getBaseTimeCode();
        ar(btc,Tebase,Tdeminbase, payload);
        if (hasInfo(action_))
        {
            ar (info_);
        }
    }

    template <class Archive>
    void load (Archive &ar)
    {
        ar (action_, source_id, source_handle, dest_id, dest_handle);

        ar (counter,flags);

        decltype (actionTime.getBaseTimeCode ()) btc;
        decltype (Te.getBaseTimeCode()) Tebase;
        decltype (Tdemin.getBaseTimeCode()) Tdeminbase;

        ar (btc, Tebase, Tdeminbase, payload);

        actionTime.setBaseTimeCode (btc);
        Te.setBaseTimeCode(Tebase);
        Tdemin.setBaseTimeCode(Tdeminbase);
        if (hasInfo(action_))
        {
            if (!info_)
            {
                info_ = std::make_unique<AdditionalInfo> ();
            }
            ar (info_);
        }
    }

    // functions that convert to and from a byte stream

    /** convert a command to a raw data bytes
    @param[out] data pointer to memory to store the command
    @param[in] buffer_size-- the size of the buffer
    @return the size of the buffer actually used
    */
    int toByteArray (char *data, size_t buffer_size) const;
    /** convert to a string using a reference*/
    void to_string (std::string &data) const;
    /** convert to a byte string*/
	std::string to_string() const;
    /** packettize the message with a simple header and tail sequence
    */
    std::string packetize() const;
    /** covert to a byte vector using a reference*/
	void to_vector(std::vector<char> &data) const;
    /** convert a command to a byte vector*/
	std::vector<char> to_vector() const;
    /** generate a command from a raw data stream*/
    void fromByteArray (const char *data, size_t buffer_size);
    /** load a command from a packetized stream 
    @return the number of bytes used
    */
    size_t depacketize(const char *data, size_t buffer_size);
    /** read a command from a string*/
    void from_string (const std::string &data);
    /** read a command from a char vector*/
	void from_vector(const std::vector<char> &data);

};

#define SET_ACTION_FLAG(M,flag) do{M.flags|=(uint16_t(1)<<(flag));}while(false)

#define CHECK_ACTION_FLAG(M,flag) ((M.flags&(uint16_t(1)<<(flag)))!=0)

#define CLEAR_ACTION_FLAG(M,flag) do{M.flags&=~(uint16_t(1)<<(flag));}while(false)



/** create a new message object that copies all the information from the ActionMessage into newly allocated memory for the
 * message
 */
std::unique_ptr<Message> createMessage (const ActionMessage &cmd);

/** create a new message object that moves all the information from the ActionMessage into newly allocated memory for the
 * message
 */
std::unique_ptr<Message> createMessage (ActionMessage &&cmd);

/** check if a command is a protocol command*/
inline bool isProtocolCommand(const ActionMessage &command) noexcept
{
    return ((command.action() == CMD_PROTOCOL) || (command.action() == CMD_PROTOCOL_PRIORITY) || (command.action() == CMD_PROTOCOL_BIG));
}
/** check if a command is a priority command*/
inline bool isPriorityCommand(const ActionMessage &command) noexcept
{
    return (command.action() < action_message_def::action_t::cmd_ignore);
}

/** check if a command is a disconnect command*/
inline bool isDisconnectCommand(const ActionMessage &command) noexcept
{
    return ((command.action() == CMD_DISCONNECT) || (command.action() == CMD_PRIORITY_DISCONNECT) || (command.action() == CMD_TERMINATE_IMMEDIATELY));
}

/** check if a command is a priority command*/
inline bool isValidCommand(const ActionMessage &command) noexcept
{
    return (command.action() != action_message_def::action_t::cmd_invalid);
}
/** generate a human readable string with information about a command
@param command the command to generate the string for
@return a string representing information about the command
*/
std::string prettyPrintString(const ActionMessage &command);

/** stream operator for a command
*/
std::ostream& operator<<(std::ostream& os, const ActionMessage & command);

}  // namespace helics
#endif
