/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "helics-time.hpp"
#include "helics/helics-config.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

/** @file
@details defining data used for storing the data for values and for messages
*/

/** \namespace helics
@details the core namespace for the helics C++ library
all user functions are found in this namespace along with many other functions in the Core API
 */
namespace helics {
/** basic data object for use in the user API layer
@details An adapter over a string,  many objects will be strings actually so this is just a wrapper
for that common use case, and many other objects are small, so the small string optimization takes
advantage of that
*/
class data_block {
  private:
    std::string m_data;  //!< using a string to represent the data
    friend class data_view;  //!< let data view access the string directly
    friend class ActionMessage;  //!< let action Message access the string directly
  public:
    /** default constructor */
    data_block() = default;
    /**default destructor*/
    ~data_block() = default;
    /** size allocation constructor */
    explicit data_block(size_t blockSize) { m_data.resize(blockSize); }
    /** size and data */
    data_block(size_t blockSize, char init): m_data(blockSize, init) {}
    /** copy constructor */
    data_block(const data_block& db) = default;
    /** move constructor */
    data_block(data_block&& db) = default;
    /** construct from char * */
    // NOLINTNEXTLINE
    /* implicit */ data_block(const char* s): m_data(s) {}
    /** construct from string */
    // NOLINTNEXTLINE
    /* implicit */ data_block(const std::string& str): m_data(str) {}
    /** move from string */
    // NOLINTNEXTLINE
    /* implicit */ data_block(std::string&& str) noexcept: m_data(std::move(str)) {}
    /** char * and length */
    data_block(const char* s, size_t len): m_data(s, len) {}
    /** construct from a vector object */
    // NOLINTNEXTLINE
    /* implicit */ data_block(const std::vector<char>& vdata): m_data(vdata.data(), vdata.size()) {}
    /** construct from an arbitrary vector*/
    template<class X>
    // NOLINTNEXTLINE
    /* implicit */ data_block(const std::vector<X>& vdata):
        m_data(reinterpret_cast<const char*>(vdata.data()), vdata.size() * sizeof(X))
    {
    }
    /** copy assignment operator*/
    data_block& operator=(const data_block& db) = default;
    /** move assignment operator*/
    data_block& operator=(data_block&& db) = default;
    /** assign from a string*/
    data_block& operator=(std::string str)
    {
        m_data = std::move(str);
        return *this;
    }
    /** assign the data block from a const char * */
    data_block& operator=(const char* s)
    {
        m_data.assign(s);
        return *this;
    }
    /** assignment from string and length*/
    data_block& assign(const char* s, size_t len)
    {
        m_data.assign(s, len);
        return *this;
    }
    /** swap function */
    void swap(data_block& db2) noexcept { m_data.swap(db2.m_data); }
    /** append the existing data with a additional data*/
    void append(const char* s, size_t len) { m_data.append(s, len); }
    /** append the existing data with a string*/
    void append(const std::string& str) { m_data.append(str); }
    /** equality operator with another data block*/
    bool operator==(const data_block& db) const { return m_data == db.m_data; }
    /** equality operator with a string*/
    bool operator==(const std::string& str) const { return m_data == str; }
    /** less then operator to order the data_blocks if need be*/
    bool operator<(const data_block& db) const { return (m_data < db.m_data); }
    /** less then operator to order the data_blocks if need be*/
    bool operator>(const data_block& db) const { return (m_data > db.m_data); }
    /** return a pointer to the data*/
    char* data() { return &(m_data.front()); }
    /** if the object is const return a const pointer*/
    const char* data() const { return &(m_data.front()); }

