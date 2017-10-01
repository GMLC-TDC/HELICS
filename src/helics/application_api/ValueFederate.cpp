/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "ValueFederate.h"
#include "ValueFederateManager.h"
#include "core/core.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#include "json/json.h"
#pragma warning(pop)
#else
#include "json/json.h"
#endif

#include <fstream>
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
ValueFederate::ValueFederate (const std::string &file) : Federate (file)
{
    vfManager = std::make_unique<ValueFederateManager> (coreObject, getID ());
}

ValueFederate::ValueFederate () {}

ValueFederate::ValueFederate (bool /*res*/)
{
    vfManager = std::make_unique<ValueFederateManager> (coreObject, getID ());
}

ValueFederate::ValueFederate (ValueFederate &&fed) noexcept = default;

ValueFederate::~ValueFederate () = default;

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
ValueFederate::registerPublication (const std::string &name, const std::string &type, const std::string &units)
{
    if (state != op_states::startup)
    {
        throw (InvalidFunctionCall ("cannot call register publication after entering initialization mode"));
    }
    return vfManager->registerPublication (getName () + separator_ + name, type, units);
}

publication_id_t ValueFederate::registerGlobalPublication (const std::string &name,
                                                           const std::string &type,
                                                           const std::string &units)
{
    if (state != op_states::startup)
    {
        throw (InvalidFunctionCall ("cannot call register publication after entering initialization mode"));
    }
    return vfManager->registerPublication (name, type, units);
}
subscription_id_t ValueFederate::registerRequiredSubscription (const std::string &name,
                                                               const std::string &type,
                                                               const std::string &units)
{
    if (state != op_states::startup)
    {
        throw (InvalidFunctionCall ("cannot call register subscription after entering initialization mode"));
    }
    return vfManager->registerRequiredSubscription (name, type, units);
}

subscription_id_t ValueFederate::registerOptionalSubscription (const std::string &name,
                                                               const std::string &type,
                                                               const std::string &units)
{
    if (state != op_states::startup)
    {
        throw (InvalidFunctionCall ("cannot call register subscription after entering initialization mode"));
    }
    return vfManager->registerOptionalSubscription (name, type, units);
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
    std::ifstream file (jsonString);
    Json_helics::Value doc;

    if (file.is_open ())
    {
        Json_helics::CharReaderBuilder rbuilder;
        std::string errs;
        bool ok = Json_helics::parseFromStream (rbuilder, file, &doc, &errs);
        if (!ok)
        {
            // should I throw an error here?
            return;
        }
    }
    else
    {
        Json_helics::Reader stringReader;
        bool ok = stringReader.parse (jsonString, doc, false);
        if (!ok)
        {
            // should I throw an error here?
            return;
        }
    }
    if (doc.isMember ("publications"))
    {
        auto pubs = doc["publications"];
        for (auto pubIt = pubs.begin (); pubIt != pubs.end (); ++pubIt)
        {
            auto pub = (*pubIt);
            auto name = pub["name"].asString ();
            auto type = (pub.isMember ("type")) ? pub["type"].asString () : "";
            auto units = (pub.isMember ("units")) ? pub["units"].asString () : "";
            bool global = (pub.isMember ("global")) ? !(pub["global"].asBool ()) : false;
            if (global)
            {
                registerGlobalPublication (name, type, units);
            }
            else
            {
                registerPublication (name, type, units);
            }
        }
    }
    if (doc.isMember ("subscriptions"))
    {
        auto subs = doc["subscriptions"];
        for (auto subIt = subs.begin (); subIt != subs.end (); ++subIt)
        {
            auto sub = (*subIt);
            auto name = sub["name"].asString ();
            auto units = (sub.isMember ("units")) ? sub["units"].asString () : "";
            auto type = (sub.isMember ("type")) ? sub["type"].asString () : "";
            bool required = (sub.isMember ("optional")) ? !(sub["optional"].asBool ()) : false;
            if (sub.isMember ("required"))
            {
                required = sub["required"].asBool ();
            }
            subscription_id_t id;
            if (required)
            {
                id = registerRequiredSubscription (name, type, units);
            }
            else
            {
                id = registerOptionalSubscription (name, type, units);
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

void ValueFederate::StartupToInitializeStateTransition () { vfManager->StartupToInitializeStateTransition (); }
void ValueFederate::InitializeToExecuteStateTransition () { vfManager->InitializeToExecuteStateTransition (); }

std::vector<subscription_id_t> ValueFederate::queryUpdates () { return vfManager->queryUpdates (); }

std::string ValueFederate::getSubscriptionName (subscription_id_t sub_id) const
{
    return vfManager->getSubscriptionName (sub_id);
}

subscription_id_t ValueFederate::getSubscriptionId (const std::string &name) const
{
    return vfManager->getSubscriptionId (name);
}

subscription_id_t ValueFederate::getSubscriptionId (const std::string &name, int index1) const
{
    return vfManager->getSubscriptionId (name + '_' + std::to_string (index1));
}

subscription_id_t ValueFederate::getSubscriptionId (const std::string &name, int index1, int index2) const
{
    return vfManager->getSubscriptionId (name + '_' + std::to_string (index1) + '_' + std::to_string (index2));
}

std::string ValueFederate::getPublicationName (publication_id_t pub_id) const
{
    return vfManager->getPublicationName (pub_id);
}

publication_id_t ValueFederate::getPublicationId (const std::string &name) const
{
    auto id = vfManager->getPublicationId (name);
    if (id == invalid_id_value)
    {
        id = vfManager->getPublicationId (getName () + separator_ + name);
    }
    return id;
}

publication_id_t ValueFederate::getPublicationId (const std::string &name, int index1) const
{
    return vfManager->getPublicationId (name + '_' + std::to_string (index1));
}

publication_id_t ValueFederate::getPublicationId (const std::string &name, int index1, int index2) const
{
    return vfManager->getPublicationId (name + '_' + std::to_string (index1) + '_' + std::to_string (index2));
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
}