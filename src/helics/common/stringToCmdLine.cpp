/*
 * LLNS Copyright Start
 * Copyright (c) 2017, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was modified by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute;
the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence
Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

#include "stringToCmdLine.h"

std::vector<std::string> splitlineQuotes (const std::string &line);

std::string removeQuotes (const std::string &str);

stringToCmdLine::stringToCmdLine (const std::string &cmdString) { load (cmdString); }

static std::string nullstr;

void stringToCmdLine::load (const std::string &cmdString)
{
    stringCap = splitlineQuotes (cmdString);

    for (auto &str : stringCap)
    {
        str = removeQuotes (str);
        auto eloc = str.find_first_of ('=');
        if ((eloc != std::string::npos) && (str.size () > eloc + 2))
        {
            if ((str[eloc + 1] == '"') && (str.back () == '"'))
            {
                str.pop_back ();
                str.erase (eloc + 1, 1);
            }
            else if ((str[eloc + 1] == '\'') && (str.back () == '\''))
            {
                str.pop_back ();
                str.erase (eloc + 1, 1);
            }
            else if ((str[eloc + 1] == '`') && (str.back () == '`'))
            {
                str.pop_back ();
                str.erase (eloc + 1, 1);
            }
        }
    }

    argCount = static_cast<int> (stringCap.size ()) + 1;
    stringPtrs.resize (argCount);
    for (int ii = 1; ii < argCount; ++ii)
    {
        stringPtrs[ii] = &(stringCap[ii - 1][0]);
    }
    stringPtrs[0] = &(nullstr[0]);
}
static const std::string quoteChars (R"raw("'`)raw");

static const std::string delimChars (" \t\n\r");

std::vector<std::string> splitlineQuotes (const std::string &line)
{
    auto sectionLoc = line.find_first_of (quoteChars);
    if ((sectionLoc != std::string::npos) && (sectionLoc > 0))
    {
        while (line[sectionLoc - 1] != '\\')  // check for escape character
        {
            sectionLoc = line.find_first_of (quoteChars, sectionLoc + 1);
        }
    }
    std::vector<std::string> strVec;
    decltype (sectionLoc) start = 0;
    if (sectionLoc == std::string::npos)
    {
        auto pos = line.find_first_of (delimChars);
        while (pos != std::string::npos)
        {
            if (pos != start)
            {
                strVec.push_back (line.substr (start, pos - start));
            }

            start = pos + 1;
            pos = line.find_first_of (delimChars, start);
        }
        if (start < line.length ())
        {
            strVec.push_back (line.substr (start));
        }
        return strVec;
    }

    auto d1 = line.find_first_of (delimChars);
    if (d1 == std::string::npos)  // there are no delimiters
    {
        return {line};
    }

    while (start < line.length ())
    {
        if (sectionLoc > d1)
        {
            if (start != d1)
            {
                strVec.push_back (line.substr (start, d1 - start));
            }
            start = d1 + 1;
            d1 = line.find_first_of (delimChars, start);
        }
        else  // now we are in a quote
        {
            sectionLoc = line.find_first_of (line[sectionLoc], sectionLoc + 1);
            if (sectionLoc != std::string::npos)
            {
                d1 = line.find_first_of (delimChars, sectionLoc + 1);
                sectionLoc = line.find_first_of (quoteChars, d1 + 1);
            }
            else
            {
                strVec.push_back (line.substr (start));
                start = sectionLoc;
            }
        }
        // get the last string
        if (d1 == std::string::npos)
        {
            if (start < line.length ())
            {
                strVec.push_back (line.substr (start));
            }
            start = d1;
        }
    }
    return strVec;
}

std::string trim (const std::string &input)
{
    const auto strStart = input.find_first_not_of (delimChars);
    if (strStart == std::string::npos)
    {
        return "";  // no content
    }

    const auto strEnd = input.find_last_not_of (delimChars);

    return input.substr (strStart, strEnd - strStart + 1);
}

std::string removeQuotes (const std::string &str)
{
    auto newString = trim (str);
    if (!newString.empty ())
    {
        if ((newString.front () == '\"') || (newString.front () == '\'') || (newString.front () == '`'))
        {
            if (newString.back () == newString.front ())
            {
                newString.pop_back ();
                newString.erase (0, 1);
            }
        }
    }
    return newString;
}
