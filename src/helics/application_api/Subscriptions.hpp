/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "Inputs.hpp"

namespace helics
{
/** primary subscription object class
@details can convert between the helics primary base class types
*/
class Subscription : public Input
{
  private:
    std::string target_;

  public:
    Subscription () = default;
    Subscription (ValueFederate *valueFed, std::string key, const std::string &units = std::string ())
        : Input (valueFed, std::string (), units), target_ (std::move (key))
    {
        addTarget (target_);
    }

    template <class FedPtr>
    Subscription (FedPtr &valueFed, std::string key, const std::string &units = std::string ())
        : Input (valueFed, std::string (), units), target_ (std::move (key))
    {
        addTarget (target_);
    }

    Subscription (ValueFederate *valueFed,
                  std::string key,
                  helics_type_t defType,
                  const std::string &units = std::string ())
        : Input (valueFed, std::string (), defType, units), target_ (std::move (key))
    {
        addTarget (target_);
    }

    template <class FedPtr>
    Subscription (FedPtr &valueFed,
                  std::string key,
                  helics_type_t defType,
                  const std::string &units = std::string ())
        : SubscriptionBase (valueFed, key, typeNameStringRef (defType), units), target_ (key)
    {
        addTarget (target_);
    }

    /** generate a subscription object from a preexisting subscription
    @param valueFed a pointer to the appropriate value Federate
    @param subIndex the index of the subscription
    */
    Subscription (ValueFederate *valueFed, int subIndex) : Input (valueFed, subIndex) {}
    /** get the target of a subscription*/
    const std::string &getTarget () const { return target_; }
};

/** class to handle a subscription
@tparam X the class of the value associated with a subscription*/
template <class X>
class SubscriptionT : public InputT<X>
{
  public:
  private:
    std::string target_;

  public:
    SubscriptionT () = default;
    /**constructor to build a subscription object
    @param[in] valueFed  the ValueFederate to use
    @param[in] name the name of the subscription
    @param[in] units the units associated with a Federate
    */
    SubscriptionT (ValueFederate *valueFed, std::string target, const std::string &units = std::string ())
        : InputT<X> (valueFed, std::string (), units), target_ (std::move (target))
    {
        addTarget (target_);
    }
    /**constructor to build a subscription object
    @param[in] valueFed  the ValueFederate to use
    @param[in] target the key of the field to subscribe to
    @param[in] units the units associated with a Federate
    */
    template <class FedPtr>
    SubscriptionT (FedPtr &valueFed, std::string target, const std::string &units = std::string ())
        : InputT<X> (valueFed, std::string, units), target_ (std::move (target))
    {
        addTarget (target_);
    }

    const std::string &getTarget () const { return target_; }
};

/** class to handle a Vector Subscription
@tparam X the class of the value associated with the vector subscription*/
template <class X>
class VectorSubscription : public InputT<X>
{
  private:
    std::string m_key;  //!< the key for the subscription
    std::function<void(int, Time)> update_callback;  //!< callback function for when a value is updated
  public:
    VectorSubscription () noexcept {};
    /**constructor to build a subscription object
   @param[in] valueFed  the ValueFederate to use
   @param[in] key the identifier for the publication to subscribe to
   @param[in] startIndex the index to start with
   @param[in] count the number of values to subscribe to
   @param[in] defValue the default value
   @param[in] units the units associated with the Subscription
   */
    VectorSubscription (ValueFederate *valueFed,
                        std::string key,
                        int startIndex,
                        int count,
                        const X &defValue,
                        const std::string &units = std::string ())
        : InputT<X> (valueFed, std::string (), units), m_key (std::move (key))
    {
        for (auto ind = startIndex; ind < startIndex + count; ++ind)
        {
            fed->addTarget (id, key, ind);
            ids.push_back (id);
        }

        fed->registerSubscriptionNotificationCallback (ids, [this](input_id_t id, Time tm) {
            handleCallback (id, tm);
        });
    }
    /**constructor to build a subscription object
    @param[in] valueFed  the ValueFederate to use
    @param[in] key the identifier for the publication to subscribe to
    @param[in] startIndex the index to start with
    @param[in] count the number of values to subscribe to
    @param[in] defValue the default value
    @param[in] units the units associated with the Subscription
    */
    template <class FedPtr>
    VectorSubscription (FedPtr valueFed,
                        const std::string &key,
                        int startIndex,
                        int count,
                        const X &defValue,
                        const std::string &units = std::string ())
        : VectorSubscription (std::addressof (*valueFed), key, startIndex, count, defValue, units)
    {
        static_assert (std::is_base_of<ValueFederate, std::remove_reference_t<decltype (*valueFed)>>::value,
                       "first argument must be a pointer to a ValueFederate");
    }