    /** check if the block is empty*/
    bool empty() const noexcept { return m_data.empty(); }
    /** get the size of the data block*/
    size_t size() const { return m_data.length(); }
    /** resize the data storage*/
    void resize(size_t newSize) { m_data.resize(newSize); }
    /** resize the data storage*/
    void resize(size_t newSize, char T) { m_data.resize(newSize, T); }
    /** reserve space in a data_block*/
    void reserve(size_t space) { m_data.reserve(space); }
    /** get a string reference*/
    const std::string& to_string() const { return m_data; }
    /** bracket operator to get a character value*/
    char& operator[](int index) { return m_data[index]; }
    /** bracket operator to get a character value*/
    char operator[](int index) const { return m_data[index]; }
    /** non const iterator*/
    auto begin() { return m_data.begin(); }
    /** non const iterator end*/
    auto end() { return m_data.end(); }
    /** const iterator*/
    auto cbegin() const { return m_data.cbegin(); }
    /** const iterator end*/
    auto cend() const { return m_data.cend(); }
    /** add a character to the data*/
    void push_back(char newchar) { m_data.push_back(newchar); }
};

/** operator to check if two data blocks are not equal to each other*/
inline bool operator!=(const data_block& db1, const data_block& db2)
{
    return !(db1 == db2);
}

/** class containing a message structure*/
class Message {
  public:
    Time time = timeZero;  //!< the event time the message is sent
    std::uint16_t flags{0};  //!< message flags
    std::uint16_t messageValidation{0U};  //!< extra field for user object usage, not used by HELICS
    std::int32_t messageID{0};  //!< the messageID for a message
    data_block data;  //!< the data packet for the message
    std::string dest;  //!< the destination of the message
    std::string source;  //!< the most recent source of the message
    std::string original_source;  //!< the original source of the message
    std::string original_dest;  //!< the original destination of a message
    std::int32_t counter{0};  //!< indexing counter not used directly by helics
    void* backReference{nullptr};  //!< back referencing pointer not used by helics

    /** default constructor*/
    Message() = default;
    /** swap operation for the Message*/
    void swap(Message& m2) noexcept
    {
        std::swap(time, m2.time);
        std::swap(flags, m2.flags);
        std::swap(messageID, m2.messageID);
        original_source.swap(m2.original_source);
        source.swap(m2.source);
        dest.swap(m2.dest);
        data.swap(m2.data);
        original_dest.swap(m2.original_dest);
    }
    /** check if the Message contains an actual Message
    @return false if there is no Message data*/
    bool isValid() const noexcept
    {
        return (!data.empty()) ? true : ((!source.empty()) ? true : (!dest.empty()));
    }
    /** get the payload as a string*/
    const std::string& to_string() const { return data.to_string(); }
    /** clear all data from the message*/
    void clear()
    {
        time = timeZero;
        flags = 0;
        messageID = 0;
        data.resize(0);
        dest.clear();
        source.clear();
        original_source.clear();
        original_dest.clear();
        counter = 0;
    }
};

/**
 * FilterOperator abstract class
 @details FilterOperators will transform a message in some way in a direct fashion
 *
 */
class FilterOperator {
  public:
    /** default constructor*/
    FilterOperator() = default;
    /**virtual destructor*/
    virtual ~FilterOperator() = default;
    /** filter the message either modify the message or generate a new one*/
    virtual std::unique_ptr<Message> process(std::unique_ptr<Message> message) = 0;
    /** filter the message either modify the message or generate a new one*/
    virtual std::vector<std::unique_ptr<Message>> processVector(std::unique_ptr<Message> message)
    {
        std::vector<std::unique_ptr<Message>> ret;
        auto res = process(std::move(message));
        if (res) {
            ret.push_back(std::move(res));
        }
        return ret;
    }
    /** make the operator work like one
    @details calls the process function*/
    std::unique_ptr<Message> operator()(std::unique_ptr<Message> message)
    {
        return process(std::move(message));
    }
    /** indicator if the filter Operator has the capability of generating completely new messages or
     * redirecting messages*/
    virtual bool isMessageGenerating() const { return false; }
};

/** special filter operator defining no operation the original message is simply returned
 */
class NullFilterOperator final: public FilterOperator {
  public:
    /**default constructor*/
    NullFilterOperator() = default;
    virtual std::unique_ptr<Message> process(std::unique_ptr<Message> message) override
    {
        return message;
    }
};

/** helper template to check whether an index is actually valid for a particular vector
@tparam SizedDataType a vector like data type that must have a size function
@param testSize an index to test
@param vec a reference to a vector like object that has a size method
@return true if it is a valid index false otherwise*/
template<class sizeType, class SizedDataType>
inline bool isValidIndex(sizeType testSize, const SizedDataType& vec)
{
    return ((testSize >= sizeType(0)) && (testSize < static_cast<sizeType>(vec.size())));
}

}  // namespace helics
