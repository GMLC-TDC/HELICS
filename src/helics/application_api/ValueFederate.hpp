/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../core/core-data.hpp"
#include "Federate.hpp"
#include "Inputs.hpp"
#include "Publications.hpp"
#include "ValueConverter.hpp"
#include "data_view.hpp"

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace helics {
/** @brief PIMPL design pattern with the implementation details for the ValueFederate*/
class ValueFederateManager;
/** class defining the value based interface */
class HELICS_CXX_EXPORT ValueFederate:
    public virtual Federate  // using virtual inheritance to allow combination federate
{
  public:
    /**constructor taking a federate information structure and using the default core
    @param fedName the name of the federate, can be empty to use the name from fedInfo or an auto
    generated one
    @param fedInfo  a federate information structure
    */
    ValueFederate(std::string_view fedName, const FederateInfo& fedInfo);

    /**constructor taking a core and a federate information structure, core information in fedInfo
    is ignored
    @param fedName the name of the federate, can be empty to use the name from fedInfo or an auto
    generated one
    @param core a shared ptr to a core to join
    @param fedInfo  a federate information structure
    */
    ValueFederate(std::string_view fedName,
                  const std::shared_ptr<Core>& core,
                  const FederateInfo& fedInfo = FederateInfo{});

    /**constructor taking a CoreApp and a federate information structure
    @param fedName the name of the federate can be empty to use a name from the federateInfo
    @param core a CoreApp with the core to connect to.
    @param fedInfo  a federate information structure
    */
    ValueFederate(std::string_view fedName,
                  CoreApp& core,
                  const FederateInfo& fedInfo = FederateInfo{});

    /**constructor taking a string with the required information
    @param configString can be either a JSON file a TOML file (with extension TOML) or a string
    containing JSON code
    */
    explicit ValueFederate(const std::string& configString);

    /**constructor taking a name and a string with the required information
    @param fedName the name of the federate, can be empty to use the name from the configString
    @param configString can be either a JSON file a TOML file (with extension TOML) or a string
    containing JSON code or a string with command line arguments
    */
    ValueFederate(std::string_view fedName, const std::string& configString);

    /** default constructor*/
    explicit ValueFederate();

    /** special constructor called by child class to initialize the class vs the default constructor
     */
    explicit ValueFederate(bool res);

    /** this is an overload for the string operation to deconflict with the bool version
     */
    explicit ValueFederate(const char* configString);

    /** federate is not copyable*/
    ValueFederate(const ValueFederate& fed) = delete;
    /** default move constructor*/
    ValueFederate(ValueFederate&& fed) noexcept;
    /** destructor*/
    virtual ~ValueFederate();

    /** default move assignment*/
    ValueFederate& operator=(ValueFederate&& fed) noexcept;
    /** delete copy assignment*/
    ValueFederate& operator=(const ValueFederate& fed) = delete;
    /** register a publication
    @details call is only valid in startup mode
    @param name the name of the publication
    @param type a string defining the type of the publication
    @param units a string defining the units of the publication [optional]
    @return a publication id object for use as an identifier
    */
    Publication& registerPublication(std::string_view name,
                                     std::string_view type,
                                     std::string_view units = std::string_view{});
    /** register a publication
    @details call is only valid in startup mode by default prepends the name with the federate name
    @param name the name of the publication
    @param units  the optional units of the publication
    @return an identifier for use with this publication
    */
    template<typename X>
    Publication& registerPublication(std::string_view name,
                                     std::string_view units = std::string_view{})
    {
        return registerPublication(name, ValueConverter<X>::type(), units);
    }

    /** register a publication
    @details call is only valid in startup mode
    @param name the name of the publication
    @param type a string defining the type of the publication
    @param units a string defining the units of the publication [optional]
    @return a publication object reference for use as an identifier
    */
    Publication& registerGlobalPublication(std::string_view name,
                                           std::string_view type,
                                           std::string_view units = std::string_view{});
    /** register a publication
    @details call is only valid in startup mode by default prepends the name with the federate name
    @param name the name of the publication
    @param units  the optional units of the publication
    @return an identifier for use with this publication
    */
    template<typename X>
    Publication& registerGlobalPublication(std::string_view name,
                                           std::string_view units = std::string_view{})
    {
        return registerGlobalPublication(name, ValueConverter<X>::type(), units);
    }

    /** register a publication as part of an indexed structure
    @details call is only valid in startup mode by default prepends the name with the federate name
    the name is registered as a global structure with the index appended
    @param name the name of the publication
    @param index1 an index associated with the publication
    @param units  the optional units of the publication
    @return an identifier for use with this publication
    */
    template<typename X>
    Publication& registerIndexedPublication(std::string_view name,
                                            int index1,
                                            std::string_view units = std::string_view{})
    {
        return registerGlobalPublication<X>(std::string(name) + '_' + std::to_string(index1),
                                            units);
    }
    /** register a publication as part of a 2 dimensional indexed structure
    @details call is only valid in startup mode by default prepends the name with the federate name
    the name is registered as a global structure with the indices appended
    @param name the name of the publication
    @param index1 an index associated with the publication
    @param index2 a second index
    @param units  the optional units of the publication
    @return an identifier for use with this publication
    */
    template<typename X>
    Publication& registerIndexedPublication(std::string_view name,
                                            int index1,
                                            int index2,
                                            std::string_view units = std::string_view{})
    {
        return registerGlobalPublication<X>(std::string(name) + '_' + std::to_string(index1) + '_' +
                                                std::to_string(index2),
                                            units);
    }

    /** register an input
    @details call is only valid in startup mode register a subscription with name type and units
    @param name the name of the publication to subscribe to
    @param type a string describing the type of the publication
    @param units a string describing the units on the publication
    */
    Input& registerInput(std::string_view name,
                         std::string_view type,
                         std::string_view units = std::string_view{});

    /** register a globally named input
    @details call is only valid in startup mode
    @param name the name of the input(can be blank in which case it is the same as a subscription
    @param type a string defining the type of the input
    @param units a string defining the units of the input [optional]
    @return a input id object for use as an identifier
    */
    Input& registerGlobalInput(std::string_view name,
                               std::string_view type,
                               std::string_view units = std::string_view{});
    /** register a named input
     */
    template<typename X>
    Input& registerInput(std::string_view name, std::string_view units = std::string_view{})
    {
        return registerInput(name, ValueConverter<X>::type(), units);
    }
    /** register a global named input
     */
    template<typename X>
    Input& registerGlobalInput(std::string_view name, std::string_view units = std::string_view{})
    {
        return registerGlobalInput(name, ValueConverter<X>::type(), units);
    }

    /** register a required subscription
    @details call is only valid in startup mode, register an optional subscription for a 1D array of
    values
    @param name the name of the subscription
    @param index1 the index into a 1 dimensional array of values
    @param units the optional units on the subscription
    */
    template<typename X>
    Input& registerIndexedInput(std::string_view name,
                                int index1,
                                std::string_view units = std::string_view())
    {
        return registerGlobalInput<X>(std::string(name) + '_' + std::to_string(index1), units);
    }

    /** register a publication as part of a 2 dimensional indexed structure
  @details call is only valid in startup mode by default prepends the name with the federate name
  the name is registered as a global structure with the indices appended
  @param name the name of the publication
  @param index1 an index associated with the publication
  @param index2 a second index
  @param units  the optional units of the publication
  @return an identifier for use with this publication
  */
    template<typename X>
    Input& registerIndexedInput(std::string_view name,
                                int index1,
                                int index2,
                                std::string_view units = std::string_view())
    {
        return registerGlobalInput<X>(std::string(name) + '_' + std::to_string(index1) + '_' +
                                          std::to_string(index2),
                                      units);
    }

    /** register a subscription
    @param target the name of the publication to subscribe to
    @param units the units associated with the desired output
    */
    Input& registerSubscription(std::string_view target,
                                std::string_view units = std::string_view{});

    /** register a subscription
    @details register a subscription for a 1D array of values
    @param target the name of the publication to target
    @param index1 the index into a 1 dimensional array of values
    @param units the optional units on the subscription
    */
    Input& registerIndexedSubscription(std::string_view target,
                                       int index1,
                                       std::string_view units = std::string_view{})
    {
        return registerSubscription(std::string(target) + '_' + std::to_string(index1), units);
    }

    /** register a subscription for an index of a 2-D array of values
    @details call is only valid in startup mode
    @param target the name of the publication to subscribe to
    @param index1 the first index of a 2-D value structure
    @param index2 the 2nd index of a 2-D value structure
    @param units the optional units on the subscription
    */
    Input& registerIndexedSubscription(std::string_view target,
                                       int index1,
                                       int index2,
                                       std::string_view units = std::string_view{})
    {
        return registerSubscription(std::string(target) + '_' + std::to_string(index1) + '_' +
                                        std::to_string(index2),
                                    units);
    }

    using Federate::addAlias;
    /** add a shortcut for locating an input
    @details primarily for use in looking up an id from a different location
    creates a local shortcut for referring to a input which may not have another name
    @param inp the input object
    @param shortcutName the name of the shortcut
    */
    void addAlias(const Input& inp, std::string_view shortcutName);

    /** add a shortcut for locating a publication
    @details primarily for use in looking up an id from a different location
    creates a local shortcut for referring to a publication which may not have another name
    @param pub the publication object
    @param shortcutName the name of the shortcut
    */
    void addAlias(const Publication& pub, std::string_view shortcutName);

    virtual void setFlagOption(int flag, bool flagValue = true) override;

    /** set the default value for a subscription
    @details this is the value returned prior to any publications
    @param inp the subscription identifier
    @param block the data view representing the default value
    @throw std::invalid_argument if id is invalid
    */
    void setDefaultValue(const Input& inp, data_view block);

    /** register a set of interfaces defined in a file
    @details call is only valid in startup mode to add an TOML files must have extension .toml or
    .TOML
    @param configString  the location of the file(JSON or TOML) or JSON String to load to generate
    the interfaces
    */
    virtual void registerInterfaces(const std::string& configString) override;

    /** register a set of value interfaces (publications and subscriptions)
    @details call is only valid in startup mode it is a protected call to add an TOML files must
    have extension .toml or .TOML
    @param configString  the location of the file(JSON or TOML) or JSON String to load to generate
    the interfaces
    */
    void registerValueInterfaces(const std::string& configString);

  private:
    void loadFederateData();

    /** register interfaces through a json file or string*/
    void registerValueInterfacesJson(const std::string& jsonString);
    /** details of the registration process*/
    void registerValueInterfacesJsonDetail(const fileops::JsonBuffer& json, bool defaultGlobal);
    /** register interface through a toml value or string*/
    void registerValueInterfacesToml(const std::string& tomlString);

  public:
    /** get a value as raw data block from the system
    @param inp an input object to get the data from
    @return a constant data block
    @throw std::invalid_argument if id is invalid
    */
    data_view getBytes(const Input& inp);

    /** force an input to get Data From the Core
    @param inp an input object to get the data from
    @return true if the value was registered as an update, which will be true in most cases
    @throw std::invalid_argument if id is invalid
    */
    bool forceCoreUpdate(Input& inp);

    /** publish a value
    @param pub the publication identifier
    @param block a data block containing the data
    @throw invalid_argument if the publication id is invalid
    */
    void publishBytes(const Publication& pub, data_view block);

    /** publish data
  @param pub the publication identifier
  @param data a const char pointer to raw data
  @param data_size the length of the data
  @throw invalid_argument if the publication id is invalid
  */
    void publishBytes(const Publication& pub, const char* data, size_t data_size)
    {
        publishBytes(pub, data_view{data, data_size});
    }

    /** register a set of publications based on a publication JSON
    @param jsonString a json string containing the data to publish and establish publications from
    */
    void registerFromPublicationJSON(const std::string& jsonString);

    /** publish a set of values in json format
     @param jsonString a json string containing the data to publish
     */
    void publishJSON(const std::string& jsonString);

    /** add a destination target to a publication
    @param pub the publication object to add a target to
    @param target the name of the input to send the data to
    */
    void addTarget(const Publication& pub, std::string_view target);
    /** add a source target to an input/subscription
    @param inp the input object to add a named publication
    @param target the name of the publication to get data from
    */
    void addTarget(const Input& inp, std::string_view target);
    /** remove a destination target from a publication
    @param pub the publication object to add a target to
    @param target the name of the input to remove
    */
    void removeTarget(const Publication& pub, std::string_view target);
    /** remove a publication from an input/subscription
    @param inp the input object to add a named publication
    @param target the name of the publication to remove
    */
    void removeTarget(const Input& inp, std::string_view target);

    /** add a 1-d Indexed target to an interface
   @details call is only valid in startup mode, register an optional subscription for a 1D array of
   values
   @param iObject an interface object to add the target to
   @param target the name of the target
   @param index1 the index into a 1 dimensional array of values
   */
    template<class iType>
    void addIndexedTarget(const iType& iObject, std::string_view target, int index1)
    {
        addTarget(iObject, std::string(target) + '_' + std::to_string(index1));
    }

    /** add an indexed target to an interface
    @details call is only valid in startup mode
    @param iObject an interface object such as a publication, filter or input
    @param target the name of the target
    @param index1 the first index of a 2-D value structure
    @param index2 the 2nd index of a 2-D value structure
    */
    template<class iType>
    void addIndexedTarget(const iType& iObject, std::string_view target, int index1, int index2)
    {
        addTarget(iObject,
                  std::string(target) + '_' + std::to_string(index1) + '_' +
                      std::to_string(index2));
    }

    /** check if a given subscription has an update
    @return true if the subscription id is valid and has an update*/
    bool isUpdated(const Input& inp) const;
    /** get the time of the last update*/
    Time getLastUpdateTime(const Input& inp) const;

    virtual void disconnect() override;
    /** clear all the updates
    @details after this call isUpdated on all the internal objects will return false*/
    void clearUpdates();
    /** clear all the update for a specific federate
    @details after this call isUpdated on the input will return false*/
    void clearUpdate(const Input& inp);

  protected:
    virtual void updateTime(Time newTime, Time oldTime) override;
    virtual void startupToInitializeStateTransition() override;
    virtual void initializeToExecuteStateTransition(iteration_time result) override;
    virtual std::string localQuery(std::string_view queryStr) const override;

  public:
    /** get a list of all the indices of all inputs that have been updated since the last call
    @return a vector of input indices with all the values that have not been retrieved since updated
    */
    std::vector<int> queryUpdates();

    /** get the name of the first target for an input
    @return empty string if an invalid input is passed or it has no target*/
    const std::string& getTarget(const Input& inp) const;
    /** get the id of a subscription
    @return an invalid input object if the name is invalid otherwise a reference to the
    corresponding input*/
    const Input& getInput(std::string_view name) const;
    /** get the id of an input
    @return an invalid input object if the target is valid otherwise a reference to the
    corresponding input*/
    Input& getInput(std::string_view name);
    /** get an input by index
    @return an invalid input object if the index is invalid otherwise a reference to the
    corresponding input*/
    const Input& getInput(int index) const;
    /** get an input object by index
    @return an invalid input object if the target is invalid otherwise a reference to the
    corresponding input*/
    Input& getInput(int index);
    /** get an indexed input object using the name and index
    @return an invalid input object if the target is valid otherwise a reference to the
    corresponding input*/
    const Input& getInput(std::string_view name, int index1) const;
    /** get an input object from a 2-d vector of inputs
    @return an invalid input object if the target is valid otherwise a reference to the
    corresponding input*/
    const Input& getInput(std::string_view name, int index1, int index2) const;

    /** get the input id based on target
    @return an invalid input object if the target is not valid, otherwise a reference to the
    corresponding input*/
    [[deprecated("Use getInputByTarget")]] const Input&
        getSubscription(std::string_view target) const
    {
        return getInputByTarget(target);
    }

    /** get an input based on target
    @details this will only get the first input with a specific target
   @return an invalid input object if the target is not valid, otherwise a reference to the
   corresponding input*/
    [[deprecated("Use getInputByTarget")]] Input& getSubscription(std::string_view target)
    {
        return getInputByTarget(target);
    }

    /** get the input id based on target
    @return an invalid input object if the target is not valid, otherwise a reference to the
    corresponding input*/
    const Input& getInputByTarget(std::string_view target) const;

    /** get an input based on target
    @details this will only get the first input with a specific target
    @return an invalid input object if the target is not valid, otherwise a reference to the
    corresponding input*/
    Input& getInputByTarget(std::string_view target);

    /** get a publication from its name
    @param name the name of the publication
    @return an invalid publication if the index is valid otherwise a reference to the corresponding
    publication*/
    Publication& getPublication(std::string_view name);
    /** get a publication from its name
    @param name the name of the publication
    @return an invalid publication if the index is valid otherwise a reference to the corresponding
    publication*/
    const Publication& getPublication(std::string_view name) const;
    /** get a publication from its index
    @param index the 0 based index of the publication to retrieve
    @return an invalid publication if the index is valid otherwise a reference to the corresponding
    publication*/
    Publication& getPublication(int index);
    /** get a publication from its index
    @param index the 0 based index of the publication to retrieve
    @return an invalid publication if the index is valid otherwise the corresponding publication*/
    const Publication& getPublication(int index) const;

    /** get a publication from its name
    @param name the name of the publication
    @param index1 the index into a vector of publications
    @return an invalid publication if the index is valid otherwise the corresponding publication*/
    const Publication& getPublication(std::string_view name, int index1) const;
    /** get a publication from a 2-d array of publications
    @param name the name of the publication
    @param index1 the first index of  2d array
    @param index2 the second index of a 2d array of publications
    @return an invalid publication if the index is valid otherwise the corresponding publication*/
    const Publication& getPublication(std::string_view name, int index1, int index2) const;

    /** register a callback function to call when any subscribed value is updated
    @details there can only be one generic callback
    @param callback the function to call signature void(Input &, Time)
    */
    void setInputNotificationCallback(std::function<void(Input&, Time)> callback);
    /** register a callback function to call when the specified subscription is updated
    @param inp an input to set the notification callback for
    @param callback the function to call
    */
    void setInputNotificationCallback(Input& inp, std::function<void(Input&, Time)> callback);

    /** get a count of the number publications registered*/
    int getPublicationCount() const;
    /** get a count of the number subscriptions registered*/
    int getInputCount() const;

  private:
    /** @brief PIMPL design pattern with the implementation details for the ValueFederate*/
    std::unique_ptr<ValueFederateManager> vfManager;
};

/** publish directly from the publication name
@details this is a convenience function to publish directly from the publication name
this function should not be used as the primary means of publications as it does involve an
additional map find operation vs the member publish calls
@param fed a reference to a valueFederate
@param pubName  the name of the publication
@param pargs any combination of arguments that go into the other publish commands
*/
template<class... Us>
void publish(ValueFederate& fed, std::string_view pubName, Us... pargs)
{
    fed.getPublication(pubName).publish(pargs...);
}

}  // namespace helics
