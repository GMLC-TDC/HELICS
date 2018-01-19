/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "ValueFederate.hpp"
#include "../core/Core.hpp"
#include "../core/core-exceptions.hpp"
#include "ValueFederateManager.hpp"
#include "../common/JsonProcessingFunctions.hpp"

namespace helics
{
/**constructor taking a core engine and federate info structure
 */
ValueFederate::ValueFederate (const FederateInfo &fi) : Federate (fi)
{
    vfManager = std::make_unique<ValueFederateManager> (coreObject, getID ());
}
ValueFederate::ValueFederate (std::shared_ptr<Core> core, const FederateInfo &fi) : Federate (std::move (core), fi)
{
    vfManager = std::make_unique<ValueFederateManager> (coreObject, getID ());
}
ValueFederate::ValueFederate (const std::string &jsonString) : Federate (jsonString)
{
    vfManager = std::make_unique<ValueFederateManager> (coreObject, getID ());
    registerInterfaces (jsonString);
}

ValueFederate::ValueFederate () = default;

ValueFederate::ValueFederate (bool /*res*/)
{
    vfManager = std::make_unique<ValueFederateManager> (coreObject, getID ());
}

ValueFederate::ValueFederate (ValueFederate &&fed) noexcept = default;

ValueFederate::~ValueFederate () = default;

void ValueFederate::disconnect ()
{
    Federate::disconnect ();
    vfManager->disconnect ();
}

ValueFederate &ValueFederate::operator= (ValueFederate &&fed) noexcept
{
    if (getID () != fed.getID ())
    {  // the id won't be moved, as it is copied so use it as a test if it has moved already
        Federate::operator= (std::move (fed));
    }
    vfManager = std::move (fed.vfManager);
    return *this;
}

publication_id_t
ValueFederate::registerPublication (const std::string &key, const std::string &type, const std::string &units)
{
    if (state != op_states::startup)
    {
        throw (InvalidFunctionCall ("cannot call register publication after entering initialization mode"));
    }
    return vfManager->registerPublication (getName () + separator_ + key, type, units);
}

publication_id_t ValueFederate::registerGlobalPublication (const std::string &key,
                                                           const std::string &type,
                                                           const std::string &units)
{
    if (state != op_states::startup)
    {
        throw (InvalidFunctionCall ("cannot call register publication after entering initialization mode"));
    }
    return vfManager->registerPublication (key, type, units);
}
subscription_id_t ValueFederate::registerRequiredSubscription (const std::string &key,
                                                               const std::string &type,
                                                               const std::string &units)
{
    if (state != op_states::startup)
    {
        throw (InvalidFunctionCall ("cannot call register subscription after entering initialization mode"));
    }
    return vfManager->registerRequiredSubscription (key, type, units);
}

subscription_id_t ValueFederate::registerOptionalSubscription (const std::string &key,
                                                               const std::string &type,
                                                               const std::string &units)
{
    if (state != op_states::startup)
    {
        throw (InvalidFunctionCall ("cannot call register subscription after entering initialization mode"));
    }
    return vfManager->registerOptionalSubscription (key, type, units);
}

void ValueFederate::addSubscriptionShortcut (subscription_id_t subid, const std::string &shortcutName)
{
    vfManager->addSubscriptionShortcut (subid, shortcutName);
}

void ValueFederate::setDefaultValue (subscription_id_t id, data_view block)
{
    vfManager->setDefaultValue (id, block);
}

void ValueFederate::registerInterfaces (const std::string &jsonString)
{
    if (state != op_states::startup)
    {
        throw (InvalidFunctionCall ("cannot call register Interfaces after entering initialization mode"));
    }
	auto doc = loadJsonString(jsonString);
    
    if (doc.isMember ("publications"))
    {
        auto pubs = doc["publications"];
        for (const auto &pub : pubs)
        {
            auto key = (pub.isMember ("key")) ?
                         pub["key"].asString () :
                         ((pub.isMember ("name")) ? pub["name"].asString () : std::string ());

            auto id = vfManager->getPublicationId (key);
            if (id != invalid_id_value)
            {
                continue;
            }
            auto type = (pub.isMember ("type")) ? pub["type"].asString () : std ::string ();
            auto units = (pub.isMember ("units")) ? pub["units"].asString () : std::string ();
            bool global = (pub.isMember ("global")) ? (pub["global"].asBool ()) : false;
            if (global)
            {
                registerGlobalPublication (key, type, units);
            }
            else
            {
                registerPublication (key, type, units);
            }
        }
    }
    if (doc.isMember ("subscriptions"))
    {
        auto subs = doc["subscriptions"];
        for (const auto &sub : subs)
        {
            auto key = (sub.isMember ("key")) ?
                         sub["key"].asString () :
                         ((sub.isMember ("name")) ? sub["name"].asString () : std::string ());
            auto id = vfManager->getSubscriptionId (key);
            if (id != invalid_id_value)
            {
                continue;
            }
            auto units = (sub.isMember ("units")) ? sub["units"].asString () : "";
            auto type = (sub.isMember ("type")) ? sub["type"].asString () : "";
            bool required = (sub.isMember ("optional")) ? !(sub["optional"].asBool ()) : true;
            if (sub.isMember ("required"))
            {
                required = sub["required"].asBool ();
            }
            if (required)
            {
                id = registerRequiredSubscription (key, type, units);
            }
            else
            {
                id = registerOptionalSubscription (key, type, units);
            }
            if (sub.isMember ("shortcut"))
            {
                addSubscriptionShortcut (id, sub["shortcut"].asString ());
            }
        }
    }
}

data_view ValueFederate::getValueRaw (subscription_id_t id) { return vfManager->getValue (id); }

void ValueFederate::publish (publication_id_t id, data_view block)
{
    if ((state == op_states::execution) || (state == op_states::initialization))
    {
        vfManager->publish (id, block);
    }
    else
    {
        throw (InvalidFunctionCall ("publications not allowed outside of execution and initialization state"));
    }
}

bool ValueFederate::isUpdated (subscription_id_t sub_id) const { return vfManager->queryUpdate (sub_id); }

Time ValueFederate::getLastUpdateTime (subscription_id_t sub_id) const
{
    return vfManager->queryLastUpdate (sub_id);
}

void ValueFederate::updateTime (Time newTime, Time oldTime) { vfManager->updateTime (newTime, oldTime); }

void ValueFederate::startupToInitializeStateTransition () { vfManager->startupToInitializeStateTransition (); }
void ValueFederate::initializeToExecuteStateTransition () { vfManager->initializeToExecuteStateTransition (); }

std::vector<subscription_id_t> ValueFederate::queryUpdates () { return vfManager->queryUpdates (); }

std::string ValueFederate::getSubscriptionKey (subscription_id_t sub_id) const
{
    return vfManager->getSubscriptionKey (sub_id);
}

subscription_id_t ValueFederate::getSubscriptionId (const std::string &key) const
{
    return vfManager->getSubscriptionId (key);
}

subscription_id_t ValueFederate::getSubscriptionId (const std::string &key, int index1) const
{
    return vfManager->getSubscriptionId (key + '_' + std::to_string (index1));
}

subscription_id_t ValueFederate::getSubscriptionId (const std::string &key, int index1, int index2) const
{
    return vfManager->getSubscriptionId (key + '_' + std::to_string (index1) + '_' + std::to_string (index2));
}

std::string ValueFederate::getPublicationKey (publication_id_t pub_id) const
{
    return vfManager->getPublicationKey (pub_id);
}

publication_id_t ValueFederate::getPublicationId (const std::string &key) const
{
    auto id = vfManager->getPublicationId (key);
    if (id == invalid_id_value)
    {
        id = vfManager->getPublicationId (getName () + separator_ + key);
    }
    return id;
}

publication_id_t ValueFederate::getPublicationId (const std::string &key, int index1) const
{
    return vfManager->getPublicationId (key + '_' + std::to_string (index1));
}

publication_id_t ValueFederate::getPublicationId (const std::string &key, int index1, int index2) const
{
    return vfManager->getPublicationId (key + '_' + std::to_string (index1) + '_' + std::to_string (index2));
}

std::string ValueFederate::getSubscriptionUnits (subscription_id_t id) const
{
    return vfManager->getSubscriptionUnits (id);
}
std::string ValueFederate::getPublicationUnits (publication_id_t id) const
{
    return vfManager->getPublicationUnits (id);
}

std::string ValueFederate::getSubscriptionType (subscription_id_t id) const
{
    return vfManager->getSubscriptionType (id);
}
std::string ValueFederate::getPublicationType (publication_id_t id) const
{
    return vfManager->getPublicationType (id);
}

std::string ValueFederate::getPublicationType (subscription_id_t id) const
{
    return vfManager->getPublicationType (id);
}

void ValueFederate::registerSubscriptionNotificationCallback (
  std::function<void(subscription_id_t, Time)> callback)
{
    vfManager->registerCallback (callback);
}

void ValueFederate::registerSubscriptionNotificationCallback (
  subscription_id_t id,
  std::function<void(subscription_id_t, Time)> callback)
{
    vfManager->registerCallback (id, callback);
}

void ValueFederate::registerSubscriptionNotificationCallback (
  const std::vector<subscription_id_t> &ids,
  std::function<void(subscription_id_t, Time)> callback)
{
    vfManager->registerCallback (ids, callback);
}

/** get a count of the number publications registered*/
int ValueFederate::getPublicationCount () const { return vfManager->getPublicationCount (); }
/** get a count of the number subscriptions registered*/
int ValueFederate::getSubscriptionCount () const { return vfManager->getSubscriptionCount (); }
}  // namespace helics