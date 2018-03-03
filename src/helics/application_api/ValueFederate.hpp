/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/
#pragma once

#include "../core/core-data.hpp"
#include "Federate.hpp"
#include "data_view.hpp"
#include "ValueConverter.hpp"
#include <functional>

namespace helics
{
/** @brief PIMPL design pattern with the implementation details for the ValueFederate*/
class ValueFederateManager;
/** class defining the value based interface */
class ValueFederate : public virtual Federate  // using virtual inheritance to allow combination federate
{
  public:
    /**constructor taking a federate information structure and using the default core
    @param[in] fi  a federate information structure
    */
    explicit ValueFederate (const FederateInfo &fi);
    /**constructor taking a core and a federate information structure, sore information in fi is ignored
    @param[in] core a shared ptr to a core to join
    @param[in] fi  a federate information structure
    */
    ValueFederate (const std::shared_ptr<Core> &core, const FederateInfo &fi);
    /**constructor taking a string with the required information
    @param[in] jsonString can be either a json file or a string containing json code
    */
    explicit ValueFederate (const std::string &jsonString);

    /** default constructor*/
    explicit ValueFederate ();

    /** special constructor called by child class to initialize the class vs the default constructor
    */
    explicit ValueFederate (bool res);

  public:
      /** federate is not copyable*/
    ValueFederate (const ValueFederate &fed) = delete;
    /** default move constructor*/
    ValueFederate (ValueFederate &&fed) noexcept;
    /** destructor*/
    virtual ~ValueFederate ();

    /** default move assignment*/
    ValueFederate &operator= (ValueFederate &&fed) noexcept;
    /** register a publication
    @details call is only valid in startup mode
    @param[in] key the name of the publication
    @param[in] type a string defining the type of the publication
    @param[in] units a string defining the units of the publication [optional]
    @return a publication id object for use as an identifier
    */
    publication_id_t registerPublication (const std::string &key,
                                          const std::string &type,
                                          const std::string &units = std::string ());
    /** register a publication
    @details call is only valid in startup mode by default prepends the name with the federate name
    @param[in] key the name of the publication
    @param[in] units  the optional units of the publication
    @return an identifier for use with this publication
    */
    template <typename X>
    publication_id_t registerPublication (const std::string &key, const std::string &units = std::string ())
    {
        return registerPublication (key, ValueConverter<X>::type (), units);
    }

    /** register a publication
    @details call is only valid in startup mode
    @param[in] key the name of the publication
    @param[in] type a string defining the type of the publication
    @param[in] units a string defining the units of the publication [optional]
    @return a publication id object for use as an identifier
    */
    publication_id_t registerGlobalPublication (const std::string &key,
                                                const std::string &type,
                                                const std::string &units = std::string ());
    /** register a publication
    @details call is only valid in startup mode by default prepends the name with the federate name
    @param[in] key the name of the publication
    @param[in] units  the optional units of the publication
    @return an identifier for use with this publication
    */
    template <typename X>
    publication_id_t registerGlobalPublication (const std::string &key, const std::string &units = std::string ())
    {
        return registerGlobalPublication (key, ValueConverter<X>::type (), units);
    }

    /** register a publication as part of an indexed structure
    @details call is only valid in startup mode by default prepends the name with the federate name
    the name is registered as a global structure with the index appended
    @param[in] key the name of the publication
    @param[in] index1 an index associated with the publication
    @param[in] units  the optional units of the publication
    @return an identifier for use with this publication
    */
    template <typename X>
    publication_id_t
    registerPublicationIndexed (const std::string &key, int index1, const std::string &units = std::string ())
    {
        return registerGlobalPublication<X> (key + '_' + std::to_string (index1), units);
    }
    /** register a publication as part of a 2 dimensional indexed structure
    @details call is only valid in startup mode by default prepends the name with the federate name
    the name is registered as a global structure with the indices appended
    @param[in] key the name of the publication
    @param[in] index1 an index associated with the publication
    @param[in] index2 a second index
    @param[in] units  the optional units of the publication
    @return an identifier for use with this publication
    */
    template <typename X>
    publication_id_t registerPublicationIndexed (const std::string &key,
                                                 int index1,
                                                 int index2,
                                                 const std::string &units = std::string ())
    {
        return registerGlobalPublication<X> (key + '_' + std::to_string (index1) + '_' + std::to_string (index2),
                                             units);
    }

    /** register a subscription
    @details call is only valid in startup mode register a subscription with name type and units
    @param[in] key the name of the publication to subscribe to
    @param[in] type a string describing the type of the publication
    @param[in] units a string describing the units on the publication
    */
    subscription_id_t registerRequiredSubscription (const std::string &key,
                                                    const std::string &type,
                                                    const std::string &units = std::string ());
    /** register a subscription
    @details call is only valid in startup mode
    */
    template <typename X>
    subscription_id_t
    registerRequiredSubscription (const std::string &name, const std::string &units = std::string ())
    {
        return registerRequiredSubscription (name, ValueConverter<X>::type (), units);
    }

