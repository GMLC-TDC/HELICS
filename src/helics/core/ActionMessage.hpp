/*

Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "ActionMessageDefintions.hpp"
#include "Core.hpp"
#include "core-types.hpp"
#include <cereal/types/memory.hpp>
#include <memory>
#include <string>

namespace helics
{
constexpr int32_t cmd_info_basis = 65536;
/** class defining the primary message object used in Helics */
class ActionMessage
{
  public:
    /** class for containing possibly larger additional information for certain message types */
    class AdditionalInfo
    {
      public:
        std::string source;  //!< name of a registration
        std::string &type;  //!< alias source to type for registration
        std::string target;  //!< target or destination
        std::string &units;  //!< alias type to target for registration
        std::string orig_source;  //!< the original source
        std::string &type_out;  //!< alias type_out to orig_source for filter
        std::string original_dest;  //!< the original destination of a message
        /** constructor*/
        AdditionalInfo () noexcept : type (source), units (target), type_out (orig_source){};
        /** copy constructor*/
        AdditionalInfo (const AdditionalInfo &ai)
            : source (ai.source), type (source), target (ai.target), units (target), orig_source (ai.orig_source),
              type_out (orig_source), original_dest (ai.original_dest){};
        /** move constructor*/
        AdditionalInfo (AdditionalInfo &&ai) noexcept
            : source (std::move (ai.source)), type (source), target (std::move (ai.target)), units (target),
              orig_source (std::move (ai.orig_source)), type_out (orig_source),
              original_dest (std::move (ai.original_dest)){};
        ~AdditionalInfo () = default;
        template <class Archive>
        void serialize (Archive &ar)
        {
            ar (source, target, orig_source, original_dest);
        }
    };

    // need to try to make sure this object is under 64 bytes in size to fit in cache lines NOT there yet
  private:
    action_message_def::action_t messageAction = CMD_IGNORE;  // 4 -- command
  public:
    int32_t messageID = 0; //!< 8 -- message ID for a variety of purposes
    int32_t source_id = 0;  //!< 12 -- for federate_id or route_id
    int32_t source_handle = 0;  //!< 16 -- for local handle or local code
    int32_t dest_id = 0;  //!< 20 fed_id for a targeted message
    int32_t dest_handle = 0;  //!< 24 local handle for a targeted message
    uint16_t counter = 0;  //!< 26 counter for filter tracking or message counter
    uint16_t flags = 0;  //!<  28 set of messageFlags
    //4 byte gap
    Time actionTime = timeZero;  //!< 40 the time an action took place or will take place	//32
    Time Te = timeZero;  //!< 48 event time
    Time Tdemin = timeZero;  //!< 56 min dependent event time
    Time Tso = timeZero;  //!<64 the second order dependent time
    
private:
    std::unique_ptr<AdditionalInfo>
        extraInfo;  //!< pointer to an additional info structure with more data if required //72
public:
    std::string
      payload;  //!< string containing the data	//64 std::string is 32 bytes on most platforms (except libc++)
    std::string &name;  //!< alias payload to a name reference for registration functions
  
