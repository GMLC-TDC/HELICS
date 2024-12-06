/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

// For warnings about constexpr paths in visual studio from frozen libraries
#if defined(_MSC_VER)
#    pragma warning(disable : 4127 4245)
#endif

#include "ActionMessage.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "flagOperations.hpp"
#include "gmlc/utilities/base64.h"

#include <algorithm>
#include <complex>
#include <cstring>
#include <fmt/format.h>
#include <frozen/string.h>
#include <frozen/unordered_map.h>
#include <memory>
#include <ostream>
#include <string>
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
    messageAction(startingAction), source_id(sourceId), dest_id(destId)
{
}

ActionMessage::ActionMessage(ActionMessage&& act) noexcept:
    messageAction(act.messageAction), messageID(act.messageID), source_id(act.source_id),
    source_handle(act.source_handle), dest_id(act.dest_id), dest_handle(act.dest_handle),
    counter(act.counter), flags(act.flags), sequenceID(act.sequenceID), actionTime(act.actionTime),
    Te(act.Te), Tdemin(act.Tdemin), Tso(act.Tso), payload(std::move(act.payload)),
    stringData(std::move(act.stringData))
{
}

ActionMessage::ActionMessage(const ActionMessage& act):
    messageAction(act.messageAction), messageID(act.messageID), source_id(act.source_id),
    source_handle(act.source_handle), dest_id(act.dest_id), dest_handle(act.dest_handle),
    counter(act.counter), flags(act.flags), sequenceID(act.sequenceID), actionTime(act.actionTime),
    Te(act.Te), Tdemin(act.Tdemin), Tso(act.Tso), payload(act.payload), stringData(act.stringData)

{
}

