/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "helics/core/core-data.h"
namespace helics
{
data_block::data_block (data_block &&db) noexcept : m_data (std::move (db.m_data)) {}

data_block &data_block::operator= (data_block &&db) noexcept
{
    m_data = std::move (db.m_data);
    return *this;
}

/*
std::string origsrc;
std::string src;
std::string dest;
data_block data;
Time time;
*/

Message::Message (Message &&m) noexcept
    : time (m.time), data (std::move (m.data)), origsrc (std::move (m.origsrc)), dest (std::move (m.dest)),
      src (std::move (m.src))

{
}

Message &Message::operator= (Message &&m) noexcept
{
    origsrc = std::move (m.origsrc);
    src = std::move (m.src);
    dest = std::move (m.dest);
    data = std::move (m.data);
    time = m.time;
    return *this;
}

void Message::swap (Message &m2) noexcept
{
    origsrc.swap (m2.origsrc);
    src.swap (m2.src);
    dest.swap (m2.dest);
    data.swap (m2.data);
    std::swap (time, m2.time);
}

bool Message::isValid () const noexcept
{
    return (!data.empty ()) ? true : ((!src.empty ()) ? true : (!dest.empty ()));
}
}
