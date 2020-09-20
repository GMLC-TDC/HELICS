/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

// For warnings about constexpr paths in visual studio from frozen libraries
#if defined(_MSC_VER)
#    pragma warning(disable : 4127 4245)
#endif

#include "ActionMessage.hpp"

#include "../common/fmt_format.h"
#include "flagOperations.hpp"

#include <algorithm>
#include <complex>
#include <cstring>
#include <frozen/string.h>
#include <frozen/unordered_map.h>
#include <ostream>
#include <utility>
#include <vector>

namespace helics {
ActionMessage::ActionMessage(action_message_def::action_t startingAction):
    messageAction(startingAction)
{
}

ActionMessage::ActionMessage(action_message_def::action_t startingAction,
                             GlobalFederateId sourceId,
                             GlobalFederateId destId):
    messageAction(startingAction),
    source_id(sourceId), dest_id(destId)
{
}

ActionMessage::ActionMessage(ActionMessage&& act) noexcept:
    messageAction(act.messageAction), messageID(act.messageID), source_id(act.source_id),
    source_handle(act.source_handle), dest_id(act.dest_id), dest_handle(act.dest_handle),
    counter(act.counter), flags(act.flags), actionTime(act.actionTime), Te(act.Te),
    Tdemin(act.Tdemin), Tso(act.Tso), payload(std::move(act.payload)),
    stringData(std::move(act.stringData))
{
}

ActionMessage::ActionMessage(const ActionMessage& act):
    messageAction(act.messageAction), messageID(act.messageID), source_id(act.source_id),
    source_handle(act.source_handle), dest_id(act.dest_id), dest_handle(act.dest_handle),
    counter(act.counter), flags(act.flags), actionTime(act.actionTime), Te(act.Te),
    Tdemin(act.Tdemin), Tso(act.Tso), payload(act.payload), stringData(act.stringData)

{
}

ActionMessage::ActionMessage(std::unique_ptr<Message> message):
    messageAction(CMD_SEND_MESSAGE), messageID(message->messageID), actionTime(message->time),
    payload(std::move(message->data)), stringData({std::move(message->dest),
                                                   std::move(message->source),
                                                   std::move(message->original_source),
                                                   std::move(message->original_dest)})
{
}

ActionMessage::ActionMessage(const std::string& bytes): ActionMessage()
{
    from_string(bytes);
}

ActionMessage::ActionMessage(const std::vector<char>& bytes): ActionMessage()
{
    from_vector(bytes);
}

ActionMessage::ActionMessage(const void* data, size_t size): ActionMessage()
{
    fromByteArray(static_cast<const std::byte*>(data), size);
}

ActionMessage::~ActionMessage() = default;

ActionMessage& ActionMessage::operator=(const ActionMessage& act)  // NOLINT
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
    Tso = act.Tso;
    payload = act.payload;
    stringData = act.stringData;
    return *this;
}

ActionMessage& ActionMessage::operator=(ActionMessage&& act) noexcept
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
    Tso = act.Tso;
    payload = std::move(act.payload);
    stringData = std::move(act.stringData);
    return *this;
}

ActionMessage& ActionMessage::operator=(std::unique_ptr<Message> message) noexcept
{
    messageAction = CMD_SEND_MESSAGE;
    messageID = message->messageID;
    payload = std::move(message->data);
    actionTime = message->time;
    stringData = {std::move(message->dest),
                  std::move(message->source),
                  std::move(message->original_source),
                  std::move(message->original_dest)};
    return *this;
}

void ActionMessage::setAction(action_message_def::action_t newAction)
{
    messageAction = newAction;
}

static const std::string emptyStr;
const std::string& ActionMessage::getString(int index) const
{
    if (isValidIndex(index, stringData)) {
        return stringData[index];
    }
    return emptyStr;
}

void ActionMessage::setString(int index, std::string_view str)
{
    if (index >= 256 || index < 0) {
        throw(std::invalid_argument("index out of specified range (0-255)"));
    }
    if (index >= static_cast<int>(stringData.size())) {
        stringData.resize(static_cast<size_t>(index) + 1);
    }
    stringData[index] = str;
}

