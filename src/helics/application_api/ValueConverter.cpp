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
    size_t getBinaryLength(double val) { return 9; }
    static constexpr unsigned char doubleCode = 0xB0;

    static constexpr unsigned char intCode = 0x50;
    static constexpr unsigned char complexCode = 0x12;
    static constexpr unsigned char stringCode = 0x0E;
    static constexpr unsigned char vectorCode = 0x6C;
    static constexpr unsigned char npCode = 0xAE;
    static constexpr unsigned char cvCode = 0x62;
    static constexpr unsigned char customCode = 0xF4;

    void convertToBinary(char* data, double val)
    {
        std::memcpy(data, &val, 8);
        data[8] = doubleCode;
    }
}  // namespace detail

}  // namespace helics
