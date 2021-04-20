/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "ActionMessageDefintions.hpp"
#include "basic_core_types.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics {
constexpr int targetStringLoc{0};
constexpr int sourceStringLoc{1};
constexpr int unitStringLoc{1};
constexpr int origSourceStringLoc{2};
constexpr int origDestStringLoc{3};
constexpr int typeStringLoc{0};
constexpr int typeOutStringLoc{1};

constexpr int32_t cmd_info_basis{65536};

/** class defining the primary message object used in HELICS */
class ActionMessage {
    // need to try to make sure this object is under 64 bytes in size to fit in cache lines NOT
    // there yet
  private:
    action_message_def::action_t messageAction{CMD_IGNORE};  // 4 -- command
  public:
    int32_t messageID{0};  //!< 8 -- message ID for a variety of purposes
    global_federate_id source_id{parent_broker_id};  //!< 12 -- for federate_id or route_id
    interface_handle source_handle{};  //!< 16 -- for local handle or local code
    global_federate_id dest_id{parent_broker_id};  //!< 20 fed_id for a targeted message
    interface_handle dest_handle{};  //!< 24 local handle for a targeted message
    uint16_t counter{0};  //!< 26 counter for filter tracking or message counter
    uint16_t flags{0};  //!<  28 set of messageFlags
    uint32_t sequenceID{0};  //!< a sequence number for ordering
    Time actionTime{timeZero};  //!< 40 the time an action took place or will take place    //32
    std::string payload;  //!< string containing the data    //96 std::string is 32 bytes on most
                          //!< platforms (except libc++)
    std::string& name;  //!< alias payload to a name reference for registration functions
    Time Te{timeZero};  //!< 48 event time
    Time Tdemin{timeZero};  //!< 56 min dependent event time
    Time Tso{timeZero};  //!< 64 the second order dependent time
  private:
    std::vector<std::string> stringData;  //!< container for extra string data
  public:
    /** default constructor*/
    ActionMessage() noexcept: name(payload) {}
    /** construct from an action type
    @details this is intended to be an implicit constructor
    @param startingAction from an action message definition
    */
    /* implicit */ ActionMessage(action_message_def::action_t startingAction);  // NOLINT
    /** construct from action, source and destination id's
     */
    ActionMessage(action_message_def::action_t startingAction,
                  global_federate_id sourceId,
                  global_federate_id destId);
    /** move constructor*/
    ActionMessage(ActionMessage&& act) noexcept;
    /** build an action message from a message*/
    explicit ActionMessage(std::unique_ptr<Message> message);
    /** construct from a string*/
    explicit ActionMessage(const std::string& bytes);
    /** construct from a data vector*/
    explicit ActionMessage(const std::vector<char>& bytes);
    /** construct from a data pointer and size*/
    explicit ActionMessage(const char* data, size_t size);
    /** destructor*/
    ~ActionMessage();
    /** copy constructor*/
    ActionMessage(const ActionMessage& act);
    /** copy operator*/
    ActionMessage& operator=(const ActionMessage& act);
    /** move assignment*/
    ActionMessage& operator=(ActionMessage&& act) noexcept;
    /** move assignment from message data into the actionMessage
    @details take ownership of the message and move the contents out then destroy the message shell
    @param message the message to move.
    */
    ActionMessage& operator=(std::unique_ptr<Message> message) noexcept;
    /** get the action of the message*/
    action_message_def::action_t action() const noexcept { return messageAction; }
    /** set the action*/
    void setAction(action_message_def::action_t newAction);

    /** set the source from a global handle*/
    void setSource(global_handle hand)
    {
        source_id = hand.fed_id;
        source_handle = hand.handle;
    }
    /** set the destination from a global handle*/
    void setDestination(global_handle hand)
    {
        dest_id = hand.fed_id;
        dest_handle = hand.handle;
    }
    /** get the reference to the string data vector*/
    const std::vector<std::string>& getStringData() const { return stringData; }

    void clearStringData() { stringData.clear(); }
    // most use cases for this involve short strings, or already have references that need to be
    // copied so supporting move isn't  going to be that useful here, the long strings are going in
    // the payload
    void setStringData(const std::string& string1)
    {
        stringData.resize(1);
        stringData[0] = string1;
    }
    void setStringData(const std::string& string1, const std::string& string2)
    {
        stringData.resize(2);
        stringData[0] = string1;
        stringData[1] = string2;
    }
    void setStringData(const std::string& string1,
                       const std::string& string2,
                       const std::string& string3)
    {
        stringData.resize(3);
        stringData[0] = string1;
        stringData[1] = string2;
        stringData[2] = string3;
    }
    void setStringData(const std::string& string1,
                       const std::string& string2,
                       const std::string& string3,
                       const std::string& string4)
    {
        stringData.resize(4);
        stringData[0] = string1;
        stringData[1] = string2;
        stringData[2] = string3;
        stringData[3] = string4;
    }
    const std::string& getString(int index) const;

    void setString(int index, const std::string& str);
    /** get the source global_handle*/
    global_handle getSource() const { return global_handle{source_id, source_handle}; }
    /** get the global destination handle*/
    global_handle getDest() const { return global_handle{dest_id, dest_handle}; }
    /** swap the source and destination*/
    void swapSourceDest() noexcept
    {
        std::swap(source_id, dest_id);
        std::swap(source_handle, dest_handle);
    }
    /** set some extra piece of data if the full destination is not used*/
    void setExtraData(int32_t data) { dest_handle = interface_handle{data}; }
    /** get the extra piece of integer data*/
    int32_t getExtraData() const { return dest_handle.baseValue(); }
    /** set some extra piece of data if the full source is not used*/
    void setExtraDestData(int32_t data) { source_handle = interface_handle{data}; }
    /** get the extra piece of integer data used in the destination*/
    int32_t getExtraDestData() const { return source_handle.baseValue(); }
    // functions that convert to and from a byte stream