/** check for little endian*/
static inline std::uint8_t isLittleEndian()
{
    static std::int32_t test{1};
    return (*reinterpret_cast<std::int8_t*>(&test) == 1) ? std::uint8_t(1) : 0;
}

// action_message_base_size= 7 header fields(7*4 bytes)+flags(2 bytes)+counter(2 bytes)+time(8
// bytes)+payload size(4 bytes)+1 byte for number of strings=45
static constexpr std::size_t action_message_base_size =
    7 * sizeof(uint32_t) + 2 * sizeof(uint16_t) + sizeof(Time::baseType) + sizeof(int32_t) + 1;

int ActionMessage::toByteArray(std::byte* data, std::size_t buffer_size) const
{
    static const uint8_t littleEndian = isLittleEndian();
    // put the main string size in the first 4 bytes;
    std::uint32_t ssize = (messageAction != CMD_TIME_REQUEST) ?
        static_cast<uint32_t>(payload.size() & 0x00FFFFFFUL) :
        0UL;

    if ((data == nullptr) || (buffer_size == 0) || buffer_size < action_message_base_size + ssize) {
        return -1;
    }

    std::byte* dataStart = data;

    *data = std::byte{littleEndian};
    data[1] = std::byte(ssize >> 16U);
    data[2] = std::byte((ssize >> 8U) & 0xFFU);
    data[3] = std::byte(ssize & 0xFFU);
    data += sizeof(uint32_t);  // 4
    *reinterpret_cast<action_message_def::action_t*>(data) = messageAction;
    data += sizeof(action_message_def::action_t);
    *reinterpret_cast<int32_t*>(data) = messageID;
    data += sizeof(int32_t);  // 8
    *reinterpret_cast<int32_t*>(data) = source_id.baseValue();
    data += sizeof(int32_t);  // 12
    *reinterpret_cast<int32_t*>(data) = source_handle.baseValue();
    data += sizeof(int32_t);  // 16
    *reinterpret_cast<int32_t*>(data) = dest_id.baseValue();
    data += sizeof(int32_t);  // 20
    *reinterpret_cast<int32_t*>(data) = dest_handle.baseValue();
    data += sizeof(int32_t);  // 24
    *reinterpret_cast<uint16_t*>(data) = counter;
    data += sizeof(uint16_t);  // 26
    *reinterpret_cast<uint16_t*>(data) = flags;
    data += sizeof(uint16_t);  // 28
    *reinterpret_cast<int32_t*>(data) = sequenceID;
    data += sizeof(int32_t);  // 32
    auto bt = actionTime.getBaseTimeCode();
    std::memcpy(data, &(bt), sizeof(Time::baseType));
    data += sizeof(Time::baseType);  // 40

    if (messageAction == CMD_TIME_REQUEST) {
        bt = Te.getBaseTimeCode();
        std::memcpy(data, &(bt), sizeof(Time::baseType));
        data += sizeof(Time::baseType);
        bt = Tdemin.getBaseTimeCode();
        std::memcpy(data, &(bt), sizeof(Time::baseType));
        data += sizeof(Time::baseType);
        bt = Tso.getBaseTimeCode();
        std::memcpy(data, &(bt), sizeof(Time::baseType));
        data += sizeof(Time::baseType);
        *data = std::byte{0};
        ++data;
        return static_cast<int>(data - dataStart);
    }

    if (ssize > 0) {
        std::memcpy(data, payload.data(), ssize);
        data += ssize;
    }

    //  if (stringData.empty()) {
    //      *data = 0;
    //     ++data;
    // } else {
    *data = std::byte(stringData.size());
    ++data;
    ssize += action_message_base_size;
    for (const auto& str : stringData) {
        auto strsize = static_cast<uint32_t>(str.size());
        if (buffer_size < ssize) {
            return -1;
        }

        std::memcpy(data, &strsize, sizeof(uint32_t));
        data += sizeof(uint32_t);
        std::memcpy(data, str.data(), str.size());
        data += str.size();
    }
    //   }
    auto actSize = static_cast<int>(data - dataStart);
    return actSize;
}

