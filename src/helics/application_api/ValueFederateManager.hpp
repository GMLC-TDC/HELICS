/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../common/GuardedTypes.hpp"
#include "../core/LocalFederateId.hpp"
#include "Inputs.hpp"
#include "ValueFederate.hpp"
#include "data_view.hpp"
#include "gmlc/containers/DualStringMappedVector.hpp"
#include "helicsTypes.hpp"

#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace helics {
/** forward declaration of Core*/
class Core;
class ValueFederate;

/** structure used to contain information about a publication*/
struct publication_info {
    std::string name;  //!< publication name
    std::string type;  //!< publication type
    std::string units;  //!< publication units
    InterfaceHandle coreID;  //!< Handle from the core
    PublicationId id;  //!< the id used as the identifier
    int size{-1};  //!< required size of a publication
    bool forward{true};
    publication_info(std::string_view n_name, std::string_view n_type, std::string_view n_units):
        name(n_name), type(n_type), units(n_units)
    {
    }
};
/** structure used to contain information about a subscription*/
struct InputData {
    InterfaceHandle coreID;  //!< Handle from the core
    InputId id;  //!< the id used as the identifier
    data_view lastData;  //!< the last published data from a target
    Time lastUpdate{0.0};  //!< the time the subscription was last updated
    Time lastQuery{0.0};  //!< the time the query was made
    int sourceIndex{0};  //!< the index of the data source for multi-source inputs
    std::string name;  //!< input name
    std::string type;  //!< input type
    std::string units;  //!< input units
    std::string pubtype;  //!< the listed type of the corresponding publication

    std::function<void(Input&, Time)> callback;  //!< callback to trigger on update
    bool hasUpdate = false;  //!< indicator that there was an update
    InputData(std::string_view n_name, std::string_view n_type, std::string_view n_units):
        name(n_name), type(n_type), units(n_units)
    {
    }
};

/** class handling the implementation details of a value Federate*/
class ValueFederateManager {
  public:
    ValueFederateManager(Core* coreOb,
                         ValueFederate* vfed,
                         LocalFederateId id,
                         bool singleThreaded);
    ~ValueFederateManager();

    Publication&
        registerPublication(std::string_view key, std::string_view type, std::string_view units);
    /** register a subscription
    @details call is only valid in startup mode
    */
    Input& registerInput(std::string_view key, std::string_view type, std::string_view units);

    /** add a shortcut for locating a subscription
    @details primarily for use in looking up an id from a different location
    creates a local shortcut for referring to a subscription which may have a long actual name
    @param inp the subscription identifier
    @param shortcutName the name of the shortcut
    */
    void addAlias(const Input& inp, std::string_view shortcutName);

    /** add a alias/shortcut for locating a publication
    @details primarily for use in looking up an id from a different location
    creates a local shortcut for referring to a subscription which may have a long actual name
    @param pub the subscription identifier
    @param shortcutName the name of the shortcut
    */
    void addAlias(const Publication& pub, std::string_view shortcutName);
    /** add a destination target to a publication
   @param pub the identifier of the input
   @param target the name of the input to send the data to
   */
    void addTarget(const Publication& pub, std::string_view target);
    /** add a source target to an input/subscription
    @param inp the identifier of the publication
    @param target the name of the input to send the data to
    */
    void addTarget(const Input& inp, std::string_view target);

    /** remove a destination target from a publication
    @param pub the identifier of the input
    @param target the name of the input to remove
    */
    void removeTarget(const Publication& pub, std::string_view target);
    /** remove a source target from an input/subscription
    @param inp the identifier of the publication
    @param target the name of the publication to remove
    */
    void removeTarget(const Input& inp, std::string_view target);

    /** set the default value for a subscription
    @details this is the value returned prior to any publications
    @param inp the subscription identifier
    @param block the data block representing the default value
    */
    void setDefaultValue(const Input& inp, const data_view& block);

    /** get a value as raw data block from the system
    @param inp the identifier for the subscription
    @return a constant data block
    */
    data_view getValue(const Input& inp);

