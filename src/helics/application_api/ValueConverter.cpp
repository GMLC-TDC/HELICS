/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "ValueConverter.hpp"

#include "ValueConverter_impl.hpp"

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

    size_t getBinaryLength(double /*val*/) { return 9; }
    size_t getBinaryLength(std::int64_t /*val*/) { return 9; }
    size_t getBinaryLength(std::complex<double> /*val*/) { return 17; }
    size_t getBinaryLength(std::string_view val) { return val.size() + 4; }
    size_t getBinaryLength(const std::vector<double>& val)
    {
        return val.size() * sizeof(double) + 8;
    }
    size_t getBinaryLength(const double* /*val*/, size_t size) { return size * sizeof(double) + 8; }

    size_t getBinaryLength(const NamedPoint& np) { return np.name.size() + 12; }

    size_t getBinaryLength(const std::vector<std::complex<double>>& cv)
    {
        return cv.size() * sizeof(double) * 2 + 8;
    }

    size_t convertToBinary(std::byte* data, double val)
    {
        std::memcpy(data, &val, 8);
        data[8] = doubleCode | littleEndianCode;
        return 9;
    }

    size_t convertToBinary(std::byte* data, std::int64_t val)
    {
        std::memcpy(data, &val, 8);
        data[8] = intCode | littleEndianCode;
        return 9;
    }

    size_t convertToBinary(std::byte* data, const std::complex<double>& val)
    {
        std::memcpy(data, &val, 16);
        data[16] = complexCode | littleEndianCode;
        return 17;
    }

    size_t convertToBinary(std::byte* data, std::string_view val)
    {
        data[0] = stringCode | littleEndianCode;

        data[1] = std::byte((val.size() >> 16U) & 0xFFU);
        data[2] = std::byte((val.size() >> 8U) & 0xFFU);
        data[3] = std::byte((val.size() & 0xFFU));
        std::memcpy(data + 4U, val.data(), val.size());
        return val.size() + 4U;
    }

    size_t convertToBinary(std::byte* data, const NamedPoint& val)
    {
        std::memcpy(data, &val.value, 8);
        data[8] = npCode | littleEndianCode;

        data[9] = std::byte((val.name.size() >> 16U) & 0XFFU);
        data[10] = std::byte((val.name.size() >> 8U) & 0XFFU);
        data[11] = std::byte((val.name.size() & 0XFFU));
        std::memcpy(data + 12, val.name.data(), val.name.size());
        return val.name.size() + 12U;
    }

    size_t convertToBinary(std::byte* data, const std::vector<double>& val)
    {
        data[0] = vectorCode | littleEndianCode;

        data[1] = std::byte((val.size() >> 16U) & 0XFFU);
        data[2] = std::byte((val.size() >> 8U) & 0XFFU);
        data[3] = std::byte((val.size() & 0XFFU));
        std::memset(data + 4, 0, 4);
        std::memcpy(data + 8, val.data(), val.size() * sizeof(double));
        return val.size() * sizeof(double) + 8U;
    }

    size_t convertToBinary(std::byte* data, const double* val, size_t size)
    {
        data[0] = vectorCode | littleEndianCode;

        data[1] = std::byte((size >> 16U) & 0XFFU);
        data[2] = std::byte((size >> 8U) & 0XFFU);
        data[3] = std::byte((size & 0XFFU));
        std::memset(data + 4, 0, 4);
        std::memcpy(data + 8, val, size * sizeof(double));
        return size * sizeof(double) + 8U;
    }

    size_t convertToBinary(std::byte* data, const std::vector<std::complex<double>>& val)
    {
        data[0] = cvCode | littleEndianCode;

        data[1] = std::byte((val.size() >> 16U) & 0XFFU);
        data[2] = std::byte((val.size() >> 8U) & 0XFFU);
        data[3] = std::byte((val.size() & 0XFFU));
        std::memset(data + 4, 0U, 4U);
        std::memcpy(data + 8, val.data(), val.size() * sizeof(double) * 2);
        return val.size() * sizeof(double) * 2U + 8U;
    }

    size_t getDataSize(const std::byte* data, data_type type)
    {
        switch (type) {
            default:
                return 1;
            case data_type::helics_complex:
                return 2;
            case data_type::helics_string:
            case data_type::helics_vector:
            case data_type::helics_complex_vector:
            case data_type::helics_custom:
                return (static_cast<size_t>(data[1]) << 16U) +
                    (static_cast<size_t>(data[2]) << 8U) + static_cast<size_t>(data[3]);
            case data_type::helics_named_point:
                return (static_cast<size_t>(data[9]) << 16U) +
                    (static_cast<size_t>(data[10]) << 8U) + static_cast<size_t>(data[11]);
        }
    }

    data_type detectType(const std::byte* data, size_t size)
    {
        switch (size) {
            case 9:
                if ((data[8] & codeMask) == doubleCode) {
                    return data_type::helics_double;
                }
                if ((data[8] & codeMask) == intCode) {
                    return data_type::helics_int;
                }
                break;
            case 17:
                if ((data[16] & codeMask) == complexCode) {
                    return data_type::helics_complex;
                }
                break;
            default:
                break;
        }
        switch (data[0] & codeMask) {
            case vectorCode:
                return data_type::helics_vector;
            case cvCode:
                return data_type::helics_complex_vector;
            case stringCode:
                return data_type::helics_string;
            default:
                break;
        }
        if (size >= 9 && (data[8] & codeMask) == npCode) {
            return data_type::helics_named_point;
        }
        return data_type::helics_custom;
    }

    void convertFromBinary(const std::byte* data, double& val)
    {
        std::memcpy(&val, data, 8);
        if ((data[8] & endianMask) != littleEndianCode) {
            checks::swapBytes<8>(reinterpret_cast<std::byte*>(&val));
        }
    }

    void convertFromBinary(const std::byte* data, std::int64_t& val)
    {
        std::memcpy(&val, data, 8);
        if ((data[8] & endianMask) != littleEndianCode) {
            checks::swapBytes<8>(reinterpret_cast<std::byte*>(&val));
        }
    }

    void convertFromBinary(const std::byte* data, std::complex<double>& val)
    {
        std::memcpy(&val, data, 16);
        if ((data[16] & endianMask) != littleEndianCode) {
            checks::swapBytes<8>(reinterpret_cast<std::byte*>(&val));
            checks::swapBytes<8>(reinterpret_cast<std::byte*>(&val) + 8);
        }
    }

    void convertFromBinary(const std::byte* data, std::string& val)
    {
        std::size_t size = (std::to_integer<std::size_t>(data[1]) << 16U) +
            (std::to_integer<std::size_t>(data[2]) << 8U) + std::to_integer<std::size_t>(data[3]);
        val.assign(reinterpret_cast<const char*>(data) + 4,
                   reinterpret_cast<const char*>(data) + 4 + size);
    }

    void convertFromBinary(const std::byte* data, char* val)
    {
        std::size_t size = (std::to_integer<std::size_t>(data[1]) << 16U) +
            (std::to_integer<std::size_t>(data[2]) << 8U) + std::to_integer<std::size_t>(data[3]);
        memcpy(val, data + 4, size);
    }

    void convertFromBinary(const std::byte* data, NamedPoint& val)
    {
        std::memcpy(&val.value, data, 8);
        std::size_t size = (std::to_integer<std::size_t>(data[9]) << 16U) +
            (std::to_integer<std::size_t>(data[10]) << 8U) + std::to_integer<std::size_t>(data[11]);
        val.name.assign(reinterpret_cast<const char*>(data) + 12U,
                        reinterpret_cast<const char*>(data) + 12U + size);
        if ((data[8] & endianMask) != littleEndianCode) {
            checks::swapBytes<8>(reinterpret_cast<std::byte*>(&val.value));
        }
    }

    void convertFromBinary(const std::byte* data, std::vector<double>& val)
    {
        std::size_t size = (std::to_integer<std::size_t>(data[1]) << 16U) +
            (std::to_integer<std::size_t>(data[2]) << 8U) + std::to_integer<std::size_t>(data[3]);
        val.resize(size);
        std::memcpy(val.data(), data + 8, size * sizeof(double));
        if ((data[0] & endianMask) != littleEndianCode) {
            for (auto& v : val) {
                checks::swapBytes<8>(reinterpret_cast<std::byte*>(&v));
            }
        }
    }

    void convertFromBinary(const std::byte* data, double* val)
    {
        std::size_t size = (std::to_integer<std::size_t>(data[1]) << 16U) +
            (std::to_integer<std::size_t>(data[2]) << 8U) + std::to_integer<std::size_t>(data[3]);
        std::memcpy(val, data + 8, size * sizeof(double));
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
        std::size_t size = (std::to_integer<std::size_t>(data[1]) << 16U) +
            (std::to_integer<std::size_t>(data[2]) << 8U) + std::to_integer<std::size_t>(data[3]);
        val.resize(size);
        std::memcpy(val.data(), data + 8, size * sizeof(std::complex<double>));
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

}  // namespace helics