int ActionMessage::serializedByteCount() const
{
    int size{action_message_base_size};

    // for time request add an additional 3*8 bytes
    if (messageAction == CMD_TIME_REQUEST) {
        size += static_cast<int>(3 * sizeof(Time::baseType));
        return size;
    }
    size += static_cast<int>(payload.size());
    // add additional string data
    //   if (!stringData.empty()) {
    for (const auto& str : stringData) {
        // 4(to store the length)+length of the string
        size += static_cast<int>(sizeof(uint32_t) + str.size());
    }
    // }
    return size;
}

std::string ActionMessage::to_string() const
{
    std::string data;
    auto sz = serializedByteCount();
    data.resize(sz);
    toByteArray(reinterpret_cast<std::byte*>(&(data[0])), sz);
    return data;
}

constexpr auto LEADING_CHAR = '\xF3';
constexpr auto TAIL_CHAR1 = '\xFA';
constexpr auto TAIL_CHAR2 = '\xFC';

std::string ActionMessage::packetize() const
{
    std::string data;
    packetize(data);
    return data;
}

void ActionMessage::packetize(std::string& data) const
{
    auto sz = serializedByteCount();
    data.resize(sizeof(uint32_t) + static_cast<size_t>(sz));
    toByteArray(reinterpret_cast<std::byte*>(&(data[4])), sz);

    data[0] = LEADING_CHAR;
    // now generate a length header
    auto dsz = static_cast<uint32_t>(data.size());
    data[1] = static_cast<char>(((dsz >> 16U) & 0xFFU));
    data[2] = static_cast<char>(((dsz >> 8U) & 0xFFU));
    data[3] = static_cast<char>(dsz & 0xFFU);
    data.push_back(TAIL_CHAR1);
    data.push_back(TAIL_CHAR2);
}

std::vector<char> ActionMessage::to_vector() const
{
    std::vector<char> data;
    auto sz = serializedByteCount();
    data.resize(sz);
    toByteArray(reinterpret_cast<std::byte*>(data.data()), sz);
    return data;
}

void ActionMessage::to_vector(std::vector<char>& data) const
{
    auto sz = serializedByteCount();
    data.resize(sz);
    toByteArray(reinterpret_cast<std::byte*>(data.data()), sz);
}

void ActionMessage::to_string(std::string& data) const
{
    auto sz = serializedByteCount();
    data.resize(sz);
    toByteArray(reinterpret_cast<std::byte*>(&(data[0])), sz);
}

template<std::size_t DataSize>
inline void swap_bytes(std::uint8_t* data)
{
    for (std::size_t i = 0, end = DataSize / 2; i < end; ++i) {
        std::swap(data[i], data[DataSize - i - 1]);
    }
}

