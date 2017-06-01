/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#ifndef _HELICS_MESSAGE_H_
#define _HELICS_MESSAGE_H_
#pragma once

#include "extra_includes/string_view.h"
#include "helics/config.h"
#include "helics/core/helics-time.h"
#include <memory>
#include <string>
#include <vector>

namespace helics
{
/** forward declaration*/
class data_view;

/** basic data object for use in the user API layer
@details an adapter over a string,  many objects will be strings actually so this is just a wrapper for that
common use case, and many other objects are small, so the small string optimization takes advantage of that
*/
class data_block
{
  private:
    std::string m_data;  //!< using a string to represent the data
    friend class data_view;  //!< let data view access the string directly
  public:
    /** default constructor */
    data_block () noexcept {};
    /** size allocation constructor */
    data_block (size_t blockSize) { m_data.resize (blockSize); };
    /** size and data */
    data_block (size_t blockSize, char init) : m_data (blockSize, init){};
    /** copy constructor from a data_view */
    data_block (const data_view &dv);
    /** copy constructor */
    data_block (const data_block &dt) = default;
    /** move constructor */
    data_block (data_block &&dt) noexcept;
    /** construct from char * */
    data_block (const char *s) : m_data (s){};
    /** construct from string */
    data_block (const std::string &str) : m_data (str){};
    /** move from string */
    data_block (std::string &&str) noexcept : m_data (std::move (str)){};
    /** construct from string view*/
    data_block (stx::string_view str) : m_data (str.to_string ()){};
    /** char * and length */
    data_block (const char *s, size_t len) : m_data (s, len){};
    /** construct from a vector object */
    data_block (const std::vector<char> &vdata) : m_data (vdata.data (), vdata.size ()){};
    /** construct from an arbitrary vector*/
    template <class X>
    data_block (const std::vector<X> &vdata)
        : m_data (reinterpret_cast<const char *> (vdata.data ()), vdata.size () * sizeof (X))
    {
    }
    /** copy assignment operator*/
    data_block &operator= (const data_block &dt)=default;
    /** move assignment operator*/
    data_block &operator= (data_block &&dt) noexcept;
    /** assign from a data_view*/
    data_block &operator= (const data_view &dv);
    /** assign from a string*/
    data_block &operator= (std::string str)
    {
        m_data = std::move (str);
        return *this;
    }
    data_block &operator= (const char *s)
    {
        m_data.assign (s);
        return *this;
    }
    /** assignment from string and length*/
    data_block &assign (const char *s, size_t len)
    {
        m_data.assign (s, len);
        return *this;
    }
    /** swap function */
    void swap (data_block &db2) noexcept { m_data.swap (db2.m_data); }
    /** append the existing data with a additional data*/
    void append (const char *s, size_t len) { m_data.append (s, len); }
    /** append the existing data with a additional data*/
    void append (const data_view &dv);
    /** append the existing data with a string*/
    void append (const std::string &str) { m_data.append (str); }
    /** equality operator with another data block*/
    bool operator== (const data_block &db) const { return m_data == db.m_data; }
    /** equality operator with a string*/
    bool operator== (const std::string &str) const { return m_data == str; }
    /** less then operator to order the data_blocks if need be*/
    bool operator< (const data_block &db) const { return (m_data < db.m_data); }
    /** return a pointer to the data*/
    char *data () { return &(m_data.front ()); }
    /** if the object is const return a const pointer*/
    const char *data () const { return &(m_data.front ()); }

