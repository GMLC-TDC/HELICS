/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "MessageGenerator.hpp"

#include <fstream>
#include <iostream>

#include "helics/common/stringOps.h"

namespace helics
{
namespace test
{
MessageGenerator::MessageGenerator (const std::string &messageFile) { loadMessages (messageFile); }

void MessageGenerator::loadMessages (const std::string &messageFile)
{
    using namespace stringOps;
    std::ifstream infile (messageFile);
    std::string str;

    bool mlineComment = false;
    // count the lines
    while (std::getline (infile, str))
    {
        if (str.empty ())
        {
            continue;
        }
        auto fc = str.find_first_not_of (" \t\n\r\0");
        if (fc == std::string::npos)
        {
            continue;
        }
        if (mlineComment)
        {
            if (fc + 2 < str.size ())
            {
                if ((str[fc] == '#') && (str[fc + 1] == '#') && (str[fc + 2] == ']'))
                {
                    mlineComment = false;
                }
            }
            continue;
        }
        else if (str[fc] == '#')
        {
            if (fc + 2 < str.size ())
            {
                if ((str[fc + 1] == '#') && (str[fc + 2] == '['))
                {
                    mlineComment = true;
                }
            }
            continue;
        }
        /* time key type value units*/
        auto blk = splitlineBracket (str, ",\t ", default_bracket_chars, delimiter_compression::off);

        trimString (blk[0]);
        if (blk[0] == "grant")
        {
            if (blk.size () < 4)
            {
                throw (std::invalid_argument ("grant block not valid"));
            }
            ActionMessage grant (CMD_TIME_GRANT);
            grant.source_id = global_federate_id (std::stoi (blk[1]));
            grant.dest_id = global_federate_id (std::stoi (blk[2]));
            grant.actionTime = loadTimeFromString (blk[3]);
            messages.push_back (grant);
        }
        else if (blk[0] == "request")
        {
            if (blk.size () < 7)
            {
                throw (std::invalid_argument ("grant block not valid"));
            }
            ActionMessage req (CMD_TIME_REQUEST);
            req.source_id = global_federate_id (std::stoi (blk[1]));
            req.dest_id = global_federate_id (std::stoi (blk[2]));
            req.actionTime = loadTimeFromString (blk[3]);
            req.Te = loadTimeFromString (blk[4]);
            req.Tdemin = loadTimeFromString (blk[5]);
            req.Tso = loadTimeFromString (blk[6]);
            messages.push_back (req);
        }
        else if (blk[0] == "ignore")
        {
            messages.emplace_back (CMD_IGNORE);
        }
    }
}

void MessageGenerator::addMessage (ActionMessage message) { messages.push_back (std::move (message)); }

ActionMessage MessageGenerator::nextMessage ()
{
    ActionMessage mess = CMD_IGNORE;
    if (!messages.empty ())
    {
        mess = std::move (messages.front ());
        messages.pop_front ();
    }
    return mess;
}
}  // namespace test
}  // namespace helics
