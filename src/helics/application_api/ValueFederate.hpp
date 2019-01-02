/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../core/core-data.hpp"
#include "Federate.hpp"
#include "ValueConverter.hpp"
#include "data_view.hpp"
#include <functional>

namespace helics
{
class Publication;
class Input;
/** @brief PIMPL design pattern with the implementation details for the ValueFederate*/
class ValueFederateManager;
/** class defining the value based interface */
class ValueFederate : public virtual Federate  // using virtual inheritance to allow combination federate
{
  public:
    /**constructor taking a federate information structure and using the default core
    @param[in] fi  a federate information structure
    */
    ValueFederate (const std::string &fedName, const FederateInfo &fi);
    /**constructor taking a core and a federate information structure, sore information in fi is ignored
    @param[in] core a shared ptr to a core to join
    @param[in] fi  a federate information structure
    */
    ValueFederate (const std::string &fedName, const std::shared_ptr<Core> &core, const FederateInfo &fi);
    /**constructor taking a string with the required information
    @param[in] configString can be either a JSON file a TOML file (with extension TOML) or a string containing JSON
    code
    */
    explicit ValueFederate (const std::string &configString);
    /**constructor taking a string with the required information
    @param[in] configString can be either a JSON file a TOML file (with extension TOML) or a string containing JSON
    code
    */
    ValueFederate (const std::string &fedName, const std::string &configString);

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
    Publication &registerPublication (const std::string &key,
                                      const std::string &type,
                                      const std::string &units = std::string ());
    /** register a publication
    @details call is only valid in startup mode by default prepends the name with the federate name
    @param[in] key the name of the publication
    @param[in] units  the optional units of the publication
    @return an identifier for use with this publication
    */
    template <typename X>
    Publication &registerPublication (const std::string &key, const std::string &units = std::string ())
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
    Publication &registerGlobalPublication (const std::string &key,
                                            const std::string &type,
                                            const std::string &units = std::string ());
    /** register a publication
    @details call is only valid in startup mode by default prepends the name with the federate name
    @param[in] key the name of the publication
    @param[in] units  the optional units of the publication
    @return an identifier for use with this publication
    */
    template <typename X>
    Publication &registerGlobalPublication (const std::string &key, const std::string &units = std::string ())
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
    Publication &
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
    Publication &registerPublicationIndexed (const std::string &key,
                                             int index1,
                                             int index2,
                                             const std::string &units = std::string ())
    {
        return registerGlobalPublication<X> (key + '_' + std::to_string (index1) + '_' + std::to_string (index2),
                                             units);
    }

    /** register an input
    @details call is only valid in startup mode register a subscription with name type and units
    @param[in] key the name of the publication to subscribe to
    @param[in] type a string describing the type of the publication
    @param[in] units a string describing the units on the publication
    */
    Input &
    registerInput (const std::string &key, const std::string &type, const std::string &units = std::string ());

    /** register a globally named input
    @details call is only valid in startup mode
    @param[in] key the name of the input(can be blank in which case it is the same as a subscription
    @param[in] type a string defining the type of the input
    @param[in] units a string defining the units of the input [optional]
    @return a input id object for use as an identifier
    */
    Input &registerGlobalInput (const std::string &key,
                                const std::string &type,
                                const std::string &units = std::string ());
    /** register a named input
     */
    template <typename X>
    Input &registerInput (const std::string &key, const std::string &units = std::string ())
    {
        return registerInput (key, ValueConverter<X>::type (), units);
    }
    /** register a global named input
     */
    template <typename X>
    Input &registerGlobalInput (const std::string &key, const std::string &units = std::string ())
    {
        return registerGlobalInput (key, ValueConverter<X>::type (), units);
    }

    /** register a required subscription
    @details call is only valid in startup mode, register an optional subscription for a 1D array of values
    @param[in] key the name of the subscription
    @param[in] index1 the index into a 1 dimensional array of values
    @param[in] units the optional units on the subscription
    */
    template <typename X>
    Input &registerInputIndexed (const std::string &key, int index1, const std::string &units = std::string ())
    {
        return registerGlobalInput<X> (key + '_' + std::to_string (index1), units);
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
    Input &registerInputIndexed (const std::string &key,
                                 int index1,
                                 int index2,
                                 const std::string &units = std::string ())
    {
        return registerGlobalInput<X> (key + '_' + std::to_string (index1) + '_' + std::to_string (index2), units);
    }

    /** register a subscription
    @param[in] target the name of the publication to subscribe to
    @param[in] type the type of the subscription
    @param[in] units the units associated with the desired output
    */
    Input &registerSubscription (const std::string &target, const std::string &units = std::string ());

    /** register a subscription
    @details register a subscription for a 1D array of values
    @param[in] key the name of the subscription
    @param[in] index1 the index into a 1 dimensional array of values
    @param[in] units the optional units on the subscription
    */
    Input &
    registerSubscriptionIndexed (const std::string &target, int index1, const std::string &units = std::string ())
    {
        return registerSubscription (target + '_' + std::to_string (index1), units);
    }

    /** register a subscription for an index of a 2-D array of values
    @details call is only valid in startup mode
    @param[in] key the name of the subscription
    @param[in] index1 the first index of a 2-D value structure
    @param[in] index2 the 2nd index of a 2-D value structure
    @param[in] units the optional units on the subscription
    */
    Input &registerSubscriptionIndexed (const std::string &target,
                                        int index1,
                                        int index2,
                                        const std::string &units = std::string ())
    {
        return registerSubscription (target + '_' + std::to_string (index1) + '_' + std::to_string (index2),
                                     units);
    }
    /** add a shortcut for locating an input
    @details primarily for use in looking up an id from a different location
    creates a local shortcut for referring to a input which may not have another name
    @param[in] inp the input object
    @param[in] shortcutName the name of the shortcut
    */
    void addAlias (const Input &inp, const std::string &shortcutName);

    /** add a shortcut for locating a publication
    @details primarily for use in looking up an id from a different location
    creates a local shortcut for referring to a publication which may not have another name
    @param[in] pub the publication object
    @param[in] shortcutName the name of the shortcut
    */
    void addAlias (const Publication &pub, const std::string &shortcutName);

    /** set the default value for a subscription
    @details this is the value returned prior to any publications
    @param[in] id the subscription identifier
    @param[in] block the data view representing the default value
    @throw std::invalid_argument if id is invalid
    */
    void setDefaultValue (const Input &inp, data_view block);

    /** register a set of interfaces defined in a file
    @details call is only valid in startup mode to add an TOML files must have extension .toml or .TOML
    @param[in] configString  the location of the file(JSON or TOML) or JSON String to load to generate the
    interfaces
    */
    virtual void registerInterfaces (const std::string &configString) override;

    /** register a set of value interfaces (publications and subscriptions)
    @details call is only valid in startup mode it is a protected call to add an TOML files must have extension
    .toml or .TOML
    @param[in] configString  the location of the file(JSON or TOML) or JSON String to load to generate the
    interfaces
    */
    void registerValueInterfaces (const std::string &configString);

  private:
    void registerValueInterfacesJson (const std::string &jsonString);
    void registerValueInterfacesToml (const std::string &tomlString);

  public:
    /** get a value as raw data block from the system
    @param[in] id the identifier for the subscription
    @return a constant data block
    @throw std::invalid_argument if id is invalid
    */
    data_view getValueRaw (const Input &inp);

    /** get a double value*/
    double getDouble (Input &inp);
    /** get a string value*/
    const std::string &getString (Input &inp);
    /** publish a value
    @param[in] id the publication identifier
    @param[in] a data block containing the data
    @throw invalid_argument if the publication id is invalid
    */
    void publishRaw (const Publication &pub, data_view block);

    /** publish data
  @param[in] id the publication identifier
  @param[in] data a const char pointer to raw data
  @param[in] data_size the length of the data
  @throw invalid_argument if the publication id is invalid
  */
    void publishRaw (const Publication &pub, const char *data, size_t data_size)
    {
        publishRaw (pub, data_view{data, data_size});
    }

    /** publish a string
@param[in] id the publication identifier
@param[in] data a const char pointer to a string
@throw invalid_argument if the publication id is invalid
*/
    void publish (Publication &pub, const std::string &str);

    /** publish a double
 @param[in] id the publication identifier
 @param[in] val the value to publish
 @throw invalid_argument if the publication id is invalid
 */
    void publish (Publication &pub, double val);

    /** add a destination target to a publication
    @param pub the publication object to add a target to
    target the name of the input to send the data to
    */
    void addTarget (const Publication &pub, const std::string &target);
    /** add a source target to an input/subscription
    @param inp the input object to add a named publication
    target the name of the publication to get data from
    */
    void addTarget (const Input &inp, const std::string &target);
    /** remove a destination target from a publication
    @param pub the publication object to add a target to
    target the name of the input to remove
    */
    void removeTarget (const Publication &pub, const std::string &target);
    /** remove a publication from an input/subscription
    @param inp the input object to add a named publication
    target the name of the publication to remove
    */
    void removeTarget (const Input &inp, const std::string &target);
    /** register an optional subscription
   @details call is only valid in startup mode, register an optional subscription for a 1D array of values
   @param[in] target the name of the target
   @param[in] index1 the index into a 1 dimensional array of values
   @param[in] units the optional units on the subscription
   */
    template <class iType>
    void addTargetIndexed (const iType &id,
                           const std::string &target,
                           int index1,
                           const std::string &units = std::string ())
    {
        addTarget (id, target + '_' + std::to_string (index1), units);
    }

    /** register an optional subscription for a 2-D array of values
    @details call is only valid in startup mode
    @param[in] key the name of the subscription
    @param[in] index1 the first index of a 2-D value structure
    @param[in] index2 the 2nd index of a 2-D value structure
    @param[in] units the optional units on the subscription
    */
    template <class iType>
    void addTargetIndexed (const iType &id,
                           const std::string &target,
                           int index1,
                           int index2,
                           const std::string &units = std::string ())
    {
        addTarget (id, target + '_' + std::to_string (index1) + '_' + std::to_string (index2), units);
    }

    /** check if a given subscription has an update
    @return true if the subscription id is valid and has an update*/
    bool isUpdated (const Input &inp) const;
    /** get the time of the last update*/
    Time getLastUpdateTime (const Input &inp) const;

    virtual void disconnect () override;

  protected:
    virtual void updateTime (Time newTime, Time oldTime) override;
    virtual void startupToInitializeStateTransition () override;
    virtual void initializeToExecuteStateTransition () override;
    virtual std::string localQuery (const std::string &queryStr) const override;

  public:
    /** get a list of all the values that have been updated since the last call
    @return a vector of subscription_ids with all the values that have not been retrieved since updated
    */
    std::vector<int> queryUpdates ();

    /** get the key or the string identifier of an from its id
    @return empty string if an invalid id is passed*/
    const std::string &getTarget (const Input &inp) const;
    /** get the id of a subscription
    @return ivalid_subscription_id if name is not a recognized*/
    const Input &getInput (const std::string &name) const;
    /** get the id of a subscription
    @return ivalid_subscription_id if name is not a recognized*/
    Input &getInput (const std::string &name);
    /** get the id of a subscription
    @return ivalid_subscription_id if name is not a recognized*/
    const Input &getInput (int index) const;
    /** get the id of a subscription
    @return ivalid_subscription_id if name is not a recognized*/
    Input &getInput (int index);
    /** get the id of a subscription from a vector of subscriptions
    @return ivalid_subscription_id if name is not a recognized*/
    const Input &getInput (const std::string &name, int index1) const;
    /** get the id of a subscription from a 2-d vector of subscriptions
    @return ivalid_subscription_id if name is not a recognized*/
    const Input &getInput (const std::string &name, int index1, int index2) const;

    /** get the input id based on target
    @return an input_id_t from the object, or invalid_id if no input was found
    */
    const Input &getSubscription (const std::string &key) const;

    /** get the input id based on target
    @return an input_id_t from the object, or invalid_id if no input was found
    */
    Input &getSubscription (const std::string &key);

    /** get the id of a registered publication from its id
    @param[in] name the name of the publication
    @return ivalid_publication_id if name is not recognized otherwise returns the publication_id*/
    Publication &getPublication (const std::string &key);
    /** get the id of a registered publication from its id
    @param[in] name the name of the publication
    @return ivalid_publication_id if name is not recognized otherwise returns the publication_id*/
    const Publication &getPublication (const std::string &key) const;
    /** get the id of a registered publication from its id
   @param[in] name the name of the publication
   @return ivalid_publication_id if name is not recognized otherwise returns the publication_id*/
    Publication &getPublication (int index);
    /** get the id of a registered publication from its id
    @param[in] name the name of the publication
    @return ivalid_publication_id if name is not recognized otherwise returns the publication_id*/
    const Publication &getPublication (int index) const;

    /** get the id of a registered publication from its id
    @param[in] name the name of the publication
    @return ivalid_publication_id if name is not recognized otherwise returns the publication_id*/
    const Publication &getPublication (const std::string &key, int index1) const;
    /** get the id of a registered publication from its id
    @param[in] name the name of the publication
    @return ivalid_publication_id if name is not recognized otherwise returns the publication_id*/
    const Publication &getPublication (const std::string &key, int index1, int index2) const;

    /** register a callback function to call when any subscribed value is updated
    @details there can only be one generic callback
    @param[in] callback the function to call signature void(input_id_t, Time)
    */
    void setInputNotificationCallback (std::function<void(Input &, Time)> callback);
    /** register a callback function to call when the specified subscription is updated
    @param[in] id  the id to register the callback for
    @param[in] callback the function to call
    */
    void setInputNotificationCallback (Input &inp, std::function<void(Input &, Time)> callback);

    /** get a count of the number publications registered*/
    int getPublicationCount () const;
    /** get a count of the number subscriptions registered*/
    int getInputCount () const;

  private:
    /** @brief PIMPL design pattern with the implementation details for the ValueFederate*/
    std::unique_ptr<ValueFederateManager> vfManager;
};

}  // namespace helics