	/** check if the block is empty*/
	bool empty() const noexcept { return m_data.empty(); }
    /** get the size of the data block*/
    size_t size () const { return m_data.length (); }
    /** resize the data storage*/
    void resize (size_t newSize) { m_data.resize (newSize); }
    /** resize the data storage*/
    void resize (size_t newSize, char T) { m_data.resize (newSize, T); }
    /** get a string reference*/
    const std::string &string () const { return m_data; }
    /** bracket operator to get a character value*/
    char &operator[] (int index) { return m_data[index]; }
    /** bracket operator to get a character value*/
    char operator[] (int index) const { return m_data[index]; }
    /** non const iterator*/
    auto begin () { return m_data.begin (); }
    /** non const iterator end*/
    auto end () { return m_data.end (); }
    /** const iterator*/
    auto cbegin () const { return m_data.cbegin (); }
    /** const iterator end*/
    auto cend () const { return m_data.cend (); }
};

// forward declaration of the core data_t block
struct data_t;

/** class containing a constant view of data block*/
class data_view
{
  private:
    stx::string_view dblock; //!< using a string_view to represent the data
    std::shared_ptr<const data_block> ref;//!< need to capture a reference to the data being viewed if it is from a shared_ptr
    std::shared_ptr<const data_t> core_data_ref; //!< need to capture a reference to data coming from the core
  public:
    /** default constructor*/
    data_view () noexcept {};
    /** construct from a shared_ptr to a data_block*/
    data_view (std::shared_ptr<const data_block> dt) : dblock (dt->m_data), ref (std::move (dt)){};
    /** construct from a regular data_block*/
    data_view (const data_block &dt) noexcept : dblock (dt.m_data){};
    /** copy constructor*/
    data_view (const data_view &dt) noexcept = default;
    /** move constructor*/
    data_view (data_view &&dv) noexcept
        : dblock (dv.dblock), ref (std::move (dv.ref)), core_data_ref (std::move (dv.core_data_ref)){};

    /** construct from  a shared_ptr to a data_t object
    @details maintain a reference to the data as well
    */
    data_view (std::shared_ptr<const data_t> core_data) noexcept;
    /** construct from a string*/
    data_view (const char *dt) noexcept : dblock (dt){};
    /** construct from a char Pointer and length*/
    data_view (const char *dt, size_t len) noexcept : dblock (dt, len){};
    /** construct from a string*/
    data_view (const std::string &str) noexcept : dblock (str){};
    /** construct from a char vector*/
    data_view (const std::vector<char> &dvec) noexcept : dblock (dvec.data (), dvec.size ()){};
    /** construct from a string_view*/
    data_view (const stx::string_view &sview) noexcept : dblock (sview){};
    /** assignment operator from another ata_view*/
    data_view &operator= (const data_view &dv) noexcept
    {
        dblock = dv.dblock;
        ref = dv.ref;
        core_data_ref = dv.core_data_ref;
        return *this;
    }

    data_view &operator= (data_view &&dv) noexcept
    {
        dblock = dv.dblock;
        ref = std::move (dv.ref);
        core_data_ref = std::move (dv.core_data_ref);
        return *this;
    }

    /** assignment from a data_block shared_ptr*/
    data_view &operator= (std::shared_ptr<const data_block> dt) noexcept
    {
        dblock = dt->m_data;
        ref = std::move (dt);
        core_data_ref = nullptr;
        return *this;
    }
    /** assignment from a shared_ptr to a message block from the core*/
    data_view &operator= (std::shared_ptr<const data_t> dt) noexcept;
    /** assignment from a data_block*/
    data_view &operator= (const data_block &dt) noexcept
    {
        dblock = dt.m_data;
        ref = nullptr;
        core_data_ref = nullptr;
        return *this;
    }
    /** assignment from a string_view*/
    data_view &operator= (const stx::string_view &str) noexcept
    {
        dblock = str;
        ref = nullptr;
        core_data_ref = nullptr;
        return *this;
    }
    /** assignment from a const char * */
    data_view &operator= (const char *s) noexcept
    {
        dblock = s;
        ref = nullptr;
        core_data_ref = nullptr;
        return *this;
    }
    /** swap function */