    /** register a required subscription
    @details call is only valid in startup mode, register an optional subscription for a 1D array of values
    @param[in] key the name of the subscription
    @param[in] index1 the index into a 1 dimensional array of values
    @param[in] units the optional units on the subscription
    */
    template <typename X>
    subscription_id_t registerRequiredSubscriptionIndexed (const std::string &key,
                                                           int index1,
                                                           const std::string &units = std::string ())
    {
        return registerRequiredSubscription<X> (key + '_' + std::to_string (index1), units);
    }

    /** register a required subscription for a 2-D array of values
    @details call is only valid in startup mode
    @param[in] key the name of the subscription
    @param[in] index1 the first index of a 2-D value structure
    @param[in] index2 the 2nd index of a 2-D value structure
    @param[in] units the optional units on the subscription
    */
    template <typename X>
    subscription_id_t registerRequiredSubscriptionIndexed (const std::string &key,
                                                           int index1,
                                                           int index2,
                                                           const std::string &units = std::string ())
    {
        return registerRequiredSubscription<X> (key + '_' + std::to_string (index1) + '_' +
                                                  std::to_string (index2),
                                                units);
    }
    /** register a subscription
    @details call is only valid in startup mode
    @param[in] key the name of the publication to subscribe to
    @param[in] type the type of the subscription
    @param[in] units the units associated with the desired output
    */
    subscription_id_t registerOptionalSubscription (const std::string &key,
                                                    const std::string &type,
                                                    const std::string &units = std::string ());
    /** register a subscription
    @details call is only valid in startup mode
    @param[in] key the name of the subscription
    @param[in] units the optional units on the subscription
    */
    template <typename X>
    subscription_id_t
    registerOptionalSubscription (const std::string &key, const std::string &units = std::string ())
    {
        return registerOptionalSubscription (key, ValueConverter<X>::type (), units);
    }

    /** register an optional subscription
    @details call is only valid in startup mode, register an optional subscription for a 1D array of values
    @param[in] key the name of the subscription
    @param[in] index1 the index into a 1 dimensional array of values
    @param[in] units the optional units on the subscription
    */
    template <typename X>
    subscription_id_t registerOptionalSubscriptionIndexed (const std::string &key,
                                                           int index1,
                                                           const std::string &units = std::string ())
    {
        return registerOptionalSubscription<X> (key + '_' + std::to_string (index1), units);
    }

    /** register an optional subscription for a 2-D array of values
    @details call is only valid in startup mode
    @param[in] key the name of the subscription
    @param[in] index1 the first index of a 2-D value structure
    @param[in] index2 the 2nd index of a 2-D value structure
    @param[in] units the optional units on the subscription
    */
    template <typename X>
    subscription_id_t registerOptionalSubscriptionIndexed (const std::string &key,
                                                           int index1,
                                                           int index2,
                                                           const std::string &units = std::string ())
    {
        return registerOptionalSubscription<X> (key + '_' + std::to_string (index1) + '_' +
                                                  std::to_string (index2),
                                                units);
    }
    /** add a shortcut for locating a subscription
    @details primarily for use in looking up an id from a different location
    creates a local shortcut for referring to a subscription which may have a long actual name
    @param[in] the subscription identifier
    @param[in] shortcutName the name of the shortcut
    */
    void addSubscriptionShortcut (subscription_id_t subid, const std::string &shortcutName);

    /** set the default value for a subscription
    @details this is the value returned prior to any publications
    @param[in] id the subscription identifier
    @param[in] block the data view representing the default value
    @throw std::invalid_argument if id is invalid
    */
    void setDefaultValue (subscription_id_t id, data_view block);
    /** set the default value for a subscription
    @details this is the value returned prior to any publications
    @param[in] id the subscription identifier
    @param[in] block the data block representing the default value
    @throw std::invalid_argument if id is invalid
    */
    void setDefaultValue (subscription_id_t id, const data_block &block)
    {
        setDefaultValue (id, data_view (block));
    }

    /** set a default value for a subscription
    @param[in] id  the identifier for the subscription
    @param[in] val the default value
    @throw std::invalid_argument if id is invalid
    */
    template <typename X>
    void setDefaultValue (subscription_id_t id, const X &val)
    {
        setDefaultValue (id, data_view (ValueConverter<X>::convert (val)));
    }
    /** register a set of interfaces defined in a file
    @details call is only valid in startup mode
    @param[in] jsonString  the location of the file to load to generate the interfaces
    */
    virtual void registerInterfaces (const std::string &jsonString) override;

