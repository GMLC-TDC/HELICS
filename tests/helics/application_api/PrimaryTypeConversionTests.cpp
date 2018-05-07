/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <complex>
#include <list>
#include <set>

/** these test cases test out the value converters
*/
#include "helics/application_api/HelicsPrimaryTypes.hpp"

using namespace std::string_literals;
using namespace helics;
BOOST_AUTO_TEST_SUITE(type_conversion_tests)

template <class T1, class T2>
void checkTypeConversion1(const T1 &val1, const T2 &exp)
{
    defV val = val1;
    T2 v2;
    valueExtract(val, v2);
    BOOST_CHECK_MESSAGE(v2 == exp, std::string(typeid(T1).name()) + " -> " + typeid(T2).name());

}

BOOST_AUTO_TEST_CASE(string_type_tests)
{
    BOOST_CHECK(helicsType<std::string>() == helics_type_t::helicsString);
    //BOOST_CHECK(helicsType<char *>() == helics_type_t::helicsString);
}

BOOST_AUTO_TEST_CASE(string_converstion_tests)
{
    BOOST_CHECK(helicsType<std::string>() == helics_type_t::helicsString);
    //BOOST_CHECK(helicsType<char *>() == helics_type_t::helicsString);
    std::string vstr("45.786");
    double val = 45.786;
    checkTypeConversion1(vstr, vstr);
    checkTypeConversion1(vstr, val);
    checkTypeConversion1(vstr, static_cast<int64_t>(val));
    checkTypeConversion1(vstr, static_cast<float>(val));
    checkTypeConversion1(vstr, std::complex<double>(val, 0));
    checkTypeConversion1(vstr, std::vector<double>{val});
    checkTypeConversion1(vstr, std::vector<std::complex<double>>{std::complex<double>(val, 0.0)});
    checkTypeConversion1(vstr, true);
    checkTypeConversion1(vstr, named_point{ "value",val });
    std::string test1("test1");
    checkTypeConversion1(test1, named_point{ test1,std::nan("0") });
}



BOOST_AUTO_TEST_CASE(double_type_tests)
{
    BOOST_CHECK(helicsType<double>() == helics_type_t::helicsDouble);
    BOOST_CHECK(helicsType<float>() == helics_type_t::helicsInvalid);
    BOOST_CHECK(isConvertableType<float>() == true);
    BOOST_CHECK(isConvertableType<double>() == false);
}

BOOST_AUTO_TEST_CASE(double_conversion_tests)
{
    double val = 45.786;
    checkTypeConversion1(val, val);
    checkTypeConversion1(val, std::to_string(val));
    checkTypeConversion1(val, static_cast<int64_t>(val));
    checkTypeConversion1(val, static_cast<float>(val));
    checkTypeConversion1(val, std::complex<double>(val,0));
    checkTypeConversion1(val, std::vector<double>{val});
    checkTypeConversion1(val, std::vector<std::complex<double>>{std::complex<double>(val, 0.0)});
    checkTypeConversion1(val, true);
    checkTypeConversion1(val, named_point{ "value",val });
}

BOOST_AUTO_TEST_CASE(integer_type_tests)
{
    BOOST_CHECK(helicsType<int64_t>() == helics_type_t::helicsInt);
    BOOST_CHECK(helicsType<int>() == helics_type_t::helicsInvalid);
    BOOST_CHECK(isConvertableType<int>() == true);
    BOOST_CHECK(isConvertableType<int64_t>() == false);

    BOOST_CHECK(isConvertableType<short>() == true);
    BOOST_CHECK(isConvertableType<uint64_t>() == true);

    int64_t val = -10;
    checkTypeConversion1(val, val);
    checkTypeConversion1(val, std::to_string(val));
    checkTypeConversion1(val, static_cast<double>(val));
    checkTypeConversion1(val, static_cast<float>(val));
    checkTypeConversion1(val, static_cast<short>(val));
    checkTypeConversion1(val, static_cast<int>(val));
    checkTypeConversion1(val, std::complex<double>(val, 0));
    checkTypeConversion1(val, std::vector<double>{static_cast<double>(val)});
    checkTypeConversion1(val, std::vector<std::complex<double>>{std::complex<double>(val, 0.0)});
    checkTypeConversion1(val, true);
    checkTypeConversion1(static_cast<int64_t>(0), false);
    checkTypeConversion1(val, named_point{ "value",static_cast<double>(val) });
}

BOOST_AUTO_TEST_SUITE_END()