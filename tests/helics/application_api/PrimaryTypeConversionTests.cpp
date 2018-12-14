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

namespace utf = boost::unit_test;

using namespace std::string_literals;
using namespace helics;
BOOST_AUTO_TEST_SUITE (type_conversion_tests, *utf::label ("ci"))

template <class T1, class T2>
bool checkTypeConversion1 (const T1 &val1, const T2 &exp)
{
    defV val = val1;
    T2 v2;
    valueExtract (val, v2);
    if (v2 != exp)
    {
        return false;
    }
    return true;
}

BOOST_AUTO_TEST_CASE (vectorNorm_tests)
{
    using c = std::complex<double>;
    using cv = std::vector<c>;
    BOOST_CHECK_EQUAL (vectorNorm (std::vector<double> ()), 0.0);
    BOOST_CHECK_EQUAL (vectorNorm (std::vector<double>{4.0}), 4.0);
    BOOST_CHECK_EQUAL (vectorNorm (std::vector<double>{3.0, 4.0}), 5.0);
    BOOST_CHECK_EQUAL (vectorNorm (std::vector<double>{-3.0, -4.0}), 5.0);
    BOOST_CHECK_EQUAL (vectorNorm (std::vector<double>{-3.0, -3.0, -3.0, -3.0, -3.0}), std::sqrt (9.0 * 5.0));

    BOOST_CHECK_EQUAL (vectorNorm (cv ()), 0.0);
    BOOST_CHECK_EQUAL (vectorNorm (cv{c (4.0, 0)}), 4.0);
    BOOST_CHECK_EQUAL (vectorNorm (cv{c (3.0, 4.0)}), 5.0);
    BOOST_CHECK_EQUAL (vectorNorm (cv{c (-3.0, -4.0)}), 5.0);
    BOOST_CHECK_EQUAL (vectorNorm (
                         cv{c (-3.0, -4.0), c (-3.0, -4.0), c (-3.0, -4.0), c (-3.0, -4.0), c (-3.0, -4.0)}),
                       std::sqrt (25.0 * 5.0));
}

BOOST_AUTO_TEST_CASE (string_type_tests)
{
    BOOST_CHECK (helicsType<std::string> () == data_type::helicsString);
    // BOOST_CHECK(helicsType<char *>() == data_type::helicsString);
}

BOOST_AUTO_TEST_CASE (string_converstion_tests)
{
    std::string vstr ("45.786");
    double val = 45.786;
    BOOST_CHECK (checkTypeConversion1 (vstr, vstr));
    BOOST_CHECK (checkTypeConversion1 (vstr, val));
    BOOST_CHECK (checkTypeConversion1 (vstr, static_cast<int64_t> (val)));
    BOOST_CHECK (checkTypeConversion1 (vstr, static_cast<float> (val)));
    BOOST_CHECK (checkTypeConversion1 (vstr, std::complex<double> (val, 0)));
    BOOST_CHECK (checkTypeConversion1 (vstr, std::vector<double>{val}));
    BOOST_CHECK (checkTypeConversion1 (vstr, std::vector<std::complex<double>>{std::complex<double> (val, 0.0)}));
    BOOST_CHECK (checkTypeConversion1 (vstr, true));
    BOOST_CHECK (checkTypeConversion1 (vstr, named_point{"value", val}));
    std::string test1 ("test1");
    BOOST_CHECK (checkTypeConversion1 (test1, named_point{test1, std::nan ("0")}));
}

BOOST_AUTO_TEST_CASE (double_type_tests)
{
    BOOST_CHECK (helicsType<double> () == data_type::helicsDouble);
    BOOST_CHECK (helicsType<float> () == data_type::helicsCustom);
    BOOST_CHECK (isConvertableType<float> () == true);
    BOOST_CHECK (isConvertableType<double> () == false);
}

BOOST_AUTO_TEST_CASE (double_conversion_tests)
{
    double val = 45.786;
    BOOST_CHECK (checkTypeConversion1 (val, val));
    BOOST_CHECK (checkTypeConversion1 (val, std::to_string (val)));
    BOOST_CHECK (checkTypeConversion1 (val, static_cast<int64_t> (val)));
    BOOST_CHECK (checkTypeConversion1 (val, static_cast<float> (val)));
    BOOST_CHECK (checkTypeConversion1 (val, std::complex<double> (val, 0)));
    BOOST_CHECK (checkTypeConversion1 (val, std::vector<double>{val}));
    BOOST_CHECK (checkTypeConversion1 (val, std::vector<std::complex<double>>{std::complex<double> (val, 0.0)}));
    BOOST_CHECK (checkTypeConversion1 (val, true));
    BOOST_CHECK (checkTypeConversion1 (val, named_point{"value", val}));
}

BOOST_AUTO_TEST_CASE (integer_type_tests)
{
    BOOST_CHECK (helicsType<int64_t> () == data_type::helicsInt);
    BOOST_CHECK (helicsType<int> () == data_type::helicsCustom);
    BOOST_CHECK (isConvertableType<int> () == true);
    BOOST_CHECK (isConvertableType<int64_t> () == false);

    BOOST_CHECK (isConvertableType<short> () == true);
    BOOST_CHECK (isConvertableType<uint64_t> () == true);

    BOOST_CHECK (isConvertableType<char> () == true);
    BOOST_CHECK (isConvertableType<unsigned char> () == true);

    BOOST_CHECK (isConvertableType<unsigned char> () == true);
}