    /** register a set of value interfaces (publications and subscriptions)
    @details call is only valid in startup mode it is a protected call to add an
    @param[in] jsonString  the location of the file or json String to load to generate the interfaces
    */
    void registerValueInterfaces (const std::string &jsonString);

    /** get a value as raw data block from the system
    @param[in] id the identifier for the subscription
    @return a constant data block
    @throw std::invalid_argument if id is invalid
    */
    data_view getValueRaw (subscription_id_t id);

    /** get a value as raw data block from the system
    @param[in] id the identifier for the subscription
    @param[out] the value translated to a specific object
    @return a constant data block
    @throw std::invalid_argument if id is invalid
    */
    template <typename X>
    void getValue (subscription_id_t id, X &obj)
    {
        ValueConverter<X>::interpret (getValueRaw (id), obj);
    }
    /** get a value as raw data block from the system
    @param[in] id the identifier for the subscription
    @param[out] the value translated to a specific object
    @return a constant data block
    @throw std::invalid_argument if id is invalid
    */
    template <typename X>
    X getValue (subscription_id_t id)
    {
        return ValueConverter<X>::interpret (getValueRaw (id));
    }
    /** publish a value
    @param[in] id the publication identifier
    @param[in] a data block containing the data
    @throw invalid_argument if the publication id is invalid
    */
    void publish (publication_id_t id, data_view block);

    /** publish a data block
    @details this function is primarily to prevent data blocks from falling through to the template
    @param[in] id the publication identifier
    @param[in] a data block containing the data
    @throw invalid_argument if the publication id is invalid
    */
    void publish (publication_id_t id, const data_block &block) { publish (id, data_view (block)); }

    /** publish a string
    @param[in] id the publication identifier
    @param[in] data a const char pointer to a string
    @throw invalid_argument if the publication id is invalid
    */
    void publish (publication_id_t id, const char *data) { publish (id, data_view{data, strlen (data)}); }

    /** publish data
    @param[in] id the publication identifier
    @param[in] data a const char pointer to raw data
    @param[in] data_size the length of the data
    @throw invalid_argument if the publication id is invalid
    */
    void publish (publication_id_t id, const char *data, size_t data_size)
    {
        publish (id, data_view{data, data_size});
    }

    /** publish a value
    @tparam X the type of the value to publish
    @param[in] id the publication identifier
    @param[in] value a reference to a value holding the data
    @throw invalid_argument if the publication id is invalid
    */
    template <typename X>
    void publish (publication_id_t id, const X &value)
    {
        publish (id, data_view (ValueConverter<X>::convert (value)));
    }

    /** check if a given subscription has an update
    @return true if the subscription id is valid and has an update*/
    bool isUpdated (subscription_id_t sub_id) const;
    /** get the time of the last update*/
    Time getLastUpdateTime (subscription_id_t sub_id) const;

    virtual void disconnect () override;

  protected:
    virtual void updateTime (Time newTime, Time oldTime) override;
    virtual void startupToInitializeStateTransition () override;
    virtual void initializeToExecuteStateTransition () override;

  public:
    /** get a list of all the values that have been updated since the last call
    @return a vector of subscription_ids with all the values that have not been retrieved since updated
    */
    std::vector<subscription_id_t> queryUpdates ();

    /** get the key or the string identifier of a subscription from its id
    @return empty string if an invalid id is passed*/
    std::string getSubscriptionKey (subscription_id_t) const;
    /** get the id of a subscription
    @return ivalid_subscription_id if name is not a recognized*/
    subscription_id_t getSubscriptionId (const std::string &key) const;
    /** get the id of a subscription from a vector of subscriptions
    @return ivalid_subscription_id if name is not a recognized*/
    subscription_id_t getSubscriptionId (const std::string &key, int index1) const;
    /** get the id of a subscription from a 2-d vector of subscriptions
    @return ivalid_subscription_id if name is not a recognized*/
    subscription_id_t getSubscriptionId (const std::string &key, int index1, int index2) const;
    /** get the name of a publication from its id
    @return empty string if an invalid id is passed*/
    std::string getPublicationKey (publication_id_t) const;

    /** get the id of a registered publication from its id
    @param[in] name the name of the publication
    @return ivalid_publication_id if name is not recognized otherwise returns the publication_id*/
    publication_id_t getPublicationId (const std::string &key) const;

    /** get the id of a registered publication from its id
    @param[in] name the name of the publication
    @return ivalid_publication_id if name is not recognized otherwise returns the publication_id*/
    publication_id_t getPublicationId (const std::string &key, int index1) const;
    /** get the id of a registered publication from its id
    @param[in] name the name of the publication
    @return ivalid_publication_id if name is not recognized otherwise returns the publication_id*/
    publication_id_t getPublicationId (const std::string &key, int index1, int index2) const;