std::size_t ActionMessage::fromByteArray(const std::byte* data, std::size_t buffer_size)
{
    std::size_t tsize{action_message_base_size};
    static const uint8_t littleEndian = isLittleEndian();
    if (buffer_size < tsize) {
        messageAction = CMD_INVALID;
        return (0);
    }
    if (data[0] == std::byte(LEADING_CHAR)) {
        auto res = depacketize(data, buffer_size);
        if (res > 0) {
            return static_cast<int>(res);
        }
    }
    std::size_t sz = 256 * 256 * (std::to_integer<std::size_t>(data[1])) +
        256 * std::to_integer<std::size_t>(data[2]) + std::to_integer<std::size_t>(data[3]);
    tsize += sz;
    if (buffer_size < tsize) {
        messageAction = CMD_INVALID;
        return (0);
    }
    bool swap = (data[0] != std::byte{littleEndian});
    data += sizeof(uint32_t);
    memcpy(&messageAction, data, sizeof(action_message_def::action_t));
    // messageAction = *reinterpret_cast<const action_message_def::action_t *> (data);
    if (swap) {
        swap_bytes<sizeof(action_message_def::action_t)>(
            reinterpret_cast<std::uint8_t*>(&messageAction));
    }
    data += sizeof(action_message_def::action_t);
    // messageID = *reinterpret_cast<const int32_t *> (data);
    memcpy(&messageID, data, sizeof(int32_t));
    data += sizeof(int32_t);
    // source_id = GlobalFederateId{*reinterpret_cast<const int32_t *> (data)};
    memcpy(&source_id, data, sizeof(GlobalFederateId));
    data += sizeof(GlobalFederateId);
    // source_handle = InterfaceHandle{*reinterpret_cast<const int32_t *> (data)};
    memcpy(&source_handle, data, sizeof(InterfaceHandle));
    data += sizeof(InterfaceHandle);
    // dest_id = GlobalFederateId{*reinterpret_cast<const int32_t *> (data)};
    memcpy(&dest_id, data, sizeof(GlobalFederateId));
    data += sizeof(GlobalFederateId);
    // dest_handle = InterfaceHandle{*reinterpret_cast<const int32_t *> (data)};
    memcpy(&dest_handle, data, sizeof(InterfaceHandle));
    data += sizeof(InterfaceHandle);
    // counter = *reinterpret_cast<const uint16_t *> (data);
    memcpy(&counter, data, sizeof(uint16_t));
    data += sizeof(uint16_t);
    // flags = *reinterpret_cast<const uint16_t *> (data);
    memcpy(&flags, data, sizeof(uint16_t));
    data += sizeof(uint16_t);
    // sequenceID = *reinterpret_cast<const uint32_t *> (data);
    memcpy(&sequenceID, data, sizeof(uint32_t));
    data += sizeof(uint32_t);
    Time::baseType btc;
    memcpy(&btc, data, sizeof(Time::baseType));
    actionTime.setBaseTimeCode(btc);
    data += sizeof(Time::baseType);

    if (messageAction == CMD_TIME_REQUEST) {
        tsize += static_cast<int>(3 * sizeof(Time::baseType));
        if (buffer_size < tsize) {
            messageAction = CMD_INVALID;
            return (0);
        }
        memcpy(&btc, data, sizeof(Time::baseType));
        Te.setBaseTimeCode(btc);
        data += sizeof(Time::baseType);
        memcpy(&btc, data, sizeof(Time::baseType));
        Tdemin.setBaseTimeCode(btc);
        data += sizeof(Time::baseType);
        memcpy(&btc, data, sizeof(Time::baseType));
        Tso.setBaseTimeCode(btc);
        data += sizeof(Time::baseType);
    } else {
        Te = timeZero;
        Tdemin = timeZero;
        Tso = timeZero;
    }
    if (sz > 0) {
        payload.assign(data, sz);
        data += sz;
    }
    auto stringCount = std::to_integer<std::size_t>(*data);
    ++data;
    if (stringCount != 0) {
        stringData.resize(stringCount);
        tsize += 4 * stringCount;
        if (buffer_size < tsize) {
            messageAction = CMD_INVALID;
            return (0);
        }

        for (std::size_t ii = 0; ii < stringCount; ++ii) {
            uint32_t ssize;
            memcpy(&ssize, data, sizeof(uint32_t));
            data += sizeof(uint32_t);
            if (swap) {
                swap_bytes<4>(reinterpret_cast<std::uint8_t*>(&ssize));
            }
            tsize += ssize;
            if (buffer_size < tsize) {
                messageAction = CMD_INVALID;
                return (0);
            }
            stringData[ii].assign(reinterpret_cast<const char*>(data), ssize);
            data += ssize;
        }
    } else {
        stringData.clear();
    }

    if (swap) {
        swap_bytes<4>(reinterpret_cast<std::uint8_t*>(&messageID));
        swap_bytes<4>(reinterpret_cast<std::uint8_t*>(&source_id));
        swap_bytes<4>(reinterpret_cast<std::uint8_t*>(&source_handle));
        swap_bytes<4>(reinterpret_cast<std::uint8_t*>(&dest_id));
        swap_bytes<4>(reinterpret_cast<std::uint8_t*>(&dest_handle));
        swap_bytes<2>(reinterpret_cast<std::uint8_t*>(&counter));
        swap_bytes<2>(reinterpret_cast<std::uint8_t*>(&flags));
        auto timecode = actionTime.getBaseTimeCode();
        swap_bytes<sizeof(Time::baseType)>(reinterpret_cast<std::uint8_t*>(&timecode));
        actionTime.setBaseTimeCode(timecode);
        if (messageAction == CMD_TIME_REQUEST) {
            timecode = Te.getBaseTimeCode();
            swap_bytes<sizeof(Time::baseType)>(reinterpret_cast<std::uint8_t*>(&timecode));
            Te.setBaseTimeCode(timecode);
            timecode = Tdemin.getBaseTimeCode();
            swap_bytes<sizeof(Time::baseType)>(reinterpret_cast<std::uint8_t*>(&timecode));
            Tdemin.setBaseTimeCode(timecode);
            timecode = Tso.getBaseTimeCode();
            swap_bytes<sizeof(Time::baseType)>(reinterpret_cast<std::uint8_t*>(&timecode));
            Tso.setBaseTimeCode(timecode);
        }
    }
    return tsize;
}

