/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ValueConverter.hpp"

#include "ValueConverter_impl.hpp"
#include "../common/JsonProcessingFunctions.hpp"

#include <complex>
#include <vector>

namespace helics {
template class ValueConverter<int64_t>;
template class ValueConverter<uint64_t>;
template class ValueConverter<char>;
template class ValueConverter<bool>;
template class ValueConverter<std::complex<double>>;
template class ValueConverter<double>;
template class ValueConverter<float>;
template class ValueConverter<int16_t>;
template class ValueConverter<uint32_t>;
template class ValueConverter<int32_t>;
template class ValueConverter<std::vector<double>>;
template class ValueConverter<std::vector<std::string>>;
template class ValueConverter<std::vector<std::complex<double>>>;
template class ValueConverter<NamedPoint>;
}  // namespace helics


namespace helics {

namespace detail {
    namespace checks {
        /*! This code in checks namespace exerpted from cereal portable binary archives
        code is modified slightly to fit name conventions and make use of C++17

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
        inline void swapBytes(unsigned char* data)
        {
            for (std::size_t i = 0, end = DataSize / 2; i < end; ++i) {
                std::swap(data[i], data[DataSize - i - 1]);
            }
        }
    }  // namespace checks
    static constexpr const unsigned char doubleCode{0xB0};
    static constexpr const unsigned char intCode{0x50};
    static constexpr const unsigned char complexCode{0x12};
    static constexpr const unsigned char stringCode{0x0E};
    static constexpr const unsigned char vectorCode{0x6C};
    static constexpr const unsigned char npCode{0xAE};
    static constexpr const unsigned char cvCode{0x62};
    static constexpr const unsigned char customCode{0xF4};

    static constexpr const unsigned char endianMask{0x01};
    // static constexpr unsigned char lowByteMask{0xFF};
    // static constexpr unsigned char codeMask{0xFE};

    static const unsigned char endianCode = checks::isLittleEndian() ? 0 : 1;

    static constexpr const unsigned char littleEndianCode{0x0};
    // static constexpr const unsigned char bigEndianCode{0x01};


    size_t getDataSize3(const unsigned char* data)
    {
        return (static_cast<size_t>(data[4]) << 24U) +
            (static_cast<size_t>(data[5]) << 16U) + (static_cast<size_t>(data[6]) << 8U) +
            static_cast<size_t>(data[7]);
    }

    data_type detectType3(const unsigned char* data)
    {
        switch (data[0] & 0xFEU)
        {
            case intCode:
                return data_type::helics_int;
            case doubleCode:
                return data_type::helics_double;
            case complexCode:
                return data_type::helics_complex;
            case vectorCode:
                return data_type::helics_vector;
            case cvCode:
                return data_type::helics_complex_vector;
            case npCode:
                return data_type::helics_named_point;
            case customCode:
                return data_type::helics_custom;
            case stringCode:
                return data_type::helics_string;
            default:
                return data_type::helics_unknown;
        }
    }

    void convertFromBinary3(const unsigned char* data, double& val)
    {
        std::memcpy(&val, data + 8, 8);
        if ((data[0] & endianMask) != littleEndianCode) {
            checks::swapBytes<8>(reinterpret_cast<unsigned char*>(&val));
        }
    }

    void convertFromBinary3(const unsigned char* data, std::int64_t& val)
    {
        std::memcpy(&val, data + 8, 8);
        if ((data[0] & endianMask) != littleEndianCode) {
            checks::swapBytes<8>(reinterpret_cast<unsigned char*>(&val));
        }
    }

    void convertFromBinary3(const unsigned char* data, std::complex<double>& val)
    {
        // https://en.cppreference.com/w/cpp/numeric/complex
        //  the layout used here is guaranteed by the standard
        std::memcpy(&reinterpret_cast<double(&)[2]>(val)[0], data + 8, 8);
        std::memcpy(&reinterpret_cast<double(&)[2]>(val)[1], data + 16, 8);
        if ((data[0] & endianMask) != littleEndianCode) {
            checks::swapBytes<8>(reinterpret_cast<unsigned char*>(&val));
            checks::swapBytes<8>(reinterpret_cast<unsigned char*>(&val) + 8);
        }
    }

    void convertFromBinary3(const unsigned char* data, std::string& val)
    {
        std::size_t size = getDataSize3(data);
        val.assign(reinterpret_cast<const char*>(data) + 8,
                   reinterpret_cast<const char*>(data) + 8 + size);
    }

    void convertFromBinary3(const unsigned char* data, char* val)
    {
        std::size_t size = getDataSize3(data);
        memcpy(val, data + 8, size);
    }

    void convertFromBinary3(const unsigned char* data, NamedPoint& val)
    {
        std::memcpy(&val.value, data + 8, 8);
        std::size_t size = getDataSize3(data);
        val.name.assign(reinterpret_cast<const char*>(data) + 16U,
                        reinterpret_cast<const char*>(data) + 16U + size);
        if ((data[0] & endianMask) != littleEndianCode) {
            checks::swapBytes<8>(reinterpret_cast<unsigned char*>(&val.value));
        }
    }

    void convertFromBinary3(const unsigned char* data, std::vector<double>& val)
    {
        std::size_t size = getDataSize3(data);
        val.resize(size);
        if (size > 0) {
            std::memcpy(val.data(), data + 8, size * sizeof(double));
        }
        if ((data[0] & endianMask) != littleEndianCode) {
            for (auto& v : val) {
                checks::swapBytes<8>(reinterpret_cast<unsigned char*>(&v));
            }
        }
    }

    void convertFromBinary3(const unsigned char* data, double* val)
    {
        std::size_t size = getDataSize3(data);
        if (val != nullptr && size > 0) {
            std::memcpy(val, data + 8, size * sizeof(double));
        }
        if ((data[0] & endianMask) != littleEndianCode) {
            double* v = val;
            double* end = val + size;
            while (v != end) {
                checks::swapBytes<8>(reinterpret_cast<unsigned char*>(v++));
            }
        }
    }

    void convertFromBinary3(const unsigned char* data, std::vector<std::complex<double>>& val)
    {
        std::size_t size = getDataSize3(data);
        val.resize(size);
        if (size > 0) {
            std::memcpy(reinterpret_cast<double*>(val.data()),
                        data + 8,
                        size * sizeof(std::complex<double>));
        }
        if ((data[0] & endianMask) != littleEndianCode) {
            for (auto& v : val) {
                // making use of array oriented access for complex numbers
                // See https://en.cppreference.com/w/cpp/numeric/complex
                checks::swapBytes<8>(reinterpret_cast<unsigned char*>(&v));
                checks::swapBytes<8>(reinterpret_cast<unsigned char*>(&v) + sizeof(double));
            }
        }
    }
}  // namespace detail


/** interpret a view of the data block and store to the specified value*/
template<>
void ValueConverter3<std::vector<std::string>>::interpret(const data_view& block,
                                                         std::vector<std::string>& val)
{
    val.clear();
    auto str = ValueConverter3<std::string>::interpret(block);
    try {
        Json::Value V = loadJsonStr(str);
        if (V.isArray()) {
            val.reserve(V.size());
            for (auto& av : V) {
                val.emplace_back(av.asString());
            }
        } else {
            val.emplace_back(str);
        }
    }
    catch (...) {
        val.emplace_back(str);
    }
}

}  // namespace helics
