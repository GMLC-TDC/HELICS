/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

#include "ValueConverter.hpp"
#include "ValueConverter_impl.hpp"

namespace helics
{
template class ValueConverter<int64_t>;
template class ValueConverter<uint64_t>;
template class ValueConverter<char>;
template class ValueConverter<std::complex<double>>;
template class ValueConverter<double>;
template class ValueConverter<float>;
template class ValueConverter<int16_t>;
template class ValueConverter<uint32_t>;
template class ValueConverter<int32_t>;
template class ValueConverter<std::vector<double>>;
template class ValueConverter<std::vector<std::string>>;
template class ValueConverter<std::vector<std::complex<double>>>;
template class ValueConverter<named_point>;
}  // namespace helics

