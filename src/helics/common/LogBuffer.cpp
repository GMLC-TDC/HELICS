/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "LogBuffer.hpp"

#include "nlohmann/json.hpp"

#include <string>
#include <utility>

namespace helics {

void LogBuffer::resize(std::size_t newSize)
{
    if (newSize < mMaxSize) {
        auto block = mBuffer.lock();
        while (block->size() > newSize) {
            block->pop_front();
        }
    }

    mMaxSize = newSize;
}

void LogBuffer::enable(bool enabled)
{
    if (enabled) {
        if (mMaxSize == 0) {
            resize(cDefaultBufferSize);
        }
    } else {
        resize(0);
    }
}

void LogBuffer::push(int logLevel, std::string_view header, std::string_view message)
{
    if (mMaxSize.load() == 0) {
        return;
    }
    auto block = mBuffer.lock();
    if (block->size() == mMaxSize.load()) {
        block->pop_front();
    }
    block->emplace_back(logLevel, header, message);
}

void LogBuffer::process(
    const std::function<void(int, std::string_view, std::string_view)>& procFunc) const
{
    if (!procFunc) {
        return;
    }
    auto block = mBuffer.lock_shared();
    for (const auto& logMessage : *block) {
        procFunc(std::get<0>(logMessage), std::get<1>(logMessage), std::get<2>(logMessage));
    }
}

void bufferToJson(const LogBuffer& buffer, nlohmann::json& base)
{
    base["logs"] = nlohmann::json::array();
    buffer.process([&base](int level, std::string_view header, std::string_view message) {
        nlohmann::json logBlock;
        logBlock["level"] = level;
        logBlock["header"] = std::string(header);
        logBlock["message"] = std::string(message);
        base["logs"].push_back(std::move(logBlock));
    });
}

}  // namespace helics