  public:
    /** default constructor*/
    ActionMessage () noexcept : name (payload){};
    /** construct from an action type
    @details this is intended to be an implicit constructor
    @param startingAction from an action message definition
    */
    // cppcheck-suppress noExplicitConstructor
    /* implicit */ ActionMessage (action_message_def::action_t startingAction);
    /** construct from action, source and destination id's
     */
    ActionMessage (action_message_def::action_t startingAction, int32_t sourceId, int32_t destId);
    /** move constructor*/
    ActionMessage (ActionMessage &&act) noexcept;
    /** build an action message from a message*/
    explicit ActionMessage (std::unique_ptr<Message> message);
    /** construct from a string*/
    explicit ActionMessage (const std::string &bytes);
    /** construct from a data vector*/
    explicit ActionMessage (const std::vector<char> &bytes);
    /** construct from a data pointer and size*/
    explicit ActionMessage (const char *data, size_t size);
    /** destructor*/
    ~ActionMessage ();
    /** copy constructor*/
    ActionMessage (const ActionMessage &act);
    /** copy operator*/
    ActionMessage &operator= (const ActionMessage &);
    /** move assignment*/
    ActionMessage &operator= (ActionMessage &&act) noexcept;
    /** get the action of the message*/
    action_message_def::action_t action () const noexcept { return messageAction; }
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
    /** save the data to an archive*/
    template <class Archive>
    void save (Archive &ar) const
    {
        ar (messageAction,messageID, source_id, source_handle, dest_id, dest_handle);
        ar (counter, flags);

        auto btc = actionTime.getBaseTimeCode ();
        auto Tebase = Te.getBaseTimeCode ();
        auto Tdeminbase = Tdemin.getBaseTimeCode ();
        auto Tsobase = Tso.getBaseTimeCode();
        ar (btc, Tebase,Tsobase, Tdeminbase, payload);
        if (hasInfo (messageAction))
        {
            ar (extraInfo);
        }
    }
    /** load the data from an archive*/
    template <class Archive>
    void load (Archive &ar)
    {
        ar (messageAction,messageID, source_id, source_handle, dest_id, dest_handle);

        ar (counter, flags);

        decltype (actionTime.getBaseTimeCode ()) btc;
        decltype (Te.getBaseTimeCode ()) Tebase;
        decltype (Tdemin.getBaseTimeCode ()) Tdeminbase;
        decltype (Tso.getBaseTimeCode()) Tsobase;
        ar (btc, Tebase, Tdeminbase, Tsobase, payload);

        actionTime.setBaseTimeCode (btc);
        Te.setBaseTimeCode (Tebase);
        Tdemin.setBaseTimeCode (Tdeminbase);
        Tso.setBaseTimeCode(Tsobase);
        if (hasInfo (messageAction))
        {
            if (!extraInfo)
            {
                extraInfo = std::make_unique<AdditionalInfo> ();
            }
            ar (extraInfo);
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
    std::string to_string () const;
    /** packetize the message with a simple header and tail sequence
     */
    std::string packetize () const;
    /** covert to a byte vector using a reference*/
    void to_vector (std::vector<char> &data) const;
    /** convert a command to a byte vector*/
    std::vector<char> to_vector () const;
    /** generate a command from a raw data stream*/
    void fromByteArray (const char *data, size_t buffer_size);
    /** load a command from a packetized stream /ref packetize
    @return the number of bytes used
    */
    size_t depacketize (const char *data, size_t buffer_size);
    /** read a command from a string*/
    void from_string (const std::string &data);
    /** read a command from a char vector*/
    void from_vector (const std::vector<char> &data);
};

/** template function to set a flag in an object containing a flags field*/
template <class FlagContainer, class FlagIndex>
inline void setActionFlag (FlagContainer &M, FlagIndex flag)
{
    M.flags |= (static_cast<decltype (M.flags)> (1) << (flag));
}

/** template function to check a flag in an object containing a flags field*/
template <class FlagContainer, class FlagIndex>
inline bool checkActionFlag (const FlagContainer &M, FlagIndex flag)
{
    return ((M.flags & (static_cast<decltype (M.flags)> (1) << (flag))) != 0);
}

/** template function to clear a flag in an object containing a flags field*/
template <class FlagContainer, class FlagIndex>
inline void clearActionFlag (FlagContainer &M, FlagIndex flag)
{
    M.flags &= ~(static_cast<decltype (M.flags)> (1) << (flag));
}

/** create a new message object that copies all the information from the ActionMessage into newly allocated memory
 * for the message
 */
std::unique_ptr<Message> createMessageFromCommand (const ActionMessage &cmd);

/** create a new message object that moves all the information from the ActionMessage into newly allocated memory
 * for the message
 */
std::unique_ptr<Message> createMessageFromCommand (ActionMessage &&cmd);

/** check if a command is a protocol command*/
inline bool isProtocolCommand (const ActionMessage &command) noexcept
{
    return ((command.action () == CMD_PROTOCOL) || (command.action () == CMD_PROTOCOL_PRIORITY) ||
            (command.action () == CMD_PROTOCOL_BIG));
}
/** check if a command is a priority command*/
inline bool isPriorityCommand (const ActionMessage &command) noexcept
{
    return (command.action () < action_message_def::action_t::cmd_ignore);
}

inline bool isTimingCommand (const ActionMessage &command) noexcept
{
    switch (command.action ())
    {
    case CMD_DISCONNECT:
    case CMD_TIME_GRANT:
    case CMD_TIME_REQUEST:
    case CMD_EXEC_GRANT:
    case CMD_EXEC_REQUEST:
    case CMD_PRIORITY_DISCONNECT:
        return true;
    default:
        return false;
    }
}

inline bool isDependencyCommand (const ActionMessage &command) noexcept
{
    switch (command.action ())
    {
    case CMD_ADD_DEPENDENCY:
    case CMD_REMOVE_DEPENDENCY:
    case CMD_ADD_DEPENDENT:
    case CMD_REMOVE_DEPENDENT:
    case CMD_ADD_INTERDEPENDENCY:
    case CMD_REMOVE_INTERDEPENDENCY:
        return true;
    default:
        return false;
    }
}

/** check if a command is a disconnect command*/
inline bool isDisconnectCommand (const ActionMessage &command) noexcept
{
    return ((command.action () == CMD_DISCONNECT) || (command.action () == CMD_PRIORITY_DISCONNECT) ||
            (command.action () == CMD_TERMINATE_IMMEDIATELY));
}

/** check if a command is a valid command*/
inline bool isValidCommand (const ActionMessage &command) noexcept
{
    return (command.action () != action_message_def::action_t::cmd_invalid);
}
/** generate a human readable string with information about a command
@param command the command to generate the string for
@return a string representing information about the command
*/
std::string prettyPrintString (const ActionMessage &command);

/** stream operator for a command
 */
std::ostream &operator<< (std::ostream &os, const ActionMessage &command);

}  // namespace helics