    /** get the units of a subscriptions from its id
    @param[in] id the subscription id to query
    @return the name or empty string on unrecognized id*/
    std::string getSubscriptionUnits (subscription_id_t id) const;

    /** get the units of a publication from its id
    @param[in] id the publication id to query
    @return the units or empty string on unrecognized id*/
    std::string getPublicationUnits (publication_id_t id) const;

    /** get the type of a subscription from its id
    @param[in] id the subscription id to query
    @return the type or empty string on unrecognized id*/
    std::string getSubscriptionType (subscription_id_t id) const;

    /** get the type of a publication from its id
    @param[in] id the publication id to query
    @return the type or empty string on unrecognized id*/
    std::string getPublicationType (publication_id_t id) const;

    /** get the type of the publication of a particular subscription
    @param[in] id the subscription id to query
    @return the type or empty string on unrecognized id*/
    std::string getPublicationType (subscription_id_t id) const;

    /** register a callback function to call when any subscribed value is updated
    @details there can only be one generic callback
    @param[in] callback the function to call signature void(subscription_id_t, Time)
    */
    void registerSubscriptionNotificationCallback (std::function<void(subscription_id_t, Time)> callback);
    /** register a callback function to call when the specified subscription is updated
    @param[in] id  the id to register the callback for
    @param[in] callback the function to call
    */
    void registerSubscriptionNotificationCallback (subscription_id_t id,
                                                   std::function<void(subscription_id_t, Time)> callback);
    /** register a callback function to call when the specified subscription is updated
    @param[in] ids  the set of ids to register the callback for
    @param[in] callback the function to call
    */
    void registerSubscriptionNotificationCallback (const std::vector<subscription_id_t> &ids,
                                                   std::function<void(subscription_id_t, Time)> callback);

    /** get a count of the number publications registered*/
    int getPublicationCount () const;
    /** get a count of the number subscriptions registered*/
    int getSubscriptionCount () const;

  private:
    /** @brief PIMPL design pattern with the implementation details for the ValueFederate*/
    std::unique_ptr<ValueFederateManager> vfManager;
};

/** publish directly from the publication key name
@details this is a convenience function to publish directly from the publication key
@param fed a shared_ptr to a value federate
@param pubKey  the name of the publication
@tparam pargs any combination of arguments that go into the other publish commands
*/
template <class... Us>
void publish (std::shared_ptr<ValueFederate> &fed, const std::string &pubKey, Us... pargs)
{
    fed->publish (fed->getPublicationId (pubKey), pargs...);
}

/** publish directly from the publication key name
@details this is a convenience function to publish directly from the publication key
this function should not be used as the primary means of publications as it does involve an additional map find
operation vs the member publish calls
@param fed a reference to a valueFederate
@param pubKey  the name of the publication
@tparam pargs any combination of arguments that go into the other publish commands
*/
template <class... Us>
void publish (ValueFederate &fed, const std::string &pubKey, Us... pargs)
{
    fed.publish (fed.getPublicationId (pubKey), pargs...);
}

/** get a value directly from the subscription key name
@details this is a convenience function to get a value directly from the subscription key name
this function should not be used as the primary means of retrieving value as it does involve an additional map find
operation vs the member getValue calls
@param fed a shared pointer to a valueFederate
@param key  the name of the publication
*/
template <class X>
X getValue (std::shared_ptr<ValueFederate> &fed, const std::string &Key)
{
    return fed->getValue<X> (fed->getSubscriptionId (Key));
}

/** get a value directly from the subscription key name
@details this is a convenience function to get a value directly from the subscription key name
this function should not be used as the primary means of retrieving value as it does involve an additional map find
operation vs the member getValue calls
@param fed a shared pointer to a valueFederate
@param key  the name of the publication
@param obj the obj to store the retrieved value
*/
template <class X>
void getValue (std::shared_ptr<ValueFederate> &fed, const std::string &Key, X &obj)
{
    obj=fed->getValue<X> (fed->getSubscriptionId (Key));
}

/** get a value directly from the subscription key name
@details this is a convenience function to get a value directly from the subscription key name
this function should not be used as the primary means of retrieving value as it does involve an additional map find
operation vs the member getValue calls
@param fed a reference to a valueFederate
@param key  the name of the publication
*/
template <class X>
X getValue (ValueFederate &fed, const std::string &Key)
{
    return fed.getValue<X> (fed.getSubscriptionId (Key));
}

/** get a value directly from the subscription key name
@details this is a convenience function to get a value directly from the subscription key name
this function should not be used as the primary means of retrieving value as it does involve an additional map find
operation vs the member getValue calls
@param fed a reference to a valueFederate
@param key  the name of the publication
@param obj the obj to store the retrieved value
*/
template <class X>
void getValue (ValueFederate &fed, const std::string &Key, X &obj)
{
    obj=fed.getValue<X> (fed.getSubscriptionId (Key));
}
}  // namespace helics