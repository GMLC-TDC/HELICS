/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "SmallBuffer.hpp"
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

/** class containing a message structure*/
class Message {
  public:
    Time time = timeZero;  //!< the event time the message is sent
    std::uint16_t flags{0};  //!< message flags
    std::uint16_t messageValidation{0U};  //!< extra field for user object usage, not used by HELICS
    std::int32_t messageID{0};  //!< the messageID for a message
    SmallBuffer data;  //!< the data packet for the message
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
    std::string_view to_string() const { return data.to_string(); }
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
