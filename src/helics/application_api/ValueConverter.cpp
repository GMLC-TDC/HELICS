/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ValueConverter.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../common/frozen_map.h"

#include <complex>
#include <vector>

namespace helics {

namespace detail {
    namespace checks {
        /*! This code in checks namespace exerpted from cereal portable binary archives
        code is modified slightly to fit name conventions and make use of C++17

        \file binary.hpp
    \brief Binary input and output archives */
        /*
          Copyright (c) 2014, Randolph Voorhies, Shane Grant
          All rights reserved.

          Redistribution and use in source and binary forms, with or without
          modification, are permitted provided that the following conditions are met:
              * Redistributions of source code must retain the above copyright
                notice, this list of conditions and the following disclaimer.
              * Redistributions in binary form must reproduce the above copyright
                notice, this list of conditions and the following disclaimer in the
                documentation and/or other materials provided with the distribution.
              * Neither the name of cereal nor the
                names of its contributors may be used to endorse or promote products
                derived from this software without specific prior written permission.

          THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
          ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
          WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
          DISCLAIMED. IN NO EVENT SHALL RANDOLPH VOORHIES OR SHANE GRANT BE LIABLE FOR ANY
          DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
          (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
          LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
          ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
          (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
          SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
        */
        //! Returns true if the current machine is little endian
        /*! @ingroup Internal */
        inline bool isLittleEndian()
        {
            static constexpr std::int32_t test{1};
            return *reinterpret_cast<const char*>(&test) == 1;
        }

        //! Swaps the order of bytes for some chunk of memory
        /*! @param data The data as a uint8_t pointer
            @tparam DataSize The true size of the data
            @ingroup Internal */
        template<std::size_t DataSize>
        inline void swapBytes(std::byte* data)
        {
            for (std::size_t i = 0, end = DataSize / 2; i < end; ++i) {
                std::swap(data[i], data[DataSize - i - 1]);
            }
        }
    }  // namespace checks
    static constexpr const std::byte doubleCode{0xB0};
    static constexpr const std::byte intCode{0x50};
    static constexpr const std::byte complexCode{0x12};
    static constexpr const std::byte stringCode{0x0E};
    static constexpr const std::byte vectorCode{0x6C};
    static constexpr const std::byte npCode{0xAE};
    static constexpr const std::byte cvCode{0x62};
    static constexpr const std::byte customCode{0xF4};

    static constexpr std::byte endianMask{0x01};
    static constexpr std::byte lowByteMask{0xFF};
    static constexpr std::byte codeMask{0xFE};

    static const std::byte endianCode = checks::isLittleEndian() ? std::byte{0} : std::byte{1};

    static constexpr const std::byte littleEndianCode{0x0};
    static constexpr const std::byte bigEndianCode{0x01};

    static inline void addCodeAndSize(std::byte* data, std::byte code, size_t size)
    {
        std::memset(data, 0, 8);
        data[0] = code;
        data[3] = littleEndianCode;
        data[4] = std::byte((size >> 24U) & 0xFFU);
        data[5] = std::byte((size >> 16U) & 0xFFU);
        data[6] = std::byte((size >> 8U) & 0xFFU);
        data[7] = std::byte((size & 0xFFU));
    }

    size_t convertToBinary(std::byte* data, double val)
    {
        addCodeAndSize(data, doubleCode, 1);
        std::memcpy(data + 8, &val, 8);
        return 16;
    }

    size_t convertToBinary(std::byte* data, std::int64_t val)
    {
        addCodeAndSize(data, intCode, 1);
        std::memcpy(data + 8, &val, 8);
        return 16;
    }

    size_t convertToBinary(std::byte* data, std::complex<double> val)
    {
        addCodeAndSize(data, complexCode, 2);
        std::memcpy(data + 8, &val, 16);
        return 24;
    }

    size_t convertToBinary(std::byte* data, std::string_view val)
    {
        addCodeAndSize(data, stringCode, val.size());
        if (!val.empty()) {
            std::memcpy(data + 8U, val.data(), val.size());
        }
        return val.size() + 8U;
    }

    size_t convertToBinary(std::byte* data, const NamedPoint& val)
    {
        addCodeAndSize(data, npCode, val.name.size());
        std::memcpy(data + 8, &val.value, 8);
        if (!val.name.empty()) {
            std::memcpy(data + 16, val.name.data(), val.name.size());
        }
        return val.name.size() + 16U;
    }

    size_t convertToBinary(std::byte* data, const std::vector<double>& val)
    {
        addCodeAndSize(data, vectorCode, val.size());
        if (val.size() > 0) {
            std::memcpy(data + 8, val.data(), val.size() * sizeof(double));
        }
        return val.size() * sizeof(double) + 8U;
    }

    size_t convertToBinary(std::byte* data, const double* val, size_t size)
    {
        addCodeAndSize(data, vectorCode, size);
        if (size > 0) {
            std::memcpy(data + 8, val, size * sizeof(double));
        }
        return size * sizeof(double) + 8U;
    }

    size_t convertToBinary(std::byte* data, const std::vector<std::complex<double>>& val)
    {
        addCodeAndSize(data, cvCode, val.size());
        if (val.size() > 0) {
            std::memcpy(data + 8, val.data(), val.size() * sizeof(double) * 2);
        }
        return val.size() * sizeof(double) * 2U + 8U;
    }

    size_t convertToBinary(std::byte* data, const std::complex<double>* val, size_t size)
    {
        addCodeAndSize(data, cvCode, size);
        std::memcpy(data + 8, val, size * sizeof(double) * 2);
        return size * sizeof(double) * 2U + 8U;
    }

