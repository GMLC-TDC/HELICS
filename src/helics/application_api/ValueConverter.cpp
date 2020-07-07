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
        inline std::uint8_t isLittleEndian()
        {
            static constexpr std::int32_t test{1};
            return *reinterpret_cast<const std::int8_t*>(&test) == 1;
        }

        //! Swaps the order of bytes for some chunk of memory
        /*! @param data The data as a uint8_t pointer
            @tparam DataSize The true size of the data
            @ingroup Internal */
        template<std::size_t DataSize>
        inline void swapBytes(std::uint8_t* data)
        {
            for (std::size_t i = 0, end = DataSize / 2; i < end; ++i)
                std::swap(data[i], data[DataSize - i - 1]);
        }
    }  // namespace checks
    static constexpr std::uint8_t doubleCode = 0xB0;
    static constexpr std::uint8_t intCode = 0x50;
    static constexpr std::uint8_t complexCode = 0x12;
    static constexpr std::uint8_t stringCode = 0x0E;
    static constexpr std::uint8_t vectorCode = 0x6C;
    static constexpr std::uint8_t npCode = 0xAE;
    static constexpr std::uint8_t cvCode = 0x62;
    static constexpr std::uint8_t customCode = 0xF4;

    static const std::uint8_t littleEndianCode = checks::isLittleEndian() ? 0 : 1;

    size_t getBinaryLength(double /*val*/) { return 9;}
    size_t getBinaryLength(std::int64_t /*val*/) { return 9; }
    size_t getBinaryLength(std::complex<double> /*val*/) { return 17; }

    size_t convertToBinary(std::uint8_t* data, double val)
    {
        std::memcpy(data, &val, 8);
        data[8] = doubleCode;
        return 9;
    }

    size_t convertToBinary(std::uint8_t* data, std::int64_t val)
    {
        std::memcpy(data, &val, 8);
        data[8] = intCode;
        return 9;
    }

    size_t converToBinary(std::uint8_t* data, const std::complex<double>& val)
    {
        std::memcpy(data, &val, 16);
        data[16] = complexCode;
        return 17;
    }
}

}  // namespace helics