    void swap (data_view &dv2) noexcept
    {
        dblock.swap (dv2.dblock);
        ref.swap (dv2.ref);
        core_data_ref.swap (dv2.core_data_ref);
    }
    /** get the data block*/
    const char *data () const noexcept{ return dblock.data (); }
    /** get the length*/
    size_t size () const noexcept { return dblock.length (); }
	/** check if the view is empty*/
	bool empty() const noexcept { return dblock.empty(); }
    /** return a string of the data
    @details this actually does a copy to a new string
    */
    std::string string () const { return dblock.to_string (); }
    /** random access operator*/
    char operator[] (int index) const { return dblock[index]; }
    /** begin iterator*/
    auto begin () { return dblock.begin (); }
    /** end iterator*/
    auto end () { return dblock.end (); }
    /** begin const iterator*/
    auto cbegin () const { return dblock.cbegin (); }
    /** end const iterator*/
    auto cend () const { return dblock.cend (); }
};

struct message_t;
class Message_view;
/** class containing a message structure*/
class Message
{
  public:
    std::string origsrc; //!< the orignal source of the message
    std::string src;	//!< the most recent source of the message
    std::string dest;	//!< the destination of the message
    data_block data;	//!< the data packet for the message
    Time time;	//!< the event time the message is sent

  public:
	  /** default constructor*/
    Message () noexcept {};
	/** move constructor*/
    Message (Message &&m) noexcept;
	/** copy constructor*/
    Message (const Message &m) = default;
	/** construct from a Message View*/
    Message (const Message_view &mv);
	/** construct from a core message_t*/
    Message (const message_t *mt);
	/** move assignement*/
    Message &operator= (Message &&m) noexcept;
	/** copy assignment*/
    Message &operator= (const Message &m) = default;
	/** copy from a message_view*/
    Message &operator= (const Message_view &mv);
    /** copy from a core message ptr*/
	Message &operator= (const message_t *mt);
	/** swap operation for the Message*/
    void swap (Message &m2) noexcept;
	/** check if the Message contains an actual Message
	@return false if there is no Message data*/
    bool isValid () const;
};

/**
*  Message_view object class
@details this object is a view of a Message without allocating the actual data
*/
class Message_view
{
  public:
    stx::string_view origsrc;  //!< a view of the original source
    stx::string_view src;  //!< a view of the recent source
    stx::string_view dest;  //!< a view of the destination
    data_view data;  //!< the message data
    Time time;  //!< the message Time

  private:
    std::shared_ptr<const Message> mmp;  //!< shared_ptr to a message
    std::shared_ptr<message_t> mmt;  //!< shared_ptr to core message_t
  public:
    /** default constructor*/
    Message_view () noexcept {};
    /** copy constructor*/
    Message_view (const Message_view &mv) noexcept = default;
    /** move constructor*/
    Message_view (Message_view &&mv) noexcept;
    /** construct from a shared pointer to a Message*/
    Message_view (std::shared_ptr<const Message> mp) noexcept;
    /** construct from a shared_ptr to a message_t*/
    Message_view (std::shared_ptr<message_t> mt) noexcept;
    /** construct from a Message*/
    Message_view (const Message &mv) noexcept;
    /** copy assignment*/
    Message_view &operator= (const Message_view &mv) noexcept = default;
    /** move assignment*/
    Message_view &operator= (Message_view &&mv) noexcept;
    /** assign from shared ptr to a message*/
    Message_view &operator= (std::shared_ptr<const Message> mp) noexcept;
    /** assign from a shared ptr to a core message_t*/
    Message_view &operator= (std::shared_ptr<message_t> mt) noexcept;
    /**assign from a const Message*/
    Message_view &operator= (const Message &m) noexcept;
    /** swap a message_view with another
    @param[in] m2 the message_view to swap with*/
    void swap (Message_view &m2) noexcept;
    /** check if the Message contains an actual message
	@return true if the message contains actual data or has a definitive source or destination*/
    bool isValid () const noexcept;
};
}


namespace std
{
	template<>
	inline void swap(helics::data_block & db1, helics::data_block &db2) noexcept
	{
		db1.swap(db2);
	}
}

namespace std
{
template <>
inline void swap (helics::data_view &db1, helics::data_view &db2) noexcept
{
    db1.swap (db2);
}
}

namespace std
{
template <>
inline void swap (helics::Message &m1, helics::Message &m2) noexcept
{
    m1.swap (m2);
}
}

namespace std
{
template <>
inline void swap (helics::Message_view &m1, helics::Message_view &m2) noexcept
{
    m1.swap (m2);
}
}

#endif