    size_t getDataSize(const std::byte* data)
    {
        return (std::to_integer<size_t>(data[4]) << 24U) +
            (std::to_integer<size_t>(data[5]) << 16U) + (std::to_integer<size_t>(data[6]) << 8U) +
            std::to_integer<size_t>(data[7]);
    }

    static constexpr const frozen::unordered_map<std::int8_t, helics::data_type, 8> typeDetect{
        {std::to_integer<std::int8_t>(intCode), data_type::helics_int},
        {std::to_integer<std::int8_t>(doubleCode), data_type::helics_double},
        {std::to_integer<std::int8_t>(complexCode), data_type::helics_complex},
        {std::to_integer<std::int8_t>(vectorCode), data_type::helics_vector},
        {std::to_integer<std::int8_t>(cvCode), data_type::helics_complex_vector},
        {std::to_integer<std::int8_t>(npCode), data_type::helics_named_point},
        {std::to_integer<std::int8_t>(customCode), data_type::helics_custom},
        {std::to_integer<std::int8_t>(stringCode), data_type::helics_string}};

    data_type detectType(const std::byte* data)
    {
        const auto* res = typeDetect.find(std::to_integer<std::int8_t>(data[0]));
        return (res != typeDetect.end()) ? res->second : data_type::helics_unknown;
    }

    void convertFromBinary(const std::byte* data, double& val)
    {
        std::memcpy(&val, data + 8, 8);
        if ((data[0] & endianMask) != littleEndianCode) {
            checks::swapBytes<8>(reinterpret_cast<std::byte*>(&val));
        }
    }

    void convertFromBinary(const std::byte* data, std::int64_t& val)
    {
        std::memcpy(&val, data + 8, 8);
        if ((data[0] & endianMask) != littleEndianCode) {
            checks::swapBytes<8>(reinterpret_cast<std::byte*>(&val));
        }
    }

    void convertFromBinary(const std::byte* data, std::complex<double>& val)
    {
        std::memcpy(&val, data + 8, 16);
        if ((data[0] & endianMask) != littleEndianCode) {
            checks::swapBytes<8>(reinterpret_cast<std::byte*>(&val));
            checks::swapBytes<8>(reinterpret_cast<std::byte*>(&val) + 8);
        }
    }

    void convertFromBinary(const std::byte* data, std::string& val)
    {
        std::size_t size = getDataSize(data);
        val.assign(reinterpret_cast<const char*>(data) + 8,
                   reinterpret_cast<const char*>(data) + 8 + size);
    }

    void convertFromBinary(const std::byte* data, char* val)
    {
        std::size_t size = getDataSize(data);
        memcpy(val, data + 8, size);
    }

    void convertFromBinary(const std::byte* data, NamedPoint& val)
    {
        std::memcpy(&val.value, data + 8, 8);
        std::size_t size = getDataSize(data);
        val.name.assign(reinterpret_cast<const char*>(data) + 16U,
                        reinterpret_cast<const char*>(data) + 16U + size);
        if ((data[0] & endianMask) != littleEndianCode) {
            checks::swapBytes<8>(reinterpret_cast<std::byte*>(&val.value));
        }
    }

    void convertFromBinary(const std::byte* data, std::vector<double>& val)
    {
        std::size_t size = getDataSize(data);
        val.resize(size);
        if (size > 0) {
            std::memcpy(val.data(), data + 8, size * sizeof(double));
        }
        if ((data[0] & endianMask) != littleEndianCode) {
            for (auto& v : val) {
                checks::swapBytes<8>(reinterpret_cast<std::byte*>(&v));
            }
        }
    }

    void convertFromBinary(const std::byte* data, double* val)
    {
        std::size_t size = getDataSize(data);
        if (val != nullptr && size > 0) {
            std::memcpy(val, data + 8, size * sizeof(double));
        }
        if ((data[0] & endianMask) != littleEndianCode) {
            double* v = val;
            double* end = val + size;
            while (v != end) {
                checks::swapBytes<8>(reinterpret_cast<std::byte*>(v++));
            }
        }
    }

    void convertFromBinary(const std::byte* data, std::vector<std::complex<double>>& val)
    {
        std::size_t size = getDataSize(data);
        val.resize(size);
        if (size > 0) {
            std::memcpy(val.data(), data + 8, size * sizeof(std::complex<double>));
        }
        if ((data[0] & endianMask) != littleEndianCode) {
            for (auto& v : val) {
                // making use of array oriented access for complex numbers
                // See https://en.cppreference.com/w/cpp/numeric/complex
                checks::swapBytes<8>(reinterpret_cast<std::byte*>(&v));
                checks::swapBytes<8>(reinterpret_cast<std::byte*>(&v) + sizeof(double));
            }
        }
    }
}  // namespace detail

void ValueConverter<std::vector<std::string>>::convert(const std::vector<std::string>& val,
                                                       SmallBuffer& store)
{
    Json::Value V(Json::arrayValue);
    for (auto& str : val) {
        V.append(str);
    }
    auto strgen = generateJsonString(V);
    store.resize(detail::getBinaryLength(strgen));
    detail::convertToBinary(store.data(), strgen);
}

/** interpret a view of the data block and store to the specified value*/
void ValueConverter<std::vector<std::string>>::interpret(const data_view& block,
                                                         std::vector<std::string>& val)
{
    val.clear();
    std::string str;
    detail::convertFromBinary(block.bytes(), str);
    Json::Value V = loadJsonStr(str);
    if (V.isArray()) {
        val.reserve(V.size());
        for (auto& av : V) {
            val.emplace_back(av.asString());
        }
    } else {
        val.push_back(block.string());
    }
}

}  // namespace helics