    /** generate a size of the message in bytes if it were to be serialized*/
    int serializedByteCount() const;
    /** convert a command to a raw data bytes
    @param[out] data pointer to memory to store the command
    @param buffer_size  the size of the buffer
    @return the size of the buffer actually used
    */
    int toByteArray(char* data, int buffer_size) const;
    /** convert to a string using a reference*/
    void to_string(std::string& data) const;
    /** convert to a byte string*/
    std::string to_string() const;
    /** packetize the message with a simple header and tail sequence
     */
    std::string packetize() const;
    void packetize(std::string& data) const;
    /** covert to a byte vector using a reference*/
    void to_vector(std::vector<char>& data) const;
    /** convert a command to a byte vector*/
    std::vector<char> to_vector() const;
    /** generate a command from a raw data stream*/
    int fromByteArray(const char* data, int buffer_size);
    /** load a command from a packetized stream /ref packetize
    @return the number of bytes used
    */
    int depacketize(const char* data, int buffer_size);
    /** read a command from a string*/
    void from_string(const std::string& data);
    /** read a command from a char vector*/
    void from_vector(const std::vector<char>& data);

    friend std::unique_ptr<Message> createMessageFromCommand(const ActionMessage& cmd);
    friend std::unique_ptr<Message> createMessageFromCommand(ActionMessage&& cmd);
};

inline bool operator<(const ActionMessage& cmd, const ActionMessage& cmd2)
{
    return (cmd.actionTime < cmd2.actionTime);
}

/** create a new message object that copies all the information from the ActionMessage into newly
 * allocated memory for the message
 */
std::unique_ptr<Message> createMessageFromCommand(const ActionMessage& cmd);

/** create a new message object that moves all the information from the ActionMessage into newly
 * allocated memory for the message
 */
std::unique_ptr<Message> createMessageFromCommand(ActionMessage&& cmd);

/** check if a command is a protocol command*/
inline bool isProtocolCommand(const ActionMessage& command) noexcept
{
    return ((command.action() == CMD_PROTOCOL) || (command.action() == CMD_PROTOCOL_PRIORITY) ||
            (command.action() == CMD_PROTOCOL_BIG));
}
/** check if a command is a priority command*/
inline bool isPriorityCommand(const ActionMessage& command) noexcept
{
    return (command.action() < action_message_def::action_t::cmd_ignore);
}

inline bool isTimingCommand(const ActionMessage& command) noexcept
{
    switch (command.action()) {
        case CMD_DISCONNECT:
        case CMD_BROADCAST_DISCONNECT:
        case CMD_DISCONNECT_CORE:
        case CMD_DISCONNECT_BROKER:
        case CMD_DISCONNECT_FED:
        case CMD_TIME_GRANT:
        case CMD_TIME_REQUEST:
        case CMD_EXEC_GRANT:
        case CMD_EXEC_REQUEST:
        case CMD_PRIORITY_DISCONNECT:
        case CMD_TERMINATE_IMMEDIATELY:
        case CMD_ERROR:
        case CMD_LOCAL_ERROR:
        case CMD_GLOBAL_ERROR:
            return true;
        default:
            return false;
    }
}

inline bool isDependencyCommand(const ActionMessage& command) noexcept
{
    switch (command.action()) {
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
inline bool isDisconnectCommand(const ActionMessage& command) noexcept
{
    switch (command.action()) {
        case CMD_DISCONNECT:
        case CMD_DISCONNECT_CHECK:
        case CMD_DISCONNECT_NAME:
        case CMD_USER_DISCONNECT:
        case CMD_DISCONNECT_FED:
        case CMD_DISCONNECT_CORE:
        case CMD_PRIORITY_DISCONNECT:
        case CMD_TERMINATE_IMMEDIATELY:
        case CMD_REMOVE_FILTER:
        case CMD_REMOVE_ENDPOINT:
        case CMD_DISCONNECT_FED_ACK:
        case CMD_DISCONNECT_CORE_ACK:
        case CMD_DISCONNECT_BROKER_ACK:
        case CMD_DISCONNECT_BROKER:
        case CMD_BROADCAST_DISCONNECT:
        case CMD_STOP:
            return true;
        case CMD_TIME_GRANT:
            return (command.actionTime == Time::maxVal());
        default:
            return false;
    }
}

/** check if a command is a disconnect command*/
inline bool isErrorCommand(const ActionMessage& command) noexcept
{
    switch (command.action()) {
        case CMD_ERROR:
        case CMD_LOCAL_ERROR:
        case CMD_GLOBAL_ERROR:
            return true;
        default:
            return false;
    }
}

/** check if a command is a valid command*/
inline bool isValidCommand(const ActionMessage& command) noexcept
{
    return (command.action() != action_message_def::action_t::cmd_invalid);
}
/** generate a human readable string with information about a command
@param command the command to generate the string for
@return a string representing information about the command
*/
std::string prettyPrintString(const ActionMessage& command);

/** stream operator for a command
 */
std::ostream& operator<<(std::ostream& os, const ActionMessage& command);

/** append a message to multi message container
@param m the message to add the extra message to
@param newMessage the message to append
@return the integer location of the message in the stringData section*/
int appendMessage(ActionMessage& m, const ActionMessage& newMessage);

/** generate a string representing an error from an ActionMessage
@param command the command to generate the error string for
@return a string describing the error, if the string is not an error the string is empty
*/
std::string errorMessageString(const ActionMessage& command);

/** set the flags for an iteration request*/
void setIterationFlags(ActionMessage& command, iteration_request iterate);

}  // namespace helics
