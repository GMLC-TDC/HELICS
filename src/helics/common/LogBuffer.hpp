/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "gmlc/libguarded/shared_guarded.hpp"
#include "nlohmann/json_fwd.hpp"

#include <atomic>
#include <deque>
#include <functional>
#include <string>
#include <string_view>
#include <tuple>

namespace helics {

/** a threadsafe small buffer for log messages that will store a maximum of mMaxSize messages*/
class LogBuffer {
  private:
    gmlc::libguarded::shared_guarded<std::deque<std::tuple<int, std::string, std::string>>>
        mBuffer{};
    std::atomic<std::size_t> mMaxSize{0};

  public:
    static constexpr std::size_t cDefaultBufferSize{10UL};
    LogBuffer() = default;
    explicit LogBuffer(std::size_t maxSize): mMaxSize(maxSize) {}
    void resize(std::size_t newSize);
    void enable(bool enable = true);
    std::size_t capacity() const { return mMaxSize; }
    std::size_t size() const { return mBuffer.lock()->size(); }

    void push(int logLevel, std::string_view header, std::string_view message);

    void
        process(const std::function<void(int, std::string_view, std::string_view)>& procFunc) const;
};

/** helper function to write a log buffer to a json object*/
void bufferToJson(const LogBuffer& buffer, nlohmann::json& base);

}  // namespace helics
