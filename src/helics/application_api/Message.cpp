/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "Message.h"
#include "helics/core/core-data.h"
namespace helics
{
data_block::data_block (const data_view &dv) : m_data (dv.data (), dv.size ()) {}

data_block::data_block(data_block &&db) noexcept: m_data(std::move(db.m_data)) {}

data_block &data_block::operator= (data_block &&db) noexcept
{
	m_data = std::move(db.m_data);
	return *this;
}

data_block &data_block::operator= (const data_view &dv)
{
    m_data = dv.string ();
    return *this;
}

void data_block::append (const data_view &dv) { m_data.append (dv.data (), dv.size ()); }


data_view::data_view (std::shared_ptr<const data_t> core_data) noexcept
    : dblock (core_data->data, core_data->len), core_data_ref (std::move (core_data))
{
}

data_view &data_view::operator= (std::shared_ptr<const data_t> core_data) noexcept
{
    dblock = stx::string_view (core_data->data, core_data->len);
    ref = nullptr;
    core_data_ref = std::move (core_data);
    return *this;
}
/*
std::string origsrc;
    std::string src;
    std::string dest;
    data_block data;
    Time time;
    */

Message::Message (const Message_view &mv)
    : origsrc (mv.origsrc.to_string ()), src (mv.src.to_string ()), dest (mv.dest.to_string ()), data (mv.data),
      time (mv.time)
{
}


Message::Message (const message_t *mt)
    : origsrc (mt->origsrc), src (mt->src), dest (mt->dst), data (mt->data, mt->len), time (mt->time)
{
}

Message::Message(Message &&m) noexcept : origsrc(std::move(m.origsrc)), src(std::move(m.src)), dest(std::move(m.dest)), data(std::move(m.data)),
time(m.time)
{

}

Message &Message::operator= (const Message_view &mv)
{
    origsrc = mv.origsrc.to_string ();
    src = mv.src.to_string ();
    dest = mv.dest.to_string ();
    data = mv.data;
    time = mv.time;
    return *this;
}

Message &Message::operator= (const message_t *mt)
{
    origsrc = mt->origsrc;
    src = mt->src;
    dest = mt->dst;
    data.assign (mt->data, mt->len);
    time = mt->time;
    return *this;
}

Message &Message::operator= (Message &&m) noexcept
{
	origsrc = std::move(m.origsrc);
	src = std::move(m.src);
	dest = std::move(m.dest);
	data = std::move(m.data);
	time = m.time;
	return *this;
}

void Message::swap (Message &m2) noexcept
{
    origsrc.swap (m2.origsrc);
    src.swap (m2.src);
    dest.swap (m2.dest);
    data.swap (m2.data);
	std::swap(time, m2.time);
}


bool Message::isValid () const
{
    return (data.size () > 0) ? true : ((src.size () > 0) ? true : (dest.size () > 0));
}

Message_view::Message_view (Message_view &&mv) noexcept (
  std::is_nothrow_move_constructible<stx::string_view>::value)
    : origsrc (mv.origsrc), src (mv.src), dest (mv.dest), data (std::move (mv.data)), time (mv.time),
      mmp (std::move (mv.mmp)), mmt (std::move (mv.mmt))
{
}
Message_view::Message_view (std::shared_ptr<const Message> mp) noexcept
    : origsrc (mp->origsrc), src (mp->src), dest (mp->dest), data (mp->data), time (mp->time), mmp (std::move (mp))
{
}

Message_view::Message_view (std::shared_ptr<message_t> mt) noexcept
    : origsrc (mt->origsrc), src (mt->src), dest (mt->dst), data (mt->data, mt->len), time (mt->time),
      mmt (std::move (mt))
{
}

Message_view::Message_view (const Message &m) noexcept
    : origsrc (m.origsrc), src (m.src), dest (m.dest), data (m.data), time (m.time)
{
}

Message_view &Message_view::operator= (Message_view &&mv) noexcept
{
	origsrc = mv.origsrc;
	src = mv.src;
	dest = mv.dest;
	data = std::move(mv.data);
	time = mv.time;
	mmt = std::move(mv.mmt);
	mmp = std::move(mv.mmp);
	return *this;
}

Message_view &Message_view::operator= (std::shared_ptr<const Message> mp) noexcept
{
    origsrc = mp->origsrc;
    src = mp->src;
    dest = mp->dest;
    data = data_view (mp->data);
    time = mp->time;
    mmt = nullptr;
    mmp = std::move (mp);
    return *this;
}

Message_view &Message_view::operator= (std::shared_ptr<message_t> mt) noexcept
{
    origsrc = mt->origsrc;
    src = mt->src;
    dest = mt->dst;
    data = data_view (mt->data, mt->len);
    time = mt->time;
    mmt = std::move (mt);
    mmp = nullptr;
    return *this;
}

Message_view &Message_view::operator= (const Message &m) noexcept
{
    origsrc = m.origsrc;
    src = m.src;
    dest = m.dest;
    data = data_view (m.data);
    time = m.time;
    mmt = nullptr;
    mmp = nullptr;
    return *this;
}


void Message_view::swap (Message_view &m2) noexcept
{
    origsrc.swap (m2.origsrc);
    src.swap (m2.src);
    dest.swap (m2.dest);
    data.swap (m2.data);
	std::swap(time, m2.time);
    mmt.swap (m2.mmt);
    mmp.swap (m2.mmp);
}

bool Message_view::isValid () const noexcept
{
    return (data.size () > 0) ? true : ((src.size () > 0) ? true : (dest.size () > 0));
}
}
