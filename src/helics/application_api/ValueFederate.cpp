/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "ValueFederate.hpp"
#include "../common/JsonProcessingFunctions.hpp"
#include "../common/TomlProcessingFunctions.hpp"
#include "../core/Core.hpp"
#include "../core/core-exceptions.hpp"
#include "ValueFederateManager.hpp"

namespace helics
{
/**constructor taking a core engine and federate info structure
 */
ValueFederate::ValueFederate (const std::string &fedName, const FederateInfo &fi) : Federate (fedName,fi)
{
    // the core object get instantiated in the Federate constructor
    vfManager = std::make_unique<ValueFederateManager> (coreObject.get (), getID ());
}
ValueFederate::ValueFederate (const std::string &fedName, const std::shared_ptr<Core> &core, const FederateInfo &fi)
    : Federate (fedName,core, fi)
{
    vfManager = std::make_unique<ValueFederateManager> (coreObject.get (), getID ());
}
ValueFederate::ValueFederate (const std::string &configString) : Federate (std::string(),loadFederateInfo (configString))
{
    vfManager = std::make_unique<ValueFederateManager> (coreObject.get (), getID ());
    ValueFederate::registerInterfaces (configString);
}

ValueFederate::ValueFederate (const std::string &fedName, const std::string &configString)
    : Federate (fedName,loadFederateInfo (configString))
{
    vfManager = std::make_unique<ValueFederateManager> (coreObject.get (), getID ());
    ValueFederate::registerInterfaces (configString);
}

ValueFederate::ValueFederate () = default;

ValueFederate::ValueFederate (bool /*res*/)
{
    vfManager = std::make_unique<ValueFederateManager> (coreObject.get (), getID ());
}

ValueFederate::ValueFederate (ValueFederate &&) noexcept = default;

ValueFederate::~ValueFederate () = default;

void ValueFederate::disconnect ()
{
    Federate::disconnect ();
    vfManager->disconnect ();
}

ValueFederate &ValueFederate::operator= (ValueFederate &&fed) noexcept
{
    vfManager = std::move (fed.vfManager);
    if (getID () != fed.getID ())
    {  // the id won't be moved, as it is copied so use it as a test if it has moved already
        Federate::operator= (std::move (fed));
    }
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

input_id_t
ValueFederate::registerInput(const std::string &key, const std::string &type, const std::string &units)
{
    if (state != op_states::startup)
    {
        throw (InvalidFunctionCall ("cannot call register publication after entering initialization mode"));
    }
    return vfManager->registerInput (getName () + separator_ + key, type, units);
}

input_id_t ValueFederate::registerGlobalInput(const std::string &key, const std::string &type, const std::string &units)
{
    if (state != op_states::startup)
    {
        throw (InvalidFunctionCall ("cannot call register publication after entering initialization mode"));
    }
    return vfManager->registerInput (key, type, units);
}

input_id_t ValueFederate::registerSubscription (const std::string &key,
                                                               const std::string &units)
{
    auto id=vfManager->registerInput (std::string(), std::string(), units);
    vfManager->addTarget (id, key);
    return id;
}


void ValueFederate::addTarget(publication_id_t id, const std::string &target)
{
    vfManager->addTarget (id, target);
}

void ValueFederate::addTarget(input_id_t id, const std::string &target)
{ vfManager->addTarget (id, target); }


void ValueFederate::addShortcut (input_id_t subid, const std::string &shortcutName)
{
    vfManager->addShortcut (subid, shortcutName);
}

void ValueFederate::setDefaultValue (input_id_t id, data_view block)
{
    vfManager->setDefaultValue (id, block);
}

void ValueFederate::registerInterfaces (const std::string &configString)
{
    registerValueInterfaces (configString);
    Federate::registerInterfaces (configString);
}

void ValueFederate::registerValueInterfaces (const std::string &configString)
{
    if (state != op_states::startup)
    {
        throw (InvalidFunctionCall ("cannot call register Interfaces after entering initialization mode"));
    }
    if (hasTomlExtension (configString))
	{
        registerValueInterfacesToml (configString);
	}
	else
	{
        registerValueInterfacesJson (configString);
	}
}

void ValueFederate::registerValueInterfacesJson(const std::string &configString)
{
    auto doc = loadJson (configString);

    if (doc.isMember ("publications"))
    {
        auto pubs = doc["publications"];
        for (const auto &pub : pubs)
        {
            auto key = getKey (pub);

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
            auto key = getKey (sub);
            auto id = vfManager->getSubscriptionId (key);
            if (id != invalid_id_value)
            {
                continue;
            }
            auto units = (sub.isMember ("units")) ? sub["units"].asString () : "";
            bool required = (sub.isMember ("optional")) ? !(sub["optional"].asBool ()) : true;
            if (sub.isMember ("required"))
            {
                required = sub["required"].asBool ();
            }
            id = registerSubscription (key, units);
            if (required)
            {
                //TODO add setOPTION call
            }
            if (sub.isMember ("shortcut"))
            {
                addShortcut (id, sub["shortcut"].asString ());
            }
        }
    }
    if (doc.isMember ("inputs"))
    {
        auto ipts = doc["inputs"];
        for (const auto &ipt : ipts)
        {
            auto key = getKey (ipt);

            auto id = vfManager->getInputId (key);
            if (id != invalid_id_value)
            {
                continue;
            }
            auto type = (ipt.isMember ("type")) ? ipt["type"].asString () : std ::string ();
            auto units = (ipt.isMember ("units")) ? ipt["units"].asString () : std::string ();
            bool global = (ipt.isMember ("global")) ? (ipt["global"].asBool ()) : false;
            if (global)
            {
                registerGlobalInput (key, type, units);
            }
            else
            {
                registerInput (key, type, units);
            }
        }
    }
}

void ValueFederate::registerValueInterfacesToml(const std::string &tomlString)
{
    toml::Value doc;
    try
    {
        doc = loadToml (tomlString);
    }
    catch (const std::invalid_argument &ia)
    {
        throw (helics::InvalidParameter (ia.what ()));
    }

	auto pubs = doc.find ("publications");
    if (pubs!=nullptr)
    {
        auto &pubArray = pubs->as<toml::Array> ();
        for (const auto &pub : pubArray)
        {
            auto key = getKey (pub);

            auto id = vfManager->getPublicationId (key);
            if (id != invalid_id_value)
            {
                continue;
            }
            auto type = tomlGetOrDefault (pub, "type", std::string ());
            auto units = tomlGetOrDefault (pub, "units", std::string ());
            bool global = tomlGetOrDefault (pub, "global", false);
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
    auto subs = doc.find ("subscriptions");
    if (subs != nullptr)
    {
        auto &subArray = subs->as<toml::Array> ();
        for (const auto &sub : subArray)
        {
            auto key = getKey (sub);
            auto id = vfManager->getSubscriptionId (key);
            if (id != invalid_id_value)
            {
                continue;
            }
            auto units = tomlGetOrDefault (sub, "units", std::string ());
            bool optional = tomlGetOrDefault (sub, "optional", false);
            bool required = tomlGetOrDefault (sub, "required", !optional);
            id = registerSubscription (key, units);
            if (required)
            {
               //setInterfaceOption()
            }
            
            auto shortcut = sub.find ("shortcut");
            if (shortcut!=nullptr)
            {
                addShortcut (id, shortcut->as<std::string> ());
            }
        }
    }
    auto ipts = doc.find ("inputs");
    if (ipts != nullptr)
    {
        auto &iptArray = ipts->as<toml::Array> ();
        for (const auto &ipt : iptArray)
        {
            auto key = getKey (ipt);

            auto id = vfManager->getPublicationId (key);
            if (id != invalid_id_value)
            {
                continue;
            }
            auto type = tomlGetOrDefault (ipt, "type", std::string ());
            auto units = tomlGetOrDefault (ipt, "units", std::string ());
            bool global = tomlGetOrDefault (ipt, "global", false);
            if (global)
            {
                registerGlobalInput(key, type, units);
            }
            else
            {
                registerInput (key, type, units);
            }
        }
    }
}

data_view ValueFederate::getValueRaw (input_id_t id) { return vfManager->getValue (id); }

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

bool ValueFederate::isUpdated (input_id_t sub_id) const { return vfManager->hasUpdate (sub_id); }

Time ValueFederate::getLastUpdateTime (input_id_t sub_id) const
{
    return vfManager->getLastUpdateTime (sub_id);
}

void ValueFederate::updateTime (Time newTime, Time oldTime) { vfManager->updateTime (newTime, oldTime); }

void ValueFederate::startupToInitializeStateTransition () { vfManager->startupToInitializeStateTransition (); }
void ValueFederate::initializeToExecuteStateTransition () { vfManager->initializeToExecuteStateTransition (); }

std::vector<input_id_t> ValueFederate::queryUpdates () { return vfManager->queryUpdates (); }

std::string ValueFederate::getInputKey (input_id_t ipt_id) const
{
    return vfManager->getInputKey (ipt_id);
}

std::string ValueFederate::getTarget(input_id_t id) const
{
    return vfManager->getTarget(id);
}

input_id_t ValueFederate::getInputId (const std::string &key) const
{
    return vfManager->getInputId (key);
}

input_id_t ValueFederate::getInputId (const std::string &key, int index1) const
{
    return vfManager->getInputId (key + '_' + std::to_string (index1));
}

input_id_t ValueFederate::getInputId (const std::string &key, int index1, int index2) const
{
    return vfManager->getInputId (key + '_' + std::to_string (index1) + '_' + std::to_string (index2));
}

input_id_t ValueFederate::getSubscriptionId(const std::string &key) const
{
    return vfManager->getSubscriptionId (key);
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

std::string ValueFederate::getInputUnits (input_id_t id) const
{
    return vfManager->getInputUnits (id);
}
std::string ValueFederate::getPublicationUnits (publication_id_t id) const
{
    return vfManager->getPublicationUnits (id);
}

std::string ValueFederate::getInputType (input_id_t id) const
{
    return vfManager->getInputType (id);
}
std::string ValueFederate::getPublicationType (publication_id_t id) const
{
    return vfManager->getPublicationType (id);
}

std::string ValueFederate::getPublicationType (input_id_t id) const
{
    return vfManager->getPublicationType (id);
}

void ValueFederate::registerInputNotificationCallback (
  std::function<void(input_id_t, Time)> callback)
{
    vfManager->registerCallback (callback);
}

void ValueFederate::registerInputNotificationCallback (
  input_id_t id,
  std::function<void(input_id_t, Time)> callback)
{
    vfManager->registerCallback (id, callback);
}

void ValueFederate::registerInputNotificationCallback (
  const std::vector<input_id_t> &ids,
  std::function<void(input_id_t, Time)> callback)
{
    vfManager->registerCallback (ids, callback);
}

/** get a count of the number publications registered*/
int ValueFederate::getPublicationCount () const { return vfManager->getPublicationCount (); }
/** get a count of the number subscriptions registered*/
int ValueFederate::getInputCount () const { return vfManager->getInputCount (); }
}  // namespace helics