int ActionMessage::depacketize(const void* data, std::size_t buffer_size)
{
    const std::byte* bytes = reinterpret_cast<const std::byte*>(data);
    if (bytes[0] != std::byte(LEADING_CHAR)) {
        return 0;
    }
    if (buffer_size < 6) {
        return 0;
    }
    unsigned int message_size = std::to_integer<unsigned char>(bytes[1]);
    message_size <<= 8U;
    message_size += static_cast<unsigned char>(bytes[2]);
    message_size <<= 8U;
    message_size += static_cast<unsigned char>(bytes[3]);
    if (buffer_size < static_cast<size_t>(message_size + 2)) {
        return 0;
    }
    if (bytes[message_size] != std::byte(TAIL_CHAR1)) {
        return 0;
    }
    if (bytes[message_size + 1] != std::byte(TAIL_CHAR2)) {
        return 0;
    }

    std::size_t bytesUsed = fromByteArray(bytes + 4, message_size - 4);
    return (bytesUsed > 0) ? message_size + 2 : 0;
}

void ActionMessage::from_string(std::string_view data)
{
    fromByteArray(reinterpret_cast<const std::byte*>(data.data()), data.size());
}

void ActionMessage::from_vector(const std::vector<char>& data)
{
    fromByteArray(reinterpret_cast<const std::byte*>(data.data()), data.size());
}

