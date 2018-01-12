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

#ifndef GENERIC_STRING_OPS_H_
#define GENERIC_STRING_OPS_H_
#pragma once

/** this file defines some operations that can be performed on string like objects
*/
#include "charMapper.h"
#include <vector>

template <class X>
std::vector<X> generalized_string_split (const X &str, const X &delimiterCharacters, bool compress)
{
    std::vector<X> ret;

    auto pos = str.find_first_of (delimiterCharacters);
    decltype (pos) start = 0;
    while (pos != X::npos)
    {
        if (pos != start)
        {
            ret.push_back (str.substr (start, pos - start));
        }
        else if (!compress)
        {
            ret.push_back (X ());
        }
        start = pos + 1;
        pos = str.find_first_of (delimiterCharacters, start);
    }
    if (start < str.length ())
    {
        ret.push_back (str.substr (start));
    }
    else if (!compress)
    {
        ret.push_back (X ());
    }
    return ret;
}

template <class X>
size_t getChunkEnd(size_t start, const X &str, char ChunkStart, char ChunkEnd )
{
	int open = 1;
	size_t rlc = start;

	while (open > 0)
	{
		auto newOpen = str.find_first_of(ChunkStart, rlc + 1);
		auto newClose = str.find_first_of(ChunkEnd, rlc + 1);
		if (newClose == X::npos)
		{
			rlc = X::npos;
			break;
		}
		if (newOpen<newClose)
		{
			++open;
			rlc = newOpen;
		}
		else
		{
			--open;
			rlc = newClose;
		}
	}
	return rlc;
}

template <class X>
std::vector<X> generalized_section_splitting (const X &line,
                                              const X &delimiterCharacters,
                                              const X &sectionStartCharacters,
                                              const utilities::charMapper<unsigned char> &sectionMatch,
                                              bool compress)
{
    auto sectionLoc = line.find_first_of (sectionStartCharacters);

    if (sectionLoc == X::npos)
    {
        return generalized_string_split (line, delimiterCharacters, compress);
    }

    auto d1 = line.find_first_of (delimiterCharacters);
    if (d1 == X::npos)  // there are no delimiters
    {
        return {line};
    }
    decltype (sectionLoc) start = 0;
    std::vector<X> strVec;
    while (start < line.length ())
    {
        if (sectionLoc > d1)
        {
            if (start == d1)
            {
                if (!compress)
                {
                    strVec.push_back (X ());
                }
            }
            else
            {
                strVec.push_back (line.substr (start, d1 - start));
            }
            start = d1 + 1;
            d1 = line.find_first_of (delimiterCharacters, start);
        }
        else  // now we are in a quote
        {
			auto endLoc = getChunkEnd(sectionLoc + 1, line, line[sectionLoc], sectionMatch[line[sectionLoc]]);
            if (endLoc != X::npos)
            {
                d1 = line.find_first_of (delimiterCharacters, endLoc + 1);
				if (d1 == X::npos)
				{
					strVec.push_back(line.substr(start));
					sectionLoc = d1;
					start = d1;
				}
				else
				{
					strVec.push_back(line.substr(start, d1 - start));
					sectionLoc = line.find_first_of(sectionStartCharacters, d1 + 1);
					start = d1 + 1;
				}
				d1 = line.find_first_of(delimiterCharacters, start);
            }
            else
            {
                strVec.push_back (line.substr (start));
                start = sectionLoc;
            }
        }
        // get the last string
        if (d1 == X::npos)
        {
			if (start != X::npos)
			{
				if ((start < line.length()) || (!compress))
				{
					strVec.push_back(line.substr(start));
				}
				start = d1;
			}
        }
    }
    return strVec;
}

#endif