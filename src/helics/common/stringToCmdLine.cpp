/*
 * LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#include "stringToCmdLine.h"
#include "stringOps.h"

StringToCmdLine::StringToCmdLine (const std::string &cmdString) { load (cmdString); }

static std::string nullstr;

void StringToCmdLine::load (const std::string &cmdString)
{
    stringCap = stringOps::splitlineQuotes (cmdString, " \t\n\r", stringOps::default_quote_chars,
                                            stringOps::delimiter_compression::on);

    for (auto &str : stringCap)
    {
        str = stringOps::removeQuotes (str);
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
