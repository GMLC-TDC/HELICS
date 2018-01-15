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

#include "stringOps.h"
#include "generic_string_ops.hpp"
#ifdef __GNUC__
#pragma GCC diagnostic push
//#pragma GCC diagnostic warning "-w"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#pragma GCC diagnostic pop
#else
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#endif

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>

#ifndef TRIM
#define TRIM(X) boost::algorithm::trim (X)
#endif

std::string convertToLowerCase (const std::string &input)
{
    std::string out;
	out.reserve(input.size());
	std::transform(input.begin(), input.end(), std::back_inserter(out), ::tolower);
    return out;
}

std::string convertToUpperCase (const std::string &input)
{
    std::string out (input);
    std::transform (input.begin (), input.end (), out.begin (), ::toupper);
    return out;
}

void makeLowerCase (std::string &input)
{
    std::transform (input.begin (), input.end (), input.begin (), ::tolower);
}

void makeUpperCase (std::string &input)
{
    std::transform (input.begin (), input.end (), input.begin (), ::toupper);
}

namespace stringOps
{
stringVector splitline (const std::string &line, const std::string &delimiters, delimiter_compression compression)
{
    stringVector strVec;
    auto comp = (compression == delimiter_compression::on) ? boost::token_compress_on : boost::token_compress_off;
    boost::algorithm::split (strVec, line, boost::is_any_of (delimiters), comp);
    return strVec;
}

stringVector splitline (const std::string &line, char del)
{
    stringVector strVec;
    boost::algorithm::split (strVec, line, boost::is_from_range (del, del));
    return strVec;
}

void splitline (const std::string &line,
                stringVector &strVec,
                const std::string &delimiters,
                delimiter_compression compression)
{
    auto comp = (compression == delimiter_compression::on) ? boost::token_compress_on : boost::token_compress_off;
    boost::algorithm::split (strVec, line, boost::is_any_of (delimiters), comp);
}

void splitline (const std::string &line, stringVector &strVec, char del)
{
    boost::algorithm::split (strVec, line, boost::is_from_range (del, del));
}

static const auto pmap = utilities::pairMapper ();

stringVector splitlineQuotes (const std::string &line,
                              const std::string &delimiters,
                              const std::string &quoteChars,
                              delimiter_compression compression)
{
    bool compress = (compression == delimiter_compression::on);
    return generalized_section_splitting (line, delimiters, quoteChars, pmap, compress);
}

stringVector splitlineBracket (const std::string &line,
                               const std::string &delimiters,
                               const std::string &bracketChars,
                               delimiter_compression compression)
{
    bool compress = (compression == delimiter_compression::on);
    return generalized_section_splitting (line, delimiters, bracketChars, pmap, compress);
}

void trimString (std::string &input, const std::string &whitespace)
{
    input.erase (input.find_last_not_of (whitespace) + 1);
    input.erase (0, input.find_first_not_of (whitespace));
}

std::string trim (const std::string &input, const std::string &whitespace)
{
    const auto strStart = input.find_first_not_of (whitespace);
    if (strStart == std::string::npos)
    {
        return "";  // no content
    }

    const auto strEnd = input.find_last_not_of (whitespace);

    return input.substr (strStart, strEnd - strStart + 1);
}

void trim (stringVector &input, const std::string &whitespace)
{
    for (auto &str : input)
    {
        trimString (str, whitespace);
    }
}

static const std::string digits ("0123456789");
int trailingStringInt (const std::string &input, std::string &output, int defNum)
{
    if ((input.empty())||(isdigit (input.back ()) == 0))
    {
        output = input;
        return defNum;
    }
    int num = defNum;
    auto pos1 = input.find_last_not_of (digits);
    if (pos1 == std::string::npos)  // in case the whole thing is a number
    {
        output.clear ();
        num = std::stol (input);
    }
    else
    {
        if (pos1 == input.length () - 2)
        {
            num = input.back () - '0';
        }
        else
        {
            num = std::stol (input.substr (pos1 + 1));
        }

        if ((input[pos1] == '_') || (input[pos1] == '#'))
        {
            output = input.substr (0, pos1);
        }
        else
        {
            output = input.substr (0, pos1 + 1);
        }
    }

    return num;
}

int trailingStringInt (const std::string &input, int defNum)
{
    if (isdigit (input.back ()) == 0)
    {
        return defNum;
    }

    auto pos1 = input.find_last_not_of (digits);
    if (pos1 == std::string::npos)  // in case the whole thing is a number
    {
        return std::stol (input);
    }

    if (pos1 == input.length () - 2)
    {
        return input.back () - '0';
    }

    return std::stol (input.substr (pos1 + 1));
}

static const std::string quoteChars (R"raw("'`)raw");

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

std::string removeBrackets (const std::string &str)
{
    std::string newString = trim (str);
    if (!newString.empty ())
    {
        if ((newString.front () == '[') || (newString.front () == '(') || (newString.front () == '{') ||
            (newString.front () == '<'))
        {
            if (newString.back () == pmap[newString.front ()])
            {
                newString.pop_back ();
                newString.erase (0, 1);
            }
        }
    }
    return newString;
}

std::string getTailString (const std::string &input, char sep)
{
    auto tc = input.find_last_of (sep);
    std::string ret = (tc == std::string::npos) ? input : input.substr (tc + 1);
    return ret;
}

std::string getTailString (const std::string &input, const std::string &sep)
{
    auto tc = input.find_last_of (sep);
    std::string ret = (tc == std::string::npos) ? input : input.substr (tc + 1);
    return ret;
}

int findCloseStringMatch (const stringVector &testStrings,
                          const stringVector &iStrings,
                          string_match_type_t matchType)
{
    std::string lct;  // lower case test string
    std::string lcis;  // lower case input string
    stringVector lciStrings = iStrings;
    // make all the input strings lower case
    for (auto &st : lciStrings)
    {
        makeLowerCase (st);
    }
    for (auto &ts : testStrings)
    {
        lct = convertToLowerCase (ts);
        for (int kk = 0; kk < static_cast<int> (lciStrings.size ()); ++kk)
        {
            lcis = lciStrings[kk];
            switch (matchType)
            {
            case string_match_exact:
                if (lcis == lct)
                {
                    return kk;
                }
                break;
            case string_match_begin:
                if (lct.compare (0, lct.length (), lcis) == 0)
                {
                    return kk;
                }
                break;
            case string_match_end:
                if (lct.length () > lcis.length ())
                {
                    continue;
                }
                if (lcis.compare (lcis.length () - lct.length (), lct.length (), lct) == 0)
                {
                    return kk;
                }
                break;
            case string_match_close:
                if (lct.length () == 1)  // special case
                {  // we are checking if the single character is isolated from other other alphanumeric characters
                    auto bf = lcis.find (lct);
                    while (bf != std::string::npos)
                    {
                        if (bf == 0)
                        {
                            if ((isspace (lcis[bf + 1]) != 0) || (ispunct (lcis[bf + 1]) != 0))
                            {
                                return kk;
                            }
                        }
                        else if (bf == lcis.length () - 1)
                        {
                            if ((isspace (lcis[bf - 1]) != 0) || (ispunct (lcis[bf - 1]) != 0))
                            {
                                return kk;
                            }
                        }
                        else
                        {
                            if ((isspace (lcis[bf - 1]) != 0) || (ispunct (lcis[bf - 1]) != 0))
                            {
                                if ((isspace (lcis[bf + 1]) != 0) || (ispunct (lcis[bf + 1]) != 0))
                                {
                                    return kk;
                                }
                            }
                        }
                        bf = lcis.find (lct, bf + 1);
                    }
                }
                else
                {
                    auto bf = lcis.find (lct);
                    if (bf != std::string::npos)
                    {
                        return kk;
                    }
                    auto nstr = removeChar (lct, '_');
                  if (lcis==nstr)
                    {
                        return kk;
                    }
                }
                break;
            }
        }
    }
    return -1;
}

std::string removeChars (const std::string &source, const std::string &remchars)
{
    std::string result;
    result.reserve (source.length ());
	std::remove_copy_if(source.begin(), source.end(), std::back_inserter(result), [remchars](char in) {return (std::find(remchars.begin(), remchars.end(), in) != remchars.end()); });
	return result;
	/*
    for (auto sc : source)
    {
        bool foundany = false;
        for (auto tc : remchars)
        {
            if (sc == tc)
            {
                foundany = true;
                break;
            }
        }
        if (!foundany)
        {
            result.push_back (sc);
        }
    }
    return result;
	*/
}

std::string removeChar (const std::string &source, char remchar)
{
    std::string result;
    result.reserve (source.length ());
	std::remove_copy(source.begin(), source.end(),std::back_inserter(result), remchar);
    return result;
}

std::string characterReplace (const std::string &source, char key, std::string repStr)
{
    std::string result;
    result.reserve (source.length ());
    for (auto sc : source)
    {
        if (sc == key)
        {
			result += repStr;
        }
        else
        {
            result.push_back (sc);
        }
    }
    return result;
}

std::string xmlCharacterCodeReplace (std::string str)
{
    std::string out = std::move (str);
    auto tt = out.find ("&gt;");
    while (tt != std::string::npos)
    {
        out.replace (tt, 4, ">");
        tt = out.find ("&gt;");
    }
    tt = out.find ("&lt;");
    while (tt != std::string::npos)
    {
        out.replace (tt, 4, "<");
        tt = out.find ("&lt;");
    }
    tt = out.find ("&amp;");
    while (tt != std::string::npos)
    {
        out.replace (tt, 5, "&");
        tt = out.find ("&amp;");
    }
    tt = out.find ("&quot;");
    while (tt != std::string::npos)
    {
        out.replace (tt, 6, "\"");
        tt = out.find ("&quot;");
    }
    tt = out.find ("&apos;");
    while (tt != std::string::npos)
    {
        out.replace (tt, 6, "'");
        tt = out.find ("&apos;");
    }
    return out;
}
}  // namespace stringOps