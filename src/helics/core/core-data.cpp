/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "core-data.hpp"

#include <set>

namespace helics
{
data_block::data_block (data_block &&db) noexcept : m_data (std::move (db.m_data)) {}

data_block &data_block::operator= (data_block &&db) noexcept
{
    m_data = std::move (db.m_data);
    return *this;
}

Message::Message (Message &&m) noexcept
    : time (m.time), flags (m.flags), messageID (m.messageID), data (std::move (m.data)),
      dest (std::move (m.dest)), source (std::move (m.source)), original_source (std::move (m.original_source)),
      original_dest (std::move (m.original_dest))

{
}

Message &Message::operator= (Message &&m) noexcept
{
    time = m.time;
    flags = m.flags;
    messageID = m.messageID;
    data = std::move (m.data);
    original_source = std::move (m.original_source);
    source = std::move (m.source);
    dest = std::move (m.dest);
    original_dest = std::move (m.original_dest);
    return *this;
}

void Message::swap (Message &m2) noexcept
{
    std::swap (time, m2.time);
    std::swap (flags, m2.flags);
    std::swap (messageID, m2.messageID);
    original_source.swap (m2.original_source);
    source.swap (m2.source);
    dest.swap (m2.dest);
    data.swap (m2.data);
    original_dest.swap (m2.original_dest);
}

bool Message::isValid () const noexcept
{
    return (!data.empty ()) ? true : ((!source.empty ()) ? true : (!dest.empty ()));
}

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
    return (res != global_match_strings.end());
   
}
}  // namespace helics