    /** move constructor*/
    VectorSubscription (VectorSubscription &&vs) noexcept
        : InputT<X> (std::move (vs)), target_ (std::move (vs.target))
    {
        // need to transfer the callback to the new object
        fed->registerSubscriptionNotificationCallback (id, [this](input_id_t id, Time tm) {
            handleCallback (id, tm);
        });
    };
    /** move assignment*/
    VectorSubscription &operator= (VectorSubscription &&vs) noexcept
    {
        target_ = std::move (vs.target);
        InputT<X>::operator= (std::move (vs));
        // need to transfer the callback to the new object
        fed->registerSubscriptionNotificationCallback (id, [this](input_id_t id, Time tm) {
            handleCallback (id, tm);
        });
        return *this;
    }

    /** register a callback for the update
    @details the callback is called in the just before the time request function returns
    @param[in] callback a function with signature void(X val, Time time)
    val is the new value and time is the time the value was updated
    */
    void registerCallback (std::function<void(int, Time)> callback) { update_callback = std::move (callback); }

  private:
    void handleCallback (input_id_t id, Time time)
    {
        X out;
        auto res = std::lower_bound (ids.begin (), ids.end (), id);
        int index = static_cast<int> (res - ids.begin ());
        fed->getValue (ids[index], out);
        vals[index] = out;
        if (update_callback)
        {
            update_callback (index, time);
        }
    }
};

/** class to handle a Vector Subscription
@tparam X the class of the value associated with the vector subscription*/
template <class X>
class VectorSubscription2d : public InputT<X>
{
  private:
    std::string m_key;  //!< the name of the subscription
    std::function<void(int, Time)> update_callback;  //!< callback function for when a value is updated
  public:
    VectorSubscription2d () noexcept {};

    /**constructor to build a subscription object
    @param[in] required a flag indicating that the subscription is required to have a matching publication
    @param[in] valueFed  the ValueFederate to use
     @param[in] key the identifier for the publication to subscribe to
    @param[in] startIndex_x the index to start with in the x dimension
    @param[in] count_x the number of values in the x direction
    @param[in] startIndex_y the index to start with in the x dimension
    @param[in] count_y the number of values in the x direction
    @param[in] defValue the default value
    @param[in] units the units associated with the Subscription
    */
    template <class FedPtr>
    VectorSubscription2d (FedPtr valueFed,
                          const std::string &key,
                          int startIndex_x,
                          int count_x,
                          int startIndex_y,
                          int count_y,
                          const X &defValue,
                          const std::string &units = std::string ())
        : InputT<X> (valueFed, std::string (), defValue, units), m_key (key)
    {
        static_assert (std::is_base_of<ValueFederate, std::remove_reference_t<decltype (*valueFed)>>::value,
                       "Second argument must be a pointer to a ValueFederate");

        for (auto ind_x = startIndex_x; ind_x < startIndex_x + count_x; ++ind_x)
        {
            for (auto ind_y = startIndex_y; ind_y < startIndex_y + count_y; ++ind_y)
            {
                fed->addTarget (m_key, ind_x, ind_y);
            }
        }

        fed->registerSubscriptionNotificationCallback (id, [this](input_id_t id, Time tm) {
            handleCallback (id, tm);
        });
    }

    /** move constructor*/
    VectorSubscription2d (VectorSubscription2d &&vs) noexcept
        : InputT<X> (std::move (vs)), target_ (std::move (vs.target))
    {
        // need to transfer the callback to the new object
        fed->registerSubscriptionNotificationCallback (id, [this](input_id_t id, Time tm) {
            handleCallback (id, tm);
        });
    };

    /** move assignment*/
    VectorSubscription2d &operator= (VectorSubscription2d &&vs) noexcept
    {
        target_ = std::move (vs.target);
        InputT<X>::operator= (std::move (vs));
        // need to transfer the callback to the new object
        fed->registerSubscriptionNotificationCallback (id, [this](input_id_t id, Time tm) {
            handleCallback (id, tm);
        });
        return *this;
    }

    /** get the value in the given variable
    @param[out] out the location to store the value
    */
    const X &at (int index_x, int index_y) const
    {
        return vals[(index_x - indices[0]) * indices[3] + (index_y - indices[2])];
    }
    /** register a callback for the update
    @details the callback is called in the just before the time request function returns
    @param[in] callback a function with signature void(X val, Time time)
    val is the new value and time is the time the value was updated
    */
    void registerCallback (std::function<void(int, Time)> callback) { update_callback = std::move (callback); }

  private:
    void handleCallback (input_id_t id, Time time)
    {
        auto res = std::lower_bound (ids.begin (), ids.end (), id);
        int index = static_cast<int> (res - ids.begin ());
        fed->getValue (ids[index], vals[index]);
        if (update_callback)
        {
            update_callback (index, time);
        }
    }
};

}  // namespace helics
