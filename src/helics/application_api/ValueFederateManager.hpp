/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../common/DualMappedVector.hpp"
#include "../common/GuardedTypes.hpp"
#include "../common/MappedVector.hpp"
#include "../core/Core.hpp"
#include "Inputs.hpp"
#include "Publications.hpp"
#include "data_view.hpp"
#include "helicsTypes.hpp"
#include <atomic>

namespace helics
{
/** forward declaration of Core*/
class Core;
class ValueFederate;

/** structure used to contain information about a publication*/
struct publication_info
{
    std::string name;  //!< publication name
    std::string type;  //!< publication type
    std::string units;  //!< publication units
    interface_handle coreID;  //!< Handle from the core
    publication_id_t id;  //!< the id used as the identifier
    int size = -1;  //!< required size of a publication
    bool forward = true;
    publication_info (const std::string &n_name, const std::string &n_type, const std::string &n_units)
        : name (n_name), type (n_type), units (n_units){};
};
/** structure used to contain information about a subscription*/
struct input_info
{
    data_view lastData;  //!< the last published data from a target
    Time lastUpdate = Time (0.0);  //!< the time the subscription was last updated
    Time lastQuery = Time (0.0);  //!< the time the query was made
    std::string name;  //!< subscription name
    std::string type;  //!< subscription type
    std::string units;  //!< subscription units
    std::string pubtype;  //!< the listed type of the corresponding publication
    interface_handle coreID;  //!< Handle from the core
    input_id_t id;  //!< the id used as the identifier

    std::function<void(Input &, Time)> callback;  //!< callback to trigger on update
    bool hasUpdate = false;  //!< indicator that there was an update
    input_info (const std::string &n_name, const std::string &n_type, const std::string &n_units)
        : name (n_name), type (n_type), units (n_units){};
};

/** class handling the implementation details of a value Federate*/
class ValueFederateManager
{
  public:
    ValueFederateManager (Core *coreOb, ValueFederate *vfed, federate_id_t id);
    ~ValueFederateManager ();

    Publication &registerPublication (const std::string &key, const std::string &type, const std::string &units);
    /** register a subscription
    @details call is only valid in startup mode
    */
    Input &registerInput (const std::string &key, const std::string &type, const std::string &units);

    /** add a shortcut for locating a subscription
    @details primarily for use in looking up an id from a different location
    creates a local shortcut for referring to a subscription which may have a long actual name
    @param[in] the subscription identifier
    @param[in] shortcutName the name of the shortcut
    */
    void addAlias (const Input &inp, const std::string &shortcutName);

    /** add a alias/shortcut for locating a publication
    @details primarily for use in looking up an id from a different location
    creates a local shortcut for referring to a subscription which may have a long actual name
    @param[in] the subscription identifier
    @param[in] shortcutName the name of the shortcut
    */
    void addAlias (const Publication &pub, const std::string &shortcutName);
    /** add a destination target to a publication
   @param id the identifier of the input
   target the name of the input to send the data to
   */
    void addTarget (const Publication &pub, const std::string &target);
    /** add a source target to an input/subscription
    @param id the identifier of the publication
    target the name of the input to send the data to
    */
    void addTarget (const Input &inp, const std::string &target);

    /** set the default value for a subscription
    @details this is the value returned prior to any publications
    @param[in] id the subscription identifier
    @param[in] block the data block representing the default value
    */
    void setDefaultValue (const Input &inp, const data_view &block);

    /** get a value as raw data block from the system
    @param[in] id the identifier for the subscription
    @return a constant data block
    */
    data_view getValue (const Input &inp);

    /** publish a value*/
    void publish (const Publication &pub, const data_view &block);

    /** check if a given subscription has and update*/
    bool hasUpdate (const Input &inp) const;
    /** get the time of the last update*/
    Time getLastUpdateTime (const Input &inp) const;

    /** update the time from oldTime to newTime
    @param[in] newTime the newTime of the federate
    @param[in] oldTime the oldTime of the federate
    */
    void updateTime (Time newTime, Time oldTime);
    /** transition from Startup To the Initialize State*/
    void startupToInitializeStateTransition ();
    /** transition from initialize to execution State*/
    void initializeToExecuteStateTransition ();
    /** generate results for a local query */
    std::string localQuery (const std::string &queryStr) const;
    /** get a list of all the values that have been updated since the last call
    @return a vector of subscription_ids with all the values that have not been retrieved since updated
    */
    std::vector<int> queryUpdates ();

    /** get the target of a input*/
    const std::string &getTarget (const Input &inp) const;

    /** get an Input from Its Name
    @param name the identifier or shortcut of the input
    @return ivalid_input_id if name is not a recognized*/
    Input &getInput (const std::string &name);
    const Input &getInput (const std::string &name) const;
    /** get an input by index*/
    Input &getInput (int index);
    const Input &getInput (int index) const;
    /** get the id of a subscription
    @param key the target of a subscription
  @return ivalid_input_id if name is not a recognized*/
    const Input &getSubscription (const std::string &key) const;
    Input &getSubscription (const std::string &key);

    /** get a publication based on its key
    @param key the publication id
    @return ivalid_publication_id if name is not recognized otherwise returns the publication_id*/
    Publication &getPublication (const std::string &key);
    const Publication &getPublication (const std::string &key) const;

    Publication &getPublication (int index);
    const Publication &getPublication (int index) const;

    /** register a callback function to call when any subscribed value is updated
    @details there can only be one generic callback
    @param[in] callback the function to call
    */
    void setInputNotificationCallback (std::function<void(Input &, Time)> callback);
    /** register a callback function to call when the specified subscription is updated
    @param[in] id  the id to register the callback for
    @param[in] callback the function to call
    */
    void setInputNotificationCallback (const Input &inp, std::function<void(Input &, Time)> callback);

    /** disconnect from the coreObject*/
    void disconnect ();

    /** get a count of the number publications registered*/
    int getPublicationCount () const;
    /** get a count of the number subscriptions registered*/
    int getInputCount () const;

  private:
    shared_guarded_m<DualMappedVector<Input, std::string, interface_handle, reference_stability::stable>> inputs;
    shared_guarded_m<DualMappedVector<Publication, std::string, interface_handle, reference_stability::stable>>
      publications;
    Time CurrentTime = Time (-1.0);  //!< the current simulation time
    Core *coreObject;  //!< the pointer to the actual core
    ValueFederate *fed;  //!< pointer back to the value Federate for creation of the Publication/Inputs
    federate_id_t fedID;  //!< the federation ID from the core API
    atomic_guarded<std::function<void(Input &, Time)>> allCallback;  //!< the global callback function
    shared_guarded<std::vector<std::unique_ptr<input_info>>>
      inputData;  //!< the storage for the message queues and other unique Endpoint information
    std::multimap<std::string, interface_handle> targetIDs;  //!< container for the target identifications
    std::multimap<interface_handle, std::string> inputTargets;  //!< container for the specified input targets
  private:
    void getUpdateFromCore (interface_handle handle);
};

}  // namespace helics
