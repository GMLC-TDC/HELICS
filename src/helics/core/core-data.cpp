/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "core-data.h"
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
    : time (m.time), flags(m.flags),data (std::move (m.data)), dest(std::move(m.dest)), src(std::move(m.src)), origsrc (std::move (m.origsrc)),
       orig_dest(std::move(m.orig_dest))

{
}

Message &Message::operator= (Message &&m) noexcept
{
    origsrc = std::move (m.origsrc);
    src = std::move (m.src);
    dest = std::move (m.dest);
    data = std::move (m.data);
    time = m.time;
    flags = m.flags;
    orig_dest = std::move(m.orig_dest);
    return *this;
}

void Message::swap (Message &m2) noexcept
{
    origsrc.swap (m2.origsrc);
    src.swap (m2.src);
    dest.swap (m2.dest);
    data.swap (m2.data);
    std::swap (time, m2.time);
    std::swap(flags, m2.flags);
    orig_dest.swap(m2.orig_dest);
}

bool Message::isValid () const noexcept
{
    return (!data.empty ()) ? true : ((!src.empty ()) ? true : (!dest.empty ()));
}
}
