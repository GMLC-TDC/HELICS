/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "ValueFederate.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <utility>
#include <vector>

/** @file
@details helper function for generate subscriptions of specify types
*/

namespace helics {

/** class to handle a Vector Subscription
@tparam X the class of the value associated with the vector subscription
*/
template<class X>
class VectorSubscription {
  private:
    ValueFederate* fed = nullptr;  //!< reference to the value federate
    std::string m_key;  //!< the key for the subscription
    std::string m_units;  //!< the defined units of the federate
    std::vector<Input> ids;  //!< the id of the federate
    std::function<void(int, Time)>
        update_callback;  //!< callback function for when a value is updated
    std::vector<X> vals;  //!< storage for the values
  public:
    VectorSubscription() noexcept {};
    /**constructor to build a subscription object
   @param valueFed  the ValueFederate to use
   @param key the identifier for the publication to subscribe to
   @param startIndex the index to start with
   @param count the number of values to subscribe to
   @param defValue the default value
   @param units the units associated with the Subscription
   */
    VectorSubscription(ValueFederate* valueFed,
                       std::string_view key,
                       int startIndex,
                       int count,
                       const X& defValue,
                       std::string_view units = std::string_view{}):
        fed(valueFed), m_key(key), m_units(units)
    {
        ids.reserve(count);
        vals.resize(count, defValue);

        for (auto ind = startIndex; ind < startIndex + count; ++ind) {
            auto id = fed->registerIndexedSubscription(m_key, ind, m_units);
            ids.push_back(id);
        }

        for (auto& id : ids) {
            fed->setInputNotificationCallback(id, [this](Input& inp, Time tm) {
                handleCallback(inp, tm);
            });
        }
    }
    /**constructor to build a subscription object
    @param valueFed  the ValueFederate to use
    @param key the identifier for the publication to subscribe to
    @param startIndex the index to start with
    @param count the number of values to subscribe to
    @param defValue the default value
    @param units the units associated with the Subscription
    */
    template<class FedPtr>
    VectorSubscription(FedPtr valueFed,
                       std::string_view key,
                       int startIndex,
                       int count,
                       const X& defValue,
                       std::string_view units = std::string_view{}):
        VectorSubscription(std::addressof(*valueFed), key, startIndex, count, defValue, units)
    {
        static_assert(
            std::is_base_of<ValueFederate, std::remove_reference_t<decltype(*valueFed)>>::value,
            "first argument must be a pointer to a ValueFederate");
    }

    /** move constructor*/
    VectorSubscription(VectorSubscription&& vs) noexcept:
        fed(vs.fed), m_key(std::move(vs.m_key)), m_units(std::move(vs.m_units)),
        ids(std::move(vs.ids)), update_callback(std::move(vs.update_callback)),
        vals(std::move(vs.vals))
    {
        // need to transfer the callback to the new object
        for (auto& id : ids) {
            fed->setInputNotificationCallback(id, [this](Input& inp, Time tm) {
                handleCallback(inp, tm);
            });
        }
    };
    /** move assignment*/
    VectorSubscription& operator=(VectorSubscription&& vs) noexcept
    {
        fed = vs.fed;
        m_key = std::move(vs.m_key);
        m_units = std::move(vs.m_units);
        ids = std::move(vs.ids);
        update_callback = std::move(vs.update_callback);
        vals = std::move(vs.vals);
        // need to transfer the callback to the new object
        for (auto& id : ids) {
            fed->setInputNotificationCallback(id, [this](Input& inp, Time tm) {
                handleCallback(inp, tm);
            });
        }
        return *this;
    }
    /** get the most recent value
    @return the value*/
    const std::vector<X>& getVals() const { return vals; }
    /**get a value through its index
     */
    const X& operator[](int index) const { return vals[index]; }
    /** register a callback for the update
    @details the callback is called in the just before the time request function returns
    @param callback a function with signature void(X val, Time time)
    val is the new value and time is the time the value was updated
    */
    void setInputNotificationCallback(std::function<void(int, Time)> callback)
    {
        update_callback = std::move(callback);
    }

