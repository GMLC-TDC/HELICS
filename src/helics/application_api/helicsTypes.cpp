/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "helicsTypes.hpp"
#include "ValueConverter.hpp"
#include <map>
#include <numeric>
#include <regex>
#include <boost/lexical_cast.hpp>
#include "../common/stringOps.h"

namespace helics
{
static const std::string doubleString ("double");
static const std::string intString ("int64");
static const std::string stringString ("string");
static const std::string complexString ("complex");
static const std::string boolString ("bool");
static const std::string doubleVecString ("double_vector");
static const std::string complexVecString ("complex_vector");
static const std::string namedPointString ("named_point");
static const std::string nullString;

const std::string &typeNameStringRef (helics_type_t type)
{
    switch (type)
    {
    case helics_type_t::helicsDouble:
        return doubleString;
    case helics_type_t::helicsInt:
        return intString;
    case helics_type_t::helicsBool:
        return boolString;
    case helics_type_t::helicsString:
        return stringString;
    case helics_type_t::helicsComplex:
        return complexString;
    case helics_type_t::helicsVector:
        return doubleVecString;
    case helics_type_t::helicsComplexVector:
        return complexVecString;
    case helics_type_t::helicsNamedPoint:
        return namedPointString;
    default:
        return nullString;
    }
}

double vectorNorm (const std::vector<double> &vec)
{
    return std::sqrt (std::inner_product (vec.begin (), vec.end (), vec.begin (), 0.0));
}

double vectorNorm (const std::vector<std::complex<double>> &vec)
{
    return std::sqrt (
      std::inner_product (vec.begin (), vec.end (), vec.begin (), 0.0, std::plus<> (),
                          [](const auto &a, const auto &b) { return (a * std::conj (b)).real (); }));
}

std::string helicsComplexString (double real, double imag)
{
    std::stringstream ss;
    ss << real;
    if (imag != 0.0)
    {
        ss << ((imag >= 0.0) ? '+' : ' ') << imag << 'j';
    }
    return ss.str ();
}

std::string helicsComplexString (std::complex<double> val)
{
    return helicsComplexString (val.real (), val.imag ());
}

static const std::map<std::string, helics_type_t> typeMap{{"double", helics_type_t::helicsDouble},
                                                          {"string", helics_type_t::helicsString},
                                                          {"binary", helics_type_t::helicsBool},
                                                          {"bool", helics_type_t::helicsBool},
                                                          {"boolean", helics_type_t::helicsBool},
                                                          {"flag", helics_type_t::helicsBool},
                                                          {"float", helics_type_t::helicsDouble},
                                                          {"vector", helics_type_t::helicsVector},
                                                          {"double_vector", helics_type_t::helicsVector},
                                                          {"complex", helics_type_t::helicsComplex},
                                                          {"pair", helics_type_t::helicsComplex},
                                                          {"int", helics_type_t::helicsInt},
                                                          {"int64", helics_type_t::helicsInt},
                                                          {"integer", helics_type_t::helicsInt},
                                                          {"complex_vector", helics_type_t::helicsComplexVector},
                                                          {"d", helics_type_t::helicsDouble},
                                                          {"s", helics_type_t::helicsString},
                                                          {"f", helics_type_t::helicsDouble},
                                                          {"v", helics_type_t::helicsVector},
                                                          {"c", helics_type_t::helicsComplex},
                                                          {"i", helics_type_t::helicsInt},
                                                          {"i64", helics_type_t::helicsInt},
                                                          {"cv", helics_type_t::helicsComplexVector},
                                                          {"np", helics_type_t::helicsNamedPoint},
                                                          {"point", helics_type_t::helicsNamedPoint},
                                                          {"pt", helics_type_t::helicsNamedPoint},
                                                          {"named_point", helics_type_t::helicsNamedPoint},
                                                          {"default", helics_type_t::helicsAny},
                                                          {"def", helics_type_t::helicsAny},
                                                          {"any", helics_type_t::helicsAny},
                                                          {"all", helics_type_t::helicsAny}};

helics_type_t getTypeFromString (const std::string &typeName)
{
    auto res = typeMap.find (typeName);
    if (res == typeMap.end ())
    {
        return helics_type_t::helicsInvalid;
    }
    return res->second;
}

// regular expression to handle complex numbers of various formats
const std::regex creg (
  R"(([+-]?(\d+(\.\d+)?|\.\d+)([eE][+-]?\d+)?)\s*([+-]\s*(\d+(\.\d+)?|\.\d+)([eE][+-]?\d+)?)[ji]*)");

std::complex<double> helicsGetComplex (const std::string &val)
{
    if (val.empty ())
    {
        return std::complex<double> (-1e49, -1e49);
    }
    std::smatch m;
    double re = -1e49;
    double im = 0.0;
    std::regex_search (val, m, creg);
    try
    {
        if (m.size () == 9)
        {
            re = boost::lexical_cast<double> (m[1]);

            im = boost::lexical_cast<double> (m[6]);

            if (*m[5].first == '-')
            {
                im = -im;
            }
        }
        else
        {
            if ((val.back () == 'j') || (val.back () == 'i'))
            {
                auto strval = val.substr (0, val.size () - 1);
                stringOps::trimString (strval);
                im = boost::lexical_cast<double> (strval);
                re = 0.0;
            }
            else
            {
                auto strval = val;
                stringOps::trimString(strval);
                re = boost::lexical_cast<double> (strval);
            }
        }
    }
    catch (const boost::bad_lexical_cast &)
    {
        re = -1e49;
    }
    return {re, im};
}

std::string helicsVectorString (const std::vector<double> &val)
{
    std::string vString ("v");
    vString.append (std::to_string (val.size ()));
    vString.push_back ('[');
    for (const auto &v : val)
    {
        vString.append (std::to_string (v));
        vString.push_back (';');
        vString.push_back (' ');
    }
    if (vString.size () > 3)  // 3 for v0[ which would be for an empty vector
    {
        vString.pop_back ();
        vString.pop_back ();
    }
    vString.push_back (']');
    return vString;
}

std::string helicsVectorString (const double *vals, size_t size)
{
    std::string vString ("v");
    vString.append (std::to_string (size));
    vString.push_back ('[');
    for (size_t ii = 0; ii < size; ++ii)
    {
        vString.append (std::to_string (vals[ii]));
        vString.push_back (';');
        vString.push_back (' ');
    }
    if (vString.size () > 3)  // 3 for c0[ which would be for an empty vector
    {
        vString.pop_back ();
        vString.pop_back ();
    }
    vString.push_back (']');
    return vString;
}

std::string helicsComplexVectorString (const std::vector<std::complex<double>> &val)
{
    std::string vString ("c");
    vString.append (std::to_string (val.size ()));
    vString.push_back ('[');
    for (const auto &v : val)
    {
        vString.append (helicsComplexString (v.real (), v.imag ()));
        vString.push_back (';');
        vString.push_back (' ');
    }
    if (vString.size () > 3)
    {
        vString.pop_back ();
        vString.pop_back ();
    }
    vString.push_back (']');
    return vString;
}

std::string helicsNamedPointString (const named_point &point)
{
    return helicsNamedPointString (point.name, point.value);
}
std::string helicsNamedPointString (const std::string &pointName, double val)
{
    std::string retStr = "{\"";
    if (!pointName.empty ())
    {
        retStr.append (pointName);
    }
    else
    {
        retStr.append ("value");
    }
    retStr.push_back ('"');
    retStr.push_back (':');
    retStr.append (std::to_string (val));
    retStr.push_back ('}');
    return retStr;
}

std::string helicsNamedPointString (const char *pointName, double val)
{
    std::string retStr = "{\"";
    if (pointName != nullptr)
    {
        retStr.append (pointName);
    }
    else
    {
        retStr.append ("value");
    }
    retStr.push_back ('"');
    retStr.push_back (':');
    retStr.append (std::to_string (val));
    retStr.push_back ('}');
    return retStr;
}

std::vector<double> helicsGetVector (const std::string &val)
{
    std::vector<double> V;
    helicsGetVector (val, V);
    return V;
}

std::vector<std::complex<double>> helicsGetComplexVector (const std::string &val)
{
    std::vector<std::complex<double>> V;
    helicsGetComplexVector (val, V);
    return V;
}

named_point helicsGetNamedPoint (const std::string &val)
{
    auto loc = val.find_first_of ('{');
    if (loc == std::string::npos)
    {
        auto fb = val.find_first_of ('[');
        if (fb != std::string::npos)
        {
            return {val, std::nan ("0")};
        }
        else
        {
            auto V = helicsGetComplex (val);
            if (V.real () < -1e48)
            {
                return {val, std::nan ("0")};
            }
            if (V.imag () == 0)
            {
                return {"value", std::abs (V)};
            }
            return {val, V.real ()};
        }
    }
    auto locsep = val.find_last_of (':');
    auto locend = val.find_last_of ('}');
    auto str1 = val.substr (loc + 1, locsep - loc);
    stringOps::trimString(str1);
    str1.pop_back ();

    named_point point;
    point.name = str1.substr (1);
    auto vstr = val.substr (locsep + 1, locend - locsep - 1);
    stringOps::trimString(vstr);
    point.value = boost::lexical_cast<double> (vstr);
    return point;
}

static auto readSize (const std::string &val)
{
    auto fb = val.find_first_of ('[');
    auto size = std::stoull (val.substr (1, fb - 1));
    return size;
}

std::complex<double> getComplexFromString (const std::string &val)
{
    if (val.empty ())
    {
        return invalidValue<std::complex<double>> ();
    }
    if ((val.front () == 'v') || (val.front () == 'c'))
    {
        auto V = helicsGetVector (val);
        if (V.empty ())
        {
            return invalidValue<std::complex<double>> ();
        }
        if (V.size () == 1)
        {
            return std::complex<double> (V[0], 0.0);
        }
        return std::complex<double> (V[0], V[1]);
    }
    else if (val.front () == 'c')
    {
        auto cv = helicsGetComplexVector (val);
        if (cv.empty ())
        {
            return invalidValue<std::complex<double>> ();
        }
        return cv.front ();
    }
    return helicsGetComplex (val);
}

double getDoubleFromString (const std::string &val)
{
    if (val.empty ())
    {
        return invalidValue<double> ();
    }
    if ((val.front () == 'v') || (val.front () == 'c'))
    {
        auto V = helicsGetVector (val);
        return vectorNorm (V);
    }
    else if (val.front () == 'c')
    {
        auto cv = helicsGetComplexVector (val);
        return vectorNorm (cv);
    }
    return std::abs (helicsGetComplex (val));
}

void helicsGetVector (const std::string &val, std::vector<double> &data)
{
    if (val.empty ())
    {
        data.resize (0);
    }
    if (val.front () == 'v')
    {
        auto sz = readSize (val);
        data.reserve (sz);
        data.resize (0);
        auto fb = val.find_first_of ('[');
        for (decltype (sz) ii = 0; ii < sz; ++ii)
        {
            auto nc = val.find_first_of (";,]", fb + 1);
            try
            {
                std::string vstr = val.substr (fb + 1, nc - fb - 1);
                stringOps::trimString(vstr);
                auto V = boost::lexical_cast<double> (vstr);
                data.push_back (V);
            }
            catch (const boost::bad_lexical_cast &)
            {
                data.push_back (-1e49);
            }
            fb = nc;
        }
    }
    else if (val.front () == 'c')
    {
        auto sz = readSize (val);
        data.reserve (sz * 2);
        data.resize (0);
        auto fb = val.find_first_of ('[');
        for (decltype (sz) ii = 0; ii < sz; ++ii)
        {
            auto nc = val.find_first_of (",;]", fb + 1);
            auto V = helicsGetComplex (val.substr (fb + 1, nc - fb - 1));
            data.push_back (V.real ());
            data.push_back (V.imag ());
            fb = nc;
        }
    }
    else
    {
        auto V = helicsGetComplex (val);
        if (V.imag () == 0)
        {
            data.resize (1);
            data[0] = V.real ();
        }
        else
        {
            data.resize (2);
            data[0] = V.real ();
            data[1] = V.imag ();
        }
    }
    return;
}

void helicsGetComplexVector (const std::string &val, std::vector<std::complex<double>> &data)
{
    if (val.empty ())
    {
        data.resize (0);
    }
    if (val.front () == 'v')
    {
        auto sz = readSize (val);
        data.reserve (sz / 2);
        data.resize (0);
        auto fb = val.find_first_of ('[');
        for (decltype (sz) ii = 0; ii < sz - 1; ii += 2)
        {
            auto nc = val.find_first_of (",;]", fb + 1);
            auto nc2 = val.find_first_of (",;]", nc + 1);
            try
            {
                std::string vstr1 = val.substr (fb + 1, nc - fb - 1);
                stringOps::trimString(vstr1);
                std::string vstr2 = val.substr (nc + 1, nc2 - nc - 1);
                stringOps::trimString(vstr2);
                auto V1 = boost::lexical_cast<double> (vstr1);
                auto V2 = boost::lexical_cast<double> (vstr2);
                data.emplace_back (V1, V2);
            }
            catch (const boost::bad_lexical_cast &)
            {
                data.push_back (-1e49);
            }
            fb = nc;
        }
    }
    else if (val.front () == 'c')
    {
        auto sz = readSize (val);
        data.reserve (sz);
        data.resize (0);
        auto fb = val.find_first_of ('[');
        for (decltype (sz) ii = 0; ii < sz; ++ii)
        {
            auto nc = val.find_first_of (",;]", fb + 1);
            auto V = helicsGetComplex (val.substr (fb + 1, nc - fb - 1));
            data.push_back (V);
            fb = nc;
        }
    }
    else
    {
        auto V = helicsGetComplex (val);
        data.resize (0);
        data.push_back (V);
    }
    return;
}

data_block emptyBlock (helics_type_t outputType, helics_type_t inputType = helics_type_t::helicsAny)
{
    switch (outputType)
    {
    case helics_type_t::helicsDouble:
    default:
        return ValueConverter<double>::convert (0.0);
    case helics_type_t::helicsInt:
        return ValueConverter<int64_t>::convert (0);
    case helics_type_t::helicsComplex:
        return ValueConverter<std::complex<double>>::convert (std::complex<double> (0.0, 0.0));
    case helics_type_t::helicsBool:
        return "0";
    case helics_type_t::helicsNamedPoint:
        return ValueConverter<named_point>::convert (named_point{"", std::nan ("0")});
    case helics_type_t::helicsString:
        switch (inputType)
        {
        default:
            return std::string ();
        case helics_type_t::helicsVector:
            return helicsVectorString (std::vector<double> ());
        case helics_type_t::helicsComplexVector:
            return helicsComplexVectorString (std::vector<std::complex<double>> ());
        case helics_type_t::helicsNamedPoint:
            return "{\"\":0}";
        }
    case helics_type_t::helicsComplexVector:
    {
        return ValueConverter<std::vector<std::complex<double>>>::convert (std::vector<std::complex<double>> ());
    }
    case helics_type_t::helicsVector:
        return ValueConverter<std::vector<double>>::convert (std::vector<double> ());
    }
}
data_block typeConvert (helics_type_t type, double val)
{
    switch (type)
    {
    case helics_type_t::helicsDouble:
    default:
        return ValueConverter<double>::convert (val);
    case helics_type_t::helicsInt:
        return ValueConverter<int64_t>::convert (static_cast<int64_t> (val));
    case helics_type_t::helicsComplex:
        return ValueConverter<std::complex<double>>::convert (std::complex<double> (val, 0.0));
    case helics_type_t::helicsBool:
        return (val != 0.0) ? "1" : "0";
    case helics_type_t::helicsString:
        return std::to_string (val);
    case helics_type_t::helicsNamedPoint:
        return ValueConverter<named_point>::convert (named_point{"value", val});
    case helics_type_t::helicsComplexVector:
    {
        std::complex<double> v2 (val, 0.0);
        return ValueConverter<std::complex<double>>::convert (&v2, 1);
    }
    case helics_type_t::helicsVector:
        return ValueConverter<double>::convert (&val, 1);
    }
}
data_block typeConvert (helics_type_t type, int64_t val)
{
    switch (type)
    {
    case helics_type_t::helicsDouble:
        return ValueConverter<double>::convert (static_cast<double> (val));
    case helics_type_t::helicsInt:
    default:
        return ValueConverter<int64_t>::convert (val);
    case helics_type_t::helicsComplex:
        return ValueConverter<std::complex<double>>::convert (std::complex<double> (val, 0.0));
    case helics_type_t::helicsBool:
        return (val != 0) ? "1" : "0";
    case helics_type_t::helicsString:
        return std::to_string (val);
    case helics_type_t::helicsNamedPoint:
        if (std::abs (val) > (2ll << 51))
        {
            return ValueConverter<named_point>::convert (named_point{std::to_string (val), std::nan ("0")});
        }
        else
        {
            return ValueConverter<named_point>::convert (named_point{"value", static_cast<double> (val)});
        }

    case helics_type_t::helicsComplexVector:
    {
        std::complex<double> v2 (static_cast<double> (val), 0.0);
        return ValueConverter<std::complex<double>>::convert (&v2, 1);
    }
    case helics_type_t::helicsVector:
    {
        auto v2 = static_cast<double> (val);
        return ValueConverter<double>::convert (&v2, 1);
    }
    }
}

data_block typeConvert (helics_type_t type, const char *val)
{
    if (val == nullptr)
    {
        return emptyBlock (type);
    }
    switch (type)
    {
    case helics_type_t::helicsDouble:
        return ValueConverter<double>::convert (boost::lexical_cast<double> (val));
    case helics_type_t::helicsInt:
        return ValueConverter<int64_t>::convert (boost::lexical_cast<int64_t> (val));
    case helics_type_t::helicsComplex:
        return ValueConverter<std::complex<double>>::convert (helicsGetComplex (val));
    case helics_type_t::helicsBool:
        return (std::string ("0") == val) ? "0" : "1";
    case helics_type_t::helicsString:
    default:
        return data_block (val);
    case helics_type_t::helicsNamedPoint:
        return ValueConverter<named_point>::convert (named_point{val, std::nan ("0")});
    case helics_type_t::helicsComplexVector:
        return ValueConverter<std::vector<std::complex<double>>>::convert (helicsGetComplexVector (val));
    case helics_type_t::helicsVector:
        return ValueConverter<std::vector<double>>::convert (helicsGetVector (val));
    }
}

data_block typeConvert (helics_type_t type, const std::string &val)
{
    if (val.empty ())
    {
        return emptyBlock (type);
    }

    switch (type)
    {
    case helics_type_t::helicsDouble:
        return ValueConverter<double>::convert (boost::lexical_cast<double> (val));
    case helics_type_t::helicsInt:
        return ValueConverter<int64_t>::convert (boost::lexical_cast<int64_t> (val));
    case helics_type_t::helicsComplex:
        return ValueConverter<std::complex<double>>::convert (helicsGetComplex (val));
    case helics_type_t::helicsBool:
        return (val == "0") ? val : "1";
    case helics_type_t::helicsString:
    default:
        return val;
    case helics_type_t::helicsNamedPoint:
        return ValueConverter<named_point>::convert (named_point{val, std::nan ("0")});
    case helics_type_t::helicsComplexVector:
        return ValueConverter<std::vector<std::complex<double>>>::convert (helicsGetComplexVector (val));
    case helics_type_t::helicsVector:
        return ValueConverter<std::vector<double>>::convert (helicsGetVector (val));
    }
}

data_block typeConvert (helics_type_t type, const std::vector<double> &val)
{
    if (val.empty ())
    {
        return emptyBlock (type, helics_type_t::helicsVector);
    }
    switch (type)
    {
    case helics_type_t::helicsDouble:
        return ValueConverter<double>::convert (val[0]);

    case helics_type_t::helicsInt:
        return ValueConverter<int64_t>::convert (static_cast<int64_t> (val[0]));
    case helics_type_t::helicsComplex:
    {
        std::complex<double> V (0.0, 0.0);
        if (val.size () >= 2)
        {
            V = std::complex<double> (val[0], val[1]);
        }
        else if (val.size () == 1)
        {
            V = std::complex<double> (val[0], 0.0);
        }
        return ValueConverter<std::complex<double>>::convert (V);
    }
    case helics_type_t::helicsBool:
        return (val[0] != 0.0) ? "1" : "0";
    case helics_type_t::helicsString:
        return helicsVectorString (val);
    case helics_type_t::helicsNamedPoint:
        return ValueConverter<named_point>::convert (named_point{helicsVectorString (val), std::nan ("0")});
    case helics_type_t::helicsComplexVector:
    {
        std::vector<std::complex<double>> CD;
        CD.reserve (val.size () / 2);
        for (size_t ii = 0; ii < val.size () - 1; ++ii)
        {
            CD.emplace_back (val[ii], val[ii + 1]);
        }
        return ValueConverter<std::vector<std::complex<double>>>::convert (CD);
    }
    break;
    case helics_type_t::helicsVector:
    default:
        return ValueConverter<std::vector<double>>::convert (val);
    }
}

data_block typeConvert (helics_type_t type, const double *vals, size_t size)
{
    if ((vals == nullptr) || (size == 0))
    {
        return emptyBlock (type, helics_type_t::helicsVector);
    }
    switch (type)
    {
    case helics_type_t::helicsDouble:
        return ValueConverter<double>::convert (vals[0]);

    case helics_type_t::helicsInt:
        return ValueConverter<int64_t>::convert (static_cast<int64_t> (vals[0]));
    case helics_type_t::helicsComplex:
    {
        std::complex<double> V (0.0, 0.0);
        if (size >= 2)
        {
            V = std::complex<double> (vals[0], vals[1]);
        }
        else if (size == 1)
        {
            V = std::complex<double> (vals[0], 0.0);
        }
        return ValueConverter<std::complex<double>>::convert (V);
    }
    case helics_type_t::helicsBool:
        return (vals[0] != 0.0) ? "1" : "0";
    case helics_type_t::helicsString:
        return helicsVectorString (vals, size);
    case helics_type_t::helicsNamedPoint:
        return ValueConverter<named_point>::convert (named_point{helicsVectorString (vals, size), std::nan ("0")});
    case helics_type_t::helicsComplexVector:
    {
        std::vector<std::complex<double>> CD;
        CD.reserve (size / 2);
        for (size_t ii = 0; ii < size - 1; ++ii)
        {
            CD.emplace_back (vals[ii], vals[ii + 1]);
        }
        return ValueConverter<std::vector<std::complex<double>>>::convert (CD);
    }
    break;
    case helics_type_t::helicsVector:
    default:
        return ValueConverter<double>::convert (vals, size);
    }
}

data_block typeConvert (helics_type_t type, const std::vector<std::complex<double>> &val)
{
    if (val.empty ())
    {
        return emptyBlock (type, helics_type_t::helicsComplexVector);
    }
    switch (type)
    {
    case helics_type_t::helicsDouble:
        return ValueConverter<double>::convert (std::abs (val[0]));
    case helics_type_t::helicsInt:
        return ValueConverter<int64_t>::convert (std::abs (val[0]));
    case helics_type_t::helicsComplex:
        return ValueConverter<std::complex<double>>::convert (val[0]);
    case helics_type_t::helicsBool:
        return (std::abs (val[0]) != 0.0) ? "1" : "0";
    case helics_type_t::helicsString:
        return helicsComplexVectorString (val);
    case helics_type_t::helicsNamedPoint:
        return ValueConverter<named_point>::convert (named_point{helicsComplexVectorString (val), std::nan ("0")});
    case helics_type_t::helicsComplexVector:
    default:
        return ValueConverter<std::vector<std::complex<double>>>::convert (val);
    case helics_type_t::helicsVector:
    {
        std::vector<double> DV;
        DV.reserve (val.size () * 2);
        for (auto &vali : val)
        {
            DV.push_back (vali.real ());
            DV.push_back (vali.imag ());
        }
        return ValueConverter<std::vector<double>>::convert (DV);
    }
    }
}
data_block typeConvert (helics_type_t type, const std::complex<double> &val)
{
    switch (type)
    {
    case helics_type_t::helicsDouble:
        return ValueConverter<double>::convert (std::abs (val));
    case helics_type_t::helicsInt:
        return ValueConverter<double>::convert (static_cast<int64_t> (std::abs (val)));
    case helics_type_t::helicsComplex:
    default:
        return ValueConverter<std::complex<double>>::convert (val);
    case helics_type_t::helicsBool:
        return (std::abs (val) != 0.0) ? "1" : "0";
    case helics_type_t::helicsString:
        return helicsComplexString (val);
    case helics_type_t::helicsNamedPoint:
        if (val.imag () == 0)
        {
            return ValueConverter<named_point>::convert (named_point{"value", val.real ()});
        }
        else
        {
            return ValueConverter<named_point>::convert (named_point{helicsComplexString (val), std::nan ("0")});
        }
    case helics_type_t::helicsComplexVector:
        return ValueConverter<std::complex<double>>::convert (&val, 1);
    case helics_type_t::helicsVector:
    {
        std::vector<double> V{val.real (), val.imag ()};
        return ValueConverter<std::vector<double>>::convert (V);
    }
    }
}

data_block typeConvert (helics_type_t type, const named_point &val)
{
    if (type == helics_type_t::helicsNamedPoint)
    {
        return ValueConverter<named_point>::convert (val);
    }
    if (isnan (val.value))
    {
        // just convert the string
        return typeConvert (type, val.name);
    }
    switch (type)
    {
    case helics_type_t::helicsDouble:
        return ValueConverter<double>::convert (val.value);
    case helics_type_t::helicsInt:
        return ValueConverter<int64_t>::convert (static_cast<int64_t> (val.value));
    case helics_type_t::helicsComplex:
        return ValueConverter<std::complex<double>>::convert (std::complex<double> (val.value, 0.0));
    case helics_type_t::helicsBool:
        return (val.value != 0) ? "1" : "0";
    case helics_type_t::helicsNamedPoint:
    default:
        return ValueConverter<named_point>::convert (val);
    case helics_type_t::helicsString:
        return (isnan (val.value)) ? val.name : helicsNamedPointString (val);
    case helics_type_t::helicsComplexVector:
    {
        std::complex<double> v2 (val.value, 0.0);
        return ValueConverter<std::complex<double>>::convert (&v2, 1);
    }
    case helics_type_t::helicsVector:
        return ValueConverter<double>::convert (&(val.value), 1);
    }
}

data_block typeConvert (helics_type_t type, const char *str, double val)
{
    if (type == helics_type_t::helicsNamedPoint)
    {
        return ValueConverter<named_point>::convert (named_point (str, val));
    }
    if (isnan (val))
    {
        // just convert the string
        return typeConvert (type, str);
    }
    switch (type)
    {
    case helics_type_t::helicsDouble:
        return ValueConverter<double>::convert (val);
    case helics_type_t::helicsInt:
        return ValueConverter<int64_t>::convert (static_cast<int64_t> (val));
    case helics_type_t::helicsComplex:
        return ValueConverter<std::complex<double>>::convert (std::complex<double> (val, 0.0));
    case helics_type_t::helicsBool:
        return (val != 0) ? "1" : "0";
    case helics_type_t::helicsNamedPoint:
    default:
        return ValueConverter<named_point>::convert (named_point (str, val));
    case helics_type_t::helicsString:
        return helicsNamedPointString (str, val);
    case helics_type_t::helicsComplexVector:
    {
        std::complex<double> v2 (val, 0.0);
        return ValueConverter<std::complex<double>>::convert (&v2, 1);
    }
    case helics_type_t::helicsVector:
        return ValueConverter<double>::convert (&(val), 1);
    }
}

data_block typeConvert (helics_type_t type, bool val)
{
    switch (type)
    {
    case helics_type_t::helicsDouble:
        return ValueConverter<double>::convert (val ? 1.0 : 0.0);
    case helics_type_t::helicsInt:
        return ValueConverter<int64_t>::convert (val ? 1 : 0);
    case helics_type_t::helicsComplex:
        return ValueConverter<std::complex<double>>::convert (std::complex<double> (val ? 1.0 : 0.0, 0.0));
    case helics_type_t::helicsString:
    case helics_type_t::helicsBool:
    default:
        return val ? "1" : "0";
    case helics_type_t::helicsNamedPoint:
    {
        named_point np{"value", val ? 1.0 : 0.0};
        return ValueConverter<named_point>::convert (np);
    }

    case helics_type_t::helicsComplexVector:
    {
        std::complex<double> v2 (val ? 1.0 : 0.0, 0.0);
        return ValueConverter<std::complex<double>>::convert (&v2, 1);
    }
    case helics_type_t::helicsVector:
    {
        auto v2 = val ? 1.0 : 0.0;
        return ValueConverter<double>::convert (&v2, 1);
    }
    }
}

data_block typeConvert (helics_type_t type, const std::string &str, double val)
{
    return typeConvert (type, str.c_str (), val);
}

}  // namespace helics
