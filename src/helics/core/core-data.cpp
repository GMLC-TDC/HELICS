/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "core-data.hpp"

#include <set>

namespace helics
{

static const std::set<std::string> global_match_strings{"any", "all", "data", "string", "block"};

bool matchingTypes (const std::string &type1, const std::string &type2)
{
    if (type1 == type2)
    {
        return true;
    }
    if ((type1.empty ()) || (type2.empty ()))
    {
        return true;
    }
    if ((type1.compare (0, 3, "def") == 0) || (type2.compare (0, 3, "def") == 0))
    {
        return true;
    }
    auto res = global_match_strings.find (type1);
    if (res != global_match_strings.end ())
    {
        return true;
    }
    res = global_match_strings.find (type2);
    return (res != global_match_strings.end ());
}
}  // namespace helics
