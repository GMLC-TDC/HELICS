/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
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