  private:
    void handleCallback(Input& inp, Time time)
    {
        auto res = std::lower_bound(ids.begin(), ids.end(), inp);
        int index = static_cast<int>(res - ids.begin());
        vals[index] = inp.getValue<X>();
        if (update_callback) {
            update_callback(index, time);
        }
    }
};

/** class to handle a Vector Subscription
@tparam X the class of the value associated with the vector subscription*/
template<class X>
class VectorSubscription2d {
  private:
    ValueFederate* fed = nullptr;  //!< reference to the value federate
    std::string m_key;  //!< the name of the subscription
    std::string m_units;  //!< the defined units of the federate
    std::vector<Input> ids;  //!< the id of the federate
    std::function<void(int, Time)>
        update_callback;  //!< callback function for when a value is updated
    std::vector<X> vals;  //!< storage for the values
    std::array<int, 4> indices{{0, 0, 0, 0}};  //!< storage for the indices and start values
  public:
    VectorSubscription2d() noexcept {};

    /**constructor to build a subscription object
    @param valueFed  the ValueFederate to use
     @param key the identifier for the publication to subscribe to
    @param startIndex_x the index to start with in the x dimension
    @param count_x the number of values in the x direction
    @param startIndex_y the index to start with in the x dimension
    @param count_y the number of values in the x direction
    @param defValue the default value
    @param units the units associated with the Subscription
    */
    template<class FedPtr>
    VectorSubscription2d(FedPtr valueFed,
                         std::string_view key,
                         int startIndex_x,
                         int count_x,
                         int startIndex_y,
                         int count_y,
                         const X& defValue,
                         std::string_view units = std::string_view()):
        fed(std::addressof(*valueFed)), m_key(key), m_units(units)
    {
        static_assert(
            std::is_base_of<ValueFederate, std::remove_reference_t<decltype(*valueFed)>>::value,
            "Second argument must be a pointer to a ValueFederate");
        std::size_t arrayCount =
            static_cast<std::size_t>(count_x) * static_cast<std::size_t>(count_y);
        ids.reserve(arrayCount);
        vals.resize(arrayCount, defValue);

        for (auto ind_x = startIndex_x; ind_x < startIndex_x + count_x; ++ind_x) {
            for (auto ind_y = startIndex_y; ind_y < startIndex_y + count_y; ++ind_y) {
                auto id = fed->registerIndexedSubscription(m_key, ind_x, ind_y, m_units);
                ids.push_back(id);
            }
        }

        indices[0] = startIndex_x;
        indices[1] = count_x;
        indices[2] = startIndex_y;
        indices[3] = count_y;
        for (auto& id : ids) {
            fed->setInputNotificationCallback(id, [this](Input& inp, Time tm) {
                handleCallback(inp, tm);
            });
        }
    }

    /** move assignment*/
    VectorSubscription2d& operator=(VectorSubscription2d&& vs) noexcept
    {
        fed = vs.fed;
        m_key = std::move(vs.m_key);
        m_units = std::move(vs.m_units);
        ids = std::move(vs.ids);
        update_callback = std::move(vs.update_callback);
        vals = std::move(vs.vals);
        // need to transfer the callback to the new object
        for (auto& id : ids) {
            fed->setInputNotificationCallback(id, [this](Input& inp, Time tm) {
                handleCallback(inp, tm);
            });
        }
        indices = vs.indices;
        return *this;
    }
    /** get the most recent value
    @return the value*/
    const std::vector<X>& getVals() const { return vals; }
    /** get the value in the given variable
    @param index the location to retrieve the value for
    */
    const X& operator[](int index) const { return vals[index]; }

    /** get the value in the given variable at a particular 2d index
     */
    const X& at(int index_x, int index_y) const
    {
        return vals[(index_x - indices[0]) * indices[3] + (index_y - indices[2])];
    }
    /** register a callback for the update
    @details the callback is called in the just before the time request function returns
    @param callback a function with signature void(X val, Time time)
    val is the new value and time is the time the value was updated
    */
    void setInputNotificationCallback(std::function<void(int, Time)> callback)
    {
        update_callback = std::move(callback);
    }

  private:
    void handleCallback(const Input& inp, Time time)
    {
        auto res = std::lower_bound(ids.begin(), ids.end(), inp);
        int index = static_cast<int>(res - ids.begin());
        ids[index].getValue(vals[index]);
        if (update_callback) {
            update_callback(index, time);
        }
    }
};

}  // namespace helics