    /** publish a value*/
    void publish(const Publication& pub, const data_view& block);

    /** check if a given subscription has and update*/
    static bool hasUpdate(const Input& inp);
    /** get the time of the last update*/
    static Time getLastUpdateTime(const Input& inp);

    /** update the time from oldTime to newTime
    @param newTime the newTime of the federate
    @param oldTime the oldTime of the federate
    */
    void updateTime(Time newTime, Time oldTime);
    /** transition from Startup To the Initialize State*/
    void startupToInitializeStateTransition();
    /** transition from initialize to execution State*/
    void initializeToExecuteStateTransition(iteration_time result);
    /** generate results for a local query */
    std::string localQuery(std::string_view queryStr) const;
    /** get a list of all the values that have been updated since the last call
    @return a vector of subscription_ids with all the values that have not been retrieved since
    updated
    */
    std::vector<int> queryUpdates();

    /** get the target of a input*/
    const std::string& getTarget(const Input& inp) const;

    /** get an Input from Its Name
    @param key the identifier or shortcut of the input
    @return an empty Input if name is not a recognized */
    Input& getInput(std::string_view key);
    const Input& getInput(std::string_view key) const;
    /** get an input by index*/
    Input& getInput(int index);
    const Input& getInput(int index) const;

    /** get the id of an by the target name
    @param key the target of a input
    @return an invalid input if the target is not found*/
    const Input& getInputByTarget(std::string_view key) const;
    Input& getInputByTarget(std::string_view key);

    /** get a publication based on its key
    @param key the publication id
    @return ivalid_publication_id if name is not recognized otherwise returns the publication_id*/
    Publication& getPublication(std::string_view key);
    const Publication& getPublication(std::string_view key) const;

    Publication& getPublication(int index);
    const Publication& getPublication(int index) const;

    /** register a callback function to call when any subscribed value is updated
    @details there can only be one generic callback
    @param callback the function to call
    */
    void setInputNotificationCallback(std::function<void(Input&, Time)> callback);
    /** register a callback function to call when the specified subscription is updated
    @param inp  the id to register the callback for
    @param callback the function to call
    */
    static void setInputNotificationCallback(const Input& inp,
                                             std::function<void(Input&, Time)> callback);

    /** disconnect from the coreObject*/
    void disconnect();

    /** get a count of the number publications registered*/
    int getPublicationCount() const;
    /** get a count of the number subscriptions registered*/
    int getInputCount() const;
    /** clear all the updates
    @details after this call isUpdated on all the internal objects will return false*/
    void clearUpdates();
    /** clear an input value as updated without actually retrieving it
    @param inp the identifier for the subscription
    */
    static void clearUpdate(const Input& inp);
    bool getUpdateFromCore(Input& inp);

  public:
    bool useJsonSerialization{false};  //!< all outgoing data should be serialized as JSON
  private:
    LocalFederateId fedID;  //!< the federation ID from the core API
    shared_guarded_m_opt<gmlc::containers::DualStringMappedVector<Input, InterfaceHandle>> inputs;
    shared_guarded_m_opt<gmlc::containers::DualStringMappedVector<Publication, InterfaceHandle>>
        publications;
    Time CurrentTime{-1.0};  //!< the current simulation time
    Core* coreObject{nullptr};  //!< the pointer to the actual core
    /** pointer back to the value Federate for creation of the Publication/Inputs */
    ValueFederate* fed{nullptr};
    /// the global callback function
    atomic_guarded<std::function<void(Input&, Time)>> allCallback;
    /// the storage for the message queues and other unique Endpoint information
    shared_guarded_opt<std::deque<InputData>> inputData;
    /// container for the target identifications
    shared_guarded_opt<std::multimap<std::string, InterfaceHandle>> targetIDs;
    /// container for the specified input targets
    shared_guarded_opt<std::multimap<InterfaceHandle, std::string>> inputTargets;

  private:
    void getUpdateFromCore(InterfaceHandle handle);
};

}  // namespace helics