BOOST_AUTO_TEST_CASE (namedType_tests)
{
    BOOST_CHECK (getTypeFromString ("int") == data_type::helicsInt);
    BOOST_CHECK (getTypeFromString ("INT") == data_type::helicsInt);
    BOOST_CHECK (getTypeFromString ("char") == data_type::helicsString);
    BOOST_CHECK (getTypeFromString (typeid(int).name()) == data_type::helicsInt);
    BOOST_CHECK (getTypeFromString (typeid(float).name()) == data_type::helicsDouble);
    BOOST_CHECK (getTypeFromString (typeid(std::string).name()) == data_type::helicsString);
    BOOST_CHECK (getTypeFromString (typeid (char *).name ()) == data_type::helicsString);
    BOOST_CHECK (getTypeFromString (typeid (const char *).name ()) == data_type::helicsString);
    BOOST_CHECK (getTypeFromString (typeid (double).name ()) == data_type::helicsDouble);
    BOOST_CHECK (getTypeFromString (typeid (bool).name ()) == data_type::helicsBool);
    BOOST_CHECK (getTypeFromString (typeid (int64_t).name ()) == data_type::helicsInt);
    BOOST_CHECK (getTypeFromString (typeid (char).name ()) == data_type::helicsString);
    BOOST_CHECK (getTypeFromString (typeid (std::complex<double>).name ()) == data_type::helicsComplex);
    BOOST_CHECK (getTypeFromString ("COMPLEX") == data_type::helicsComplex);
    BOOST_CHECK (getTypeFromString ("map") == data_type::helicsCustom);
    BOOST_CHECK (getTypeFromString ("any") == data_type::helicsAny);
    BOOST_CHECK (getTypeFromString ("") == data_type::helicsAny);
    BOOST_CHECK (getTypeFromString (typeid (std::vector<std::complex<double>>).name ()) == data_type::helicsComplexVector);
    BOOST_CHECK (getTypeFromString (typeid (Time).name ()) == data_type::helicsTime);
}

BOOST_AUTO_TEST_CASE (integer_conversion_tests)
{
    int64_t val = -10;
    BOOST_CHECK (checkTypeConversion1 (val, val));
    BOOST_CHECK (checkTypeConversion1 (val, std::to_string (val)));
    BOOST_CHECK (checkTypeConversion1 (val, static_cast<double> (val)));
    BOOST_CHECK (checkTypeConversion1 (val, static_cast<float> (val)));
    BOOST_CHECK (checkTypeConversion1 (val, static_cast<short> (val)));
    BOOST_CHECK (checkTypeConversion1 (val, static_cast<int> (val)));
    BOOST_CHECK (checkTypeConversion1 (val, std::complex<double> (val, 0)));
    BOOST_CHECK (checkTypeConversion1 (val, std::vector<double>{static_cast<double> (val)}));
    BOOST_CHECK (checkTypeConversion1 (val, std::vector<std::complex<double>>{std::complex<double> (val, 0.0)}));
    BOOST_CHECK (checkTypeConversion1 (val, true));
    BOOST_CHECK (checkTypeConversion1 (static_cast<int64_t> (0), false));
    BOOST_CHECK (checkTypeConversion1 (val, named_point{"value", static_cast<double> (val)}));
}

BOOST_AUTO_TEST_CASE (namedpoint_type_tests)
{
    BOOST_CHECK (helicsType<named_point> () == data_type::helicsNamedPoint);
    // BOOST_CHECK(helicsType<char *>() == data_type::helicsString);
}

BOOST_AUTO_TEST_CASE (namedpoint_conversion_tests)
{
    double val = 45.786;
    named_point vp{"point", val};
    BOOST_CHECK (checkTypeConversion1 (vp, vp));
    BOOST_CHECK (checkTypeConversion1 (vp, val));
    BOOST_CHECK (checkTypeConversion1 (vp, static_cast<int64_t> (val)));
    BOOST_CHECK (checkTypeConversion1 (vp, static_cast<float> (val)));
    BOOST_CHECK (checkTypeConversion1 (vp, std::complex<double> (val, 0)));
    BOOST_CHECK (checkTypeConversion1 (vp, std::vector<double>{val}));
    BOOST_CHECK (checkTypeConversion1 (vp, std::vector<std::complex<double>>{std::complex<double> (val, 0.0)}));
    BOOST_CHECK (checkTypeConversion1 (vp, true));
    BOOST_CHECK (checkTypeConversion1 (vp, std::string ("{\"point\":" + std::to_string (val) + "}")));

    named_point vp2{"v2[3.0,-4.0]", std::nan ("0")};
    double v2 = 5.0;
    BOOST_CHECK (checkTypeConversion1 (vp2, vp2));
    BOOST_CHECK (checkTypeConversion1 (vp2, v2));
    BOOST_CHECK (checkTypeConversion1 (vp2, static_cast<int64_t> (v2)));
    BOOST_CHECK (checkTypeConversion1 (vp2, static_cast<float> (v2)));
    BOOST_CHECK (checkTypeConversion1 (vp2, std::complex<double> (3.0, -4.0)));
    BOOST_CHECK (checkTypeConversion1 (vp2, std::vector<double>{3.0, -4.0}));
    BOOST_CHECK (checkTypeConversion1 (vp2, std::vector<std::complex<double>>{std::complex<double> (3.0, -4.0)}));
    BOOST_CHECK (checkTypeConversion1 (vp2, true));
    BOOST_CHECK (checkTypeConversion1 (vp2, vp2.name));

    named_point t1 ("this is a longer string for testing purposes", 234.252622334);
    auto s = helicsNamedPointString (t1);
    auto t2 = helicsGetNamedPoint (s);
    BOOST_CHECK_EQUAL (t1.name, t2.name);
    BOOST_CHECK_CLOSE (t1.value, t2.value, 0.00001);
}
BOOST_AUTO_TEST_SUITE_END ()