std::unique_ptr<Message> createMessageFromCommand(const ActionMessage& cmd)
{
    auto msg = std::make_unique<Message>();
    switch (cmd.stringData.size()) {
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

std::unique_ptr<Message> createMessageFromCommand(ActionMessage&& cmd)
{
    auto msg = std::make_unique<Message>();
    switch (cmd.stringData.size()) {
        case 0:
            break;
        case 1:
            msg->dest = std::move(cmd.stringData[0]);
            break;
        case 2:
            msg->dest = std::move(cmd.stringData[0]);
            msg->source = std::move(cmd.stringData[1]);
            break;
        case 3:
            msg->dest = std::move(cmd.stringData[0]);
            msg->source = std::move(cmd.stringData[1]);
            msg->original_source = std::move(cmd.stringData[2]);
            break;
        default:
            msg->dest = std::move(cmd.stringData[0]);
            msg->source = std::move(cmd.stringData[1]);
            msg->original_source = std::move(cmd.stringData[2]);
            msg->original_dest = std::move(cmd.stringData[3]);
            break;
    }
    msg->data = std::move(cmd.payload);
    msg->time = cmd.actionTime;
    msg->messageID = cmd.messageID;
    return msg;
}

static constexpr char unknownStr[] = "unknown";

// Map to translate the action to a description
static constexpr frozen::unordered_map<action_message_def::action_t, frozen::string, 93>
    actionStrings = {
        // priority commands
        {action_message_def::action_t::cmd_priority_disconnect, "priority_disconnect"},
        {action_message_def::action_t::cmd_disconnect, "disconnect"},
        {action_message_def::action_t::cmd_disconnect_name, "disconnect by name"},
        {action_message_def::action_t::cmd_user_disconnect, "disconnect from user"},
        {action_message_def::action_t::cmd_broadcast_disconnect, "broadcast disconnect"},
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
        {action_message_def::action_t::cmd_disconnect_broker_ack, "disconnect broker acknowledge"},
        {action_message_def::action_t::cmd_disconnect_core_ack, "disconnect core acknowledge"},
        {action_message_def::action_t::cmd_disconnect_fed_ack, "disconnect fed acknowledge"},
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
        {action_message_def::action_t::cmd_remove_publication, "remove publisher"},
        {action_message_def::action_t::cmd_reg_filter, "reg_filter"},
        {action_message_def::action_t::cmd_add_filter, "add_filter"},
        {action_message_def::action_t::cmd_remove_filter, "remove filter"},
        {action_message_def::action_t::cmd_filter_link, "link filter"},
        {action_message_def::action_t::cmd_data_link, "data link"},
        {action_message_def::action_t::cmd_reg_input, "reg_input"},
        {action_message_def::action_t::cmd_add_subscriber, "add_subscriber"},
        {action_message_def::action_t::cmd_remove_subscriber, "remove subscriber"},
        {action_message_def::action_t::cmd_reg_end, "reg_end"},
        {action_message_def::action_t::cmd_resend, "reg_resend"},
        {action_message_def::action_t::cmd_add_endpoint, "add_endpoint"},
        {action_message_def::action_t::cmd_remove_endpoint, "remove endpoint"},
        {action_message_def::action_t::cmd_add_named_endpoint, "add_named_endpoint"},
        {action_message_def::action_t::cmd_add_named_input, "add_named_input"},
        {action_message_def::action_t::cmd_add_named_publication, "add_named_publication"},
        {action_message_def::action_t::cmd_add_named_filter, "add_named_filter"},
        {action_message_def::action_t::cmd_remove_named_endpoint, "remove_named_endpoint"},
        {action_message_def::action_t::cmd_disconnect_fed, "disconnect_fed"},
        {action_message_def::action_t::cmd_disconnect_broker, "disconnect_broker"},
        {action_message_def::action_t::cmd_disconnect_core, "disconnect_core"},
        {action_message_def::action_t::cmd_remove_named_input, "remove_named_input"},
        {action_message_def::action_t::cmd_remove_named_publication, "remove_named_publication"},
        {action_message_def::action_t::cmd_remove_named_filter, "remove_named_filter"},
        {action_message_def::action_t::cmd_close_interface, "close_interface"},
        {action_message_def::action_t::cmd_multi_message, "multi message"},
        {action_message_def::action_t::cmd_broker_configure, "broker_configure"},
        {action_message_def::action_t::cmd_time_barrier_request, "request time barrier"},
        {action_message_def::action_t::cmd_time_barrier, "time barrier"},
        {action_message_def::action_t::cmd_time_barrier_clear, "clear time barrier"},
        // protocol messages are meant for the communication standard and are not used in the
        // Cores/Brokers
        {action_message_def::action_t::cmd_protocol_priority, "protocol_priority"},
        {action_message_def::action_t::cmd_protocol, "protocol"},
        {action_message_def::action_t::cmd_protocol_big, "protocol_big"}};

const char* actionMessageType(action_message_def::action_t action)
{
    const auto* res = actionStrings.find(action);
    return (res != actionStrings.end()) ? res->second.data() : static_cast<const char*>(unknownStr);
}

// set of strings to translate error codes to something sensible
static constexpr frozen::unordered_map<int, frozen::string, 6> errorStrings = {
    {connection_error_code, "connection error"},
    {lost_server_connection_code, "lost connection with server"},
    {already_init_error_code, "already in initialization mode"},
    {duplicate_federate_name_error_code, "duplicate federate name detected"},
    {duplicate_broker_name_error_code, "duplicate broker name detected"},
    {mismatch_broker_key_error_code, "Broker key does not match"}};

const char* commandErrorString(int errorcode)
{
    const auto* res = errorStrings.find(errorcode);
    return (res != errorStrings.end()) ? res->second.data() : static_cast<const char*>(unknownStr);
}

std::string errorMessageString(const ActionMessage& command)
{
    if (checkActionFlag(command, error_flag)) {
        const auto& estring = command.getString(0);
        if (estring.empty()) {
            return commandErrorString(command.messageID);
        }
        return estring;
    }
    return std::string{};
}

std::string prettyPrintString(const ActionMessage& command)
{
    std::string ret(actionMessageType(command.action()));
    if (ret == unknownStr) {
        ret += " " + std::to_string(static_cast<int>(command.action()));
        return ret;
    }
    switch (command.action()) {
        case CMD_REG_FED:
            ret.push_back(':');
            ret.append(command.name());
            break;
        case CMD_FED_ACK:
            ret.push_back(':');
            ret.append(command.name());
            ret.append("--");
            if (checkActionFlag(command, error_flag)) {
                ret.append("error");
            } else {
                ret.append(std::to_string(command.dest_id.baseValue()));
            }
            break;
        case CMD_PUB:
            ret.push_back(':');
            ret.append(fmt::format("From ({}) handle({}) size {} at {} to {}",
                                   command.source_id.baseValue(),
                                   command.dest_handle.baseValue(),
                                   command.payload.size(),
                                   static_cast<double>(command.actionTime),
                                   command.dest_id.baseValue()));
            break;
        case CMD_REG_BROKER:
            ret.push_back(':');
            ret.append(command.name());
            break;
        case CMD_TIME_GRANT:
            ret.push_back(':');
            ret.append(fmt::format("From ({}) Granted Time({}) to ({})",
                                   command.source_id.baseValue(),
                                   static_cast<double>(command.actionTime),
                                   command.dest_id.baseValue()));
            break;
        case CMD_TIME_REQUEST:
            ret.push_back(':');
            ret.append(fmt::format("From ({}) Time({}, {}, {}) to ({})",
                                   command.source_id.baseValue(),
                                   static_cast<double>(command.actionTime),
                                   static_cast<double>(command.Te),
                                   static_cast<double>(command.Tdemin),
                                   command.dest_id.baseValue()));
            break;
        case CMD_FED_CONFIGURE_TIME:
        case CMD_FED_CONFIGURE_INT:
        case CMD_FED_CONFIGURE_FLAG:
            break;
        case CMD_SEND_MESSAGE:
            ret.push_back(':');
            ret.append(fmt::format("From ({})({}:{}) To {} size {} at {}",
                                   command.getString(origSourceStringLoc),
                                   command.source_id.baseValue(),
                                   command.source_handle.baseValue(),
                                   command.getString(targetStringLoc),
                                   command.payload.size(),
                                   static_cast<double>(command.actionTime)));
            break;
        default:
            ret.append(fmt::format(":From {}", command.source_id.baseValue()));
            break;
    }
    return ret;
}

std::ostream& operator<<(std::ostream& os, const ActionMessage& command)
{
    os << prettyPrintString(command);
    return os;
}

int appendMessage(ActionMessage& m, const ActionMessage& newMessage)
{
    if (m.action() == CMD_MULTI_MESSAGE) {
        if (m.counter < 255) {
            m.setString(m.counter++, newMessage.to_string());
            return m.counter;
        }
    }
    return (-1);
}

void setIterationFlags(ActionMessage& command, iteration_request iterate)
{
    switch (iterate) {
        case iteration_request::force_iteration:
            setActionFlag(command, iteration_requested_flag);
            setActionFlag(command, required_flag);
            break;
        case iteration_request::iterate_if_needed:
            setActionFlag(command, iteration_requested_flag);
            break;
        case iteration_request::no_iterations:
            break;
    }
}
}  // namespace helics