ActionMessage::ActionMessage(std::unique_ptr<Message> message):
    messageAction(CMD_SEND_MESSAGE), messageID(message->messageID), flags(message->flags),
    actionTime(message->time), payload(std::move(message->data)),
    stringData({std::move(message->dest),
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
    from_string(std::string_view(static_cast<const char*>(data), size));
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
    sequenceID = act.sequenceID;
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
    sequenceID = act.sequenceID;
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
    flags = message->flags;
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

// NOLINTNEXTLINE
static const std::string emptyStr;
const std::string& ActionMessage::getString(int index) const
{
    if (isValidIndex(index, stringData)) {
        return stringData[index];
    }
    return emptyStr;
}
static constexpr std::size_t maxPayloadSize{0x00FFFFFFUL};

void ActionMessage::setString(int index, std::string_view str)
{
    if (index >= 255 || index < 0) {
        throw(std::invalid_argument("index out of specified range (0-254)"));
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
    return (*reinterpret_cast<std::int8_t*>(&test) == 1) ? static_cast<std::uint8_t>(1) : 0;
}

// action_message_base_size= 7 header fields(7*4 bytes)+flags(2 bytes)+counter(2 bytes)+time(8
// bytes)+payload size(4 bytes)+1 byte for number of strings=45
static constexpr std::size_t action_message_base_size =
    7 * sizeof(uint32_t) + 2 * sizeof(uint16_t) + sizeof(Time::baseType) + sizeof(int32_t) + 1;

int ActionMessage::toByteArray(std::byte* data, std::size_t buffer_size) const
{
    static const uint8_t littleEndian = isLittleEndian();

    // put the main string size in the first 4 bytes;
    std::uint32_t ssize{0UL};
    if (messageAction != CMD_TIME_REQUEST) {
        if (payload.size() >= maxPayloadSize) {
            ssize = maxPayloadSize;
        } else {
            ssize = static_cast<uint32_t>(payload.size());
        }
    }

    if ((data == nullptr) || (buffer_size == 0) || buffer_size < action_message_base_size + ssize) {
        return -1;
    }

    std::byte* dataStart = data;

    *data = static_cast<std::byte>(littleEndian);
    data[1] = static_cast<std::byte>(ssize >> 16U);
    data[2] = static_cast<std::byte>((ssize >> 8U) & 0xFFU);
    data[3] = static_cast<std::byte>(ssize & 0xFFU);
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
    auto baseTimeCode = actionTime.getBaseTimeCode();
    std::memcpy(data, &(baseTimeCode), sizeof(Time::baseType));
    data += sizeof(Time::baseType);  // 40

    if (messageAction == CMD_TIME_REQUEST) {
        baseTimeCode = Te.getBaseTimeCode();
        std::memcpy(data, &(baseTimeCode), sizeof(Time::baseType));
        data += sizeof(Time::baseType);
        baseTimeCode = Tdemin.getBaseTimeCode();
        std::memcpy(data, &(baseTimeCode), sizeof(Time::baseType));
        data += sizeof(Time::baseType);
        baseTimeCode = Tso.getBaseTimeCode();
        std::memcpy(data, &(baseTimeCode), sizeof(Time::baseType));
        data += sizeof(Time::baseType);
        *data = std::byte{0};
        ++data;
        return static_cast<int>(data - dataStart);
    }

    if (ssize > 0) {
        std::memcpy(data, payload.data(), ssize);
        data += ssize;
    }
    if (payload.size() >= maxPayloadSize) {
        *data = static_cast<std::byte>(stringData.size() + 1);
    } else {
        *data = static_cast<std::byte>(stringData.size());
    }

    ++data;
    ssize += action_message_base_size;
    for (const auto& str : stringData) {
        auto strsize = static_cast<uint32_t>(str.size());
        if (buffer_size < ssize + strsize + 4) {
            return -1;
        }

        std::memcpy(data, &strsize, sizeof(uint32_t));
        data += sizeof(uint32_t);
        std::memcpy(data, str.data(), str.size());
        data += str.size();
        ssize += strsize + 4;
    }
    if (payload.size() > maxPayloadSize) {
        if (buffer_size < ssize + payload.size() - maxPayloadSize + 4) {
            return -1;
        }
        auto strsize = static_cast<uint32_t>(payload.size() - maxPayloadSize);
        std::memcpy(data, &strsize, sizeof(uint32_t));
        data += sizeof(uint32_t);
        std::memcpy(data, payload.data() + maxPayloadSize, payload.size() - maxPayloadSize);
        data += payload.size() - maxPayloadSize;
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
    for (const auto& str : stringData) {
        // 4(to store the length)+length of the string
        size += static_cast<int>(sizeof(uint32_t) + str.size());
    }
    if (payload.size() >= maxPayloadSize) {
        size += 4;
    }
    return size;
}

std::string ActionMessage::to_string() const
{
    std::string data;
    if (checkActionFlag(*this, use_json_serialization_flag)) {
        data = to_json_string();
    } else {
        auto size = serializedByteCount();
        data.resize(size);
        toByteArray(reinterpret_cast<std::byte*>(data.data()), size);
    }
    return data;
}

std::string ActionMessage::to_json_string() const
{
    nlohmann::json packet;
    packet["version"] =
        HELICS_VERSION_MAJOR * 10000 + HELICS_VERSION_MINOR * 100 + HELICS_VERSION_PATCH;
    packet["command"] = static_cast<int>(messageAction);
    packet["messageId"] = messageID;
    packet["sourceId"] = source_id.baseValue();
    packet["sourceHandle"] = source_handle.baseValue();
    packet["destId"] = dest_id.baseValue();
    packet["destHandle"] = dest_handle.baseValue();
    packet["counter"] = counter;
    packet["flags"] = flags;
    packet["sequenceId"] = sequenceID;
    packet["actionTime"] = actionTime.getBaseTimeCode();
    if (messageAction == CMD_TIME_REQUEST) {
        packet["Te"] = Te.getBaseTimeCode();
        packet["Tdemin"] = Tdemin.getBaseTimeCode();
        packet["Tso"] = Tso.getBaseTimeCode();
    }
    packet["payload"] = payload.to_string();
    packet["stringCount"] = static_cast<std::uint32_t>(stringData.size());
    if (!stringData.empty()) {
        nlohmann::json sdata = nlohmann::json::array();
        for (const auto& str : stringData) {
            sdata.push_back(str);
        }
        packet["strings"] = std::move(sdata);
    }
    try {
        return fileops::generateJsonString(packet, false);
    }
    catch (const nlohmann::json::type_error&) {
        packet["encoding"] = "base64";
        packet["payload"] = gmlc::utilities::base64_encode(payload.data(), payload.size());
        if (!stringData.empty()) {
            nlohmann::json sdata = nlohmann::json::array();
            for (const auto& str : stringData) {
                sdata.push_back(gmlc::utilities::base64_encode(str.data(), str.size()));
            }
            packet["strings"] = std::move(sdata);
        }
        return fileops::generateJsonString(packet);
    }
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

std::string ActionMessage::packetize_json() const
{
    std::string data = to_json_string();
    auto dsz = data.size() + sizeof(uint32_t);
    data.insert(data.begin(), 4, LEADING_CHAR);
    data[1] = static_cast<char>(((dsz >> 16U) & 0xFFU));
    data[2] = static_cast<char>(((dsz >> 8U) & 0xFFU));
    data[3] = static_cast<char>(dsz & 0xFFU);
    data.push_back(TAIL_CHAR1);
    data.push_back(TAIL_CHAR2);
    return data;
}

void ActionMessage::packetize(std::string& data) const
{
    auto size = serializedByteCount();
    data.resize(sizeof(uint32_t) + static_cast<size_t>(size));
    toByteArray(reinterpret_cast<std::byte*>(&(data[4])), size);

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
    auto size = serializedByteCount();
    data.resize(size);
    toByteArray(reinterpret_cast<std::byte*>(data.data()), size);
    return data;
}

void ActionMessage::to_vector(std::vector<char>& data) const
{
    auto size = serializedByteCount();
    data.resize(size);
    toByteArray(reinterpret_cast<std::byte*>(data.data()), size);
}

void ActionMessage::to_string(std::string& data) const
{
    auto size = serializedByteCount();
    data.resize(size);
    toByteArray(reinterpret_cast<std::byte*>(data.data()), size);
}

template<std::size_t DataSize>
inline void swap_bytes(std::uint8_t* data)
{
    for (std::size_t ii = 0, end = DataSize / 2; ii < end; ++ii) {
        std::swap(data[ii], data[DataSize - ii - 1]);
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
    if (data[0] == static_cast<std::byte>(LEADING_CHAR)) {
        auto res = depacketize(data, buffer_size);
        if (res > 0) {
            return static_cast<int>(res);
        }
    }
    // this means it probably is a JSON packate
    if (data[0] == std::byte{'{'}) {
        return 0;
    }
    const std::size_t size = 256ULL * 256ULL * (std::to_integer<std::size_t>(data[1])) +
        256ULL * std::to_integer<std::size_t>(data[2]) + std::to_integer<std::size_t>(data[3]);
    tsize += size;
    if (buffer_size < tsize) {
        messageAction = CMD_INVALID;
        return (0);
    }
    const bool swap = (data[0] != static_cast<std::byte>(littleEndian));
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
    memcpy(source_id.getBaseTypePointer(), data, sizeof(GlobalFederateId::BaseType));
    data += sizeof(GlobalFederateId);
    // source_handle = InterfaceHandle{*reinterpret_cast<const int32_t *> (data)};
    memcpy(source_handle.getBaseTypePointer(), data, sizeof(InterfaceHandle::BaseType));
    data += sizeof(InterfaceHandle);
    // dest_id = GlobalFederateId{*reinterpret_cast<const int32_t *> (data)};
    memcpy(dest_id.getBaseTypePointer(), data, sizeof(GlobalFederateId::BaseType));
    data += sizeof(GlobalFederateId);
    // dest_handle = InterfaceHandle{*reinterpret_cast<const int32_t *> (data)};
    memcpy(dest_handle.getBaseTypePointer(), data, sizeof(InterfaceHandle::BaseType));
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
        tsize += 3 * sizeof(Time::baseType);
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
    if (size > 0) {
        payload.assign(data, size);
        data += size;
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
    if (size == maxPayloadSize && !stringData.empty()) {
        payload.append(stringData.back());
        stringData.pop_back();
    }
    return tsize;
}

std::size_t ActionMessage::depacketize(const void* data, std::size_t buffer_size)
{
    const auto* bytes = reinterpret_cast<const std::byte*>(data);
    if (bytes[0] != static_cast<std::byte>(LEADING_CHAR)) {
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
    if (buffer_size < (static_cast<size_t>(message_size) + 2)) {
        return 0;
    }
    if (bytes[message_size] != static_cast<std::byte>(TAIL_CHAR1)) {
        return 0;
    }
    if (bytes[message_size + 1] != static_cast<std::byte>(TAIL_CHAR2)) {
        return 0;
    }

    std::size_t bytesUsed = fromByteArray(bytes + 4, message_size - 4);
    if (bytesUsed == 0U) {
        if (from_json_string(
                std::string_view(reinterpret_cast<const char*>(bytes) + 4, message_size - 4))) {
            bytesUsed = static_cast<std::size_t>(message_size) + 4;
        }
    }
    return (bytesUsed > 0) ? message_size + 2 : 0;
}

std::size_t ActionMessage::from_string(std::string_view data)
{
    auto result = fromByteArray(reinterpret_cast<const std::byte*>(data.data()), data.size());
    if (result == 0U && !data.empty() && data.front() == '{') {
        if (from_json_string(data)) {
            return data.size();
        }
    }
    return result;
}

bool ActionMessage::from_json_string(std::string_view data)
{
    try {
        auto val = fileops::loadJsonStr(data);
        // auto version = val["version"].asFloat();
        messageAction = static_cast<action_message_def::action_t>(val["command"].get<int32_t>());
        messageID = val["messageId"].get<int32_t>();
        source_id = GlobalFederateId(val["sourceId"].get<int32_t>());
        dest_id = GlobalFederateId(val["destId"].get<int32_t>());
        source_handle = InterfaceHandle(val["sourceHandle"].get<int32_t>());
        dest_handle = InterfaceHandle(val["destHandle"].get<int32_t>());
        counter = val["counter"].get<uint16_t>();
        flags = val["flags"].get<uint16_t>();
        sequenceID = val["sequenceId"].get<uint32_t>();
        actionTime.setBaseTimeCode(val["actionTime"].get<int64_t>());
        if (messageAction == CMD_TIME_REQUEST) {
            Te.setBaseTimeCode(val["Te"].get<int64_t>());
            Tdemin.setBaseTimeCode(val["Tdemin"].get<int64_t>());
            Tso.setBaseTimeCode(val["Tso"].get<int64_t>());
        }
        payload = val["payload"].get<std::string>();
        auto stringCount = val["stringCount"].get<int>();
        stringData.resize(stringCount);
        for (int ii = 0; ii < stringCount; ++ii) {
            setString(ii, val["strings"][ii].get<std::string>());
        }
        bool base64_encoding{false};
        if (val.contains("encoding") && val["encoding"].is_string()) {
            base64_encoding = val["encoding"].get<std::string>() == "base64";
        }
        if (base64_encoding) {
            payload = gmlc::utilities::base64_decode_to_string(payload.to_string());
            for (auto& stringd : stringData) {
                stringd = gmlc::utilities::base64_decode_to_string(stringd);
            }
        }
    }
    catch (...) {
        return false;
    }
    return true;
}

std::size_t ActionMessage::from_vector(const std::vector<char>& data)
{
    const std::size_t bytesUsed =
        fromByteArray(reinterpret_cast<const std::byte*>(data.data()), data.size());
    if (bytesUsed == 0 && !data.empty() && data.front() == '{') {
        if (from_json_string(std::string_view(data.data(), data.size()))) {
            return data.size();
        }
    }
    return bytesUsed;
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
    msg->flags = cmd.flags;
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
    msg->flags = cmd.flags;
    msg->messageID = cmd.messageID;
    return msg;
}

static constexpr char unknownStr[] = "unknown";

// Map to translate the action to a description
static constexpr frozen::unordered_map<action_message_def::action_t, std::string_view, 96>
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
        {action_message_def::action_t::cmd_request_current_time, "request current time"},
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
        {action_message_def::action_t::cmd_send_for_dest_filter_return,
         "send_for_dest_filter_return"},
        {action_message_def::action_t::cmd_null_message, "null message"},
        {action_message_def::action_t::cmd_null_dest_message, "null destination message"},

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
static constexpr frozen::unordered_map<int, std::string_view, 8> errorStrings = {
    {connection_error_code, "connection error result"},
    {lost_server_connection_code, "lost connection with server"},
    {already_init_error_code, "already in initialization mode"},
    {duplicate_federate_name_error_code, "duplicate federate name detected"},
    {duplicate_broker_name_error_code, "duplicate broker name detected"},
    {max_federate_count_exceeded, "the maximum number of federates has been reached"},
    {max_broker_count_exceeded, "the maximum number of brokers or cores has been reached"},
    {mismatch_broker_key_error_code, "Broker key does not match"}};

const char* commandErrorString(int errorCode)
{
    const auto* res = errorStrings.find(errorCode);
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
    std::string ret{actionMessageType(command.action())};
    if (std::string_view{ret} == std::string_view{unknownStr}) {
        ret.push_back(' ');
        ret.append(std::to_string(static_cast<int>(command.action())));
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

std::ostream& operator<<(std::ostream& out, const ActionMessage& command)
{
    out << prettyPrintString(command);
    return out;
}

int appendMessage(ActionMessage& multiMessage, const ActionMessage& newMessage)
{
    if (multiMessage.action() == CMD_MULTI_MESSAGE) {
        if (multiMessage.counter < 255) {
            multiMessage.setString(multiMessage.counter++, newMessage.to_string());
            return multiMessage.counter;
        }
    }
    return (-1);
}

void setIterationFlags(ActionMessage& command, IterationRequest iterate)
{
    switch (iterate) {
        case IterationRequest::FORCE_ITERATION:
            setActionFlag(command, iteration_requested_flag);
            setActionFlag(command, required_flag);
            break;
        case IterationRequest::ITERATE_IF_NEEDED:
            setActionFlag(command, iteration_requested_flag);
            break;
        case IterationRequest::ERROR_CONDITION:
            setActionFlag(command, error_flag);
            break;
        case IterationRequest::NO_ITERATIONS:
        default:
            break;
    }
}
}  // namespace helics
