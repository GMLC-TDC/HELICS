/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "queryFunctions.h"

#include "Federate.h"
#include <thread>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#include "json/json.h"
#pragma warning(pop)
#else
#include "json/json.h"
#endif

#include <boost/algorithm/string.hpp>

std::vector<std::string> vectorizeQueryResult (std::string &&queryres)
{
    if (queryres.empty ())
    {
        return std::vector<std::string> ();
    }
    if (queryres.front () == '[')
    {
        std::vector<std::string> strs;
        boost::split (strs, queryres, [](char c) { return c == ';'; });
        strs.front () = strs.front ().substr (1);  // get rid of the leading '['
        strs.back ().pop_back ();  // get rid of the trailing ']';
        return strs;
    }
    std::vector<std::string> res;
    res.push_back (std::move (queryres));
    return res;
}

std::vector<std::string> vectorizeQueryResult (const std::string &queryres)
{
    if (queryres.empty ())
    {
        return std::vector<std::string> ();
    }
    if (queryres.front () == '[')
    {
        std::vector<std::string> strs;
        boost::split (strs, queryres, [](char c) { return c == ';'; });
        strs.front () = strs.front ().substr (1);  // get rid of the leading '['
        strs.back ().pop_back ();  // get rid of the trailing ']';
        return strs;
    }
    std::vector<std::string> res;
    res.push_back (queryres);
    return res;
}

std::vector<std::string> vectorizeAndSortQueryResult (const std::string &queryres)
{
    auto vec = vectorizeQueryResult (queryres);
    std::sort (vec.begin (), vec.end ());
    return vec;
}

std::vector<std::string> vectorizeAndSortQueryResult (std::string &&queryres)
{
    auto vec = vectorizeQueryResult (std::move (queryres));
    std::sort (vec.begin (), vec.end ());
    return vec;
}

bool waitForInit (helics::Federate *fed, const std::string &fedName, int timeout)
{
    auto res = fed->query (fedName, "isinit");
    int waitTime = 0;
    while (res != "true")
    {
        if (res == "#invalid")
        {
            return false;
        }
        std::this_thread::sleep_for (std::chrono::milliseconds (200));
        res = fed->query (fedName, "isinit");
        waitTime += 200;
        if (waitTime >= timeout)
        {
            return false;
        }
    }
    return true;
}

bool waitForFed (helics::Federate *fed, const std::string &fedName, int timeout)
{
    auto res = fed->query (fedName, "exists");
    int waitTime = 0;
    while (res != "true")
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (200));
        res = fed->query (fedName, "exists");
        waitTime += 200;
        if (waitTime >= timeout)
        {
            return false;
        }
    }
    return true;
}