/*

Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. 
SPDX-License-Identifier: BSD-3-Clause
*/
#include "queryFunctions.hpp"

#include "Federate.hpp"
#include <thread>

#include "json/jsoncpp.h"

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

bool waitForInit (helics::Federate *fed, const std::string &fedName, std::chrono::milliseconds timeout)
{
    auto res = fed->query (fedName, "isinit");
    std::chrono::milliseconds waitTime{ 0 };
    const std::chrono::milliseconds delta{ 400 };
    while (res != "true")
    {
        if (res == "#invalid")
        {
            return false;
        }
        std::this_thread::sleep_for (delta);
        res = fed->query (fedName, "isinit");
        waitTime += delta;
        if (waitTime >= timeout)
        {
            return false;
        }
    }
    return true;
}

bool waitForFed (helics::Federate *fed, const std::string &fedName, std::chrono::milliseconds timeout)
{
    auto res = fed->query (fedName, "exists");
    std::chrono::milliseconds waitTime{ 0 };
    const std::chrono::milliseconds delta{ 400 };
    while (res != "true")
    {
        std::this_thread::sleep_for (delta);
        res = fed->query (fedName, "exists");
        waitTime += delta;
        if (waitTime >= timeout)
        {
            return false;
        }
    }
    return true;
}
