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
    vfManager = std::make_unique<ValueFederateManager> (coreObject.get (),this, getID ());
}
ValueFederate::ValueFederate (const std::string &fedName, const std::shared_ptr<Core> &core, const FederateInfo &fi)
    : Federate (fedName,core, fi)
{
    vfManager = std::make_unique<ValueFederateManager> (coreObject.get (),this, getID ());
}
ValueFederate::ValueFederate (const std::string &configString) : Federate (std::string(),loadFederateInfo (configString))
{
    vfManager = std::make_unique<ValueFederateManager> (coreObject.get (),this, getID ());
    ValueFederate::registerInterfaces (configString);
}

ValueFederate::ValueFederate (const std::string &fedName, const std::string &configString)
    : Federate (fedName,loadFederateInfo (configString))
{
    vfManager = std::make_unique<ValueFederateManager> (coreObject.get (), this,getID ());
    ValueFederate::registerInterfaces (configString);
}

ValueFederate::ValueFederate () = default;

ValueFederate::ValueFederate (bool /*res*/)
{
    vfManager = std::make_unique<ValueFederateManager> (coreObject.get (), this,getID ());
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

Publication &
ValueFederate::registerPublication (const std::string &key, const std::string &type, const std::string &units)
{
    return vfManager->registerPublication ((!key.empty())?(getName () + separator_ + key):key, type, units);
}

Publication & ValueFederate::registerGlobalPublication (const std::string &key,
                                                           const std::string &type,
                                                           const std::string &units)
{
    return vfManager->registerPublication (key, type, units);
}

Input &
ValueFederate::registerInput(const std::string &key, const std::string &type, const std::string &units)
{
    return vfManager->registerInput ((!key.empty ()) ? (getName () + separator_ + key) : key, type, units);
}

Input & ValueFederate::registerGlobalInput(const std::string &key, const std::string &type, const std::string &units)
{
    return vfManager->registerInput (key, type, units);
}

Input & ValueFederate::registerSubscription (const std::string &key,
                                                               const std::string &units)
{
    auto &inp=vfManager->registerInput (std::string(), std::string(), units);
    vfManager->addTarget (inp, key);
    return inp;
}


void ValueFederate::addTarget(const Publication &pub, const std::string &target)
{
    vfManager->addTarget (pub, target);
}

void ValueFederate::addTarget (const Input &inp, const std::string &target) { vfManager->addTarget (inp, target); }


void ValueFederate::addShortcut (const Input &inp, const std::string &shortcutName)
{
    vfManager->addShortcut (inp, shortcutName);
}

void ValueFederate::setDefaultValue (const Input &inp, data_view block)
{
    vfManager->setDefaultValue (inp, block);
}

void ValueFederate::registerInterfaces (const std::string &configString)
{
    registerValueInterfaces (configString);
    Federate::registerInterfaces (configString);
}

void ValueFederate::registerValueInterfaces (const std::string &configString)
{
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

            auto &pubAct = vfManager->getPublication (key);
            if (pubAct.isValid())
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
            auto &subAct = vfManager->getSubscription (key);
            if (subAct.isValid())
            {
                continue;
            }
            auto units = (sub.isMember ("units")) ? sub["units"].asString () : "";
            bool required = (sub.isMember ("optional")) ? !(sub["optional"].asBool ()) : true;
            if (sub.isMember ("required"))
            {
                required = sub["required"].asBool ();
            }
            auto &subNew = registerSubscription (key, units);
            if (required)
            {

                //TODO add setOPTION call
            }
            if (sub.isMember ("shortcut"))
            {
                addShortcut (subNew, sub["shortcut"].asString ());
            }
        }
    }
    if (doc.isMember ("inputs"))
    {
        auto ipts = doc["inputs"];
        for (const auto &ipt : ipts)
        {
            auto key = getKey (ipt);

            auto inp = vfManager->getInput (key);
            if (inp.isValid())
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

            auto id = vfManager->getPublication (key);
            if (id.isValid())
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
            auto id = vfManager->getSubscription (key);
            if (id.isValid())
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

            auto id = vfManager->getPublication(key);
            if (id.isValid())
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

data_view ValueFederate::getValueRaw (const Input &inp) { return vfManager->getValue (inp); }

void ValueFederate::publish (const Publication &pub, data_view block)
{
    if ((state == op_states::execution) || (state == op_states::initialization))
    {
        vfManager->publish (pub, block);
    }
    else
    {
        throw (InvalidFunctionCall ("publications not allowed outside of execution and initialization state"));
    }
}

bool ValueFederate::isUpdated (const Input &inp) const { return vfManager->hasUpdate (inp); }

Time ValueFederate::getLastUpdateTime (const Input &inp) const {
    return vfManager->getLastUpdateTime (inp);
}

void ValueFederate::updateTime (Time newTime, Time oldTime) { vfManager->updateTime (newTime, oldTime); }

void ValueFederate::startupToInitializeStateTransition () { vfManager->startupToInitializeStateTransition (); }
void ValueFederate::initializeToExecuteStateTransition () { vfManager->initializeToExecuteStateTransition (); }


std::string ValueFederate::localQuery(const std::string &queryStr) const
{
    return vfManager->localQuery(queryStr);
}

std::vector<input_id_t> ValueFederate::queryUpdates () { return vfManager->queryUpdates (); }

const std::string &ValueFederate::getInputKey (const Input &inp) const {
    return vfManager->getInputKey (inp);
}

const std::string &ValueFederate::getTarget (const Input &inp) const {
    return vfManager->getTarget(inp);
}

const Input &ValueFederate::getInput (const std::string &key) const
{
    return vfManager->getInput (key);
}

Input &ValueFederate::getInput (const std::string &key) { return vfManager->getInput (key); }

const Input &ValueFederate::getInput (const std::string &key, int index1) const
{
    return vfManager->getInput (key + '_' + std::to_string (index1));
}

const Input & ValueFederate::getInput (const std::string &key, int index1, int index2) const
{
    return vfManager->getInput (key + '_' + std::to_string (index1) + '_' + std::to_string (index2));
}

const Input & ValueFederate::getSubscription(const std::string &key) const
{
    return vfManager->getSubscription(key);
}

Input &ValueFederate::getSubscription (const std::string &key)
{
    return vfManager->getSubscription (key);
}

const std::string &ValueFederate::getPublicationKey (const Publication &pub) const
{
    return pub.getName();
}


Publication &ValueFederate::getPublication (const std::string &key)
{
    auto &pub = vfManager->getPublication (key);
    if (!pub.isValid ())
    {
        pub = vfManager->getPublication (getName () + separator_ + key);
    }
    return pub;
}

const Publication & ValueFederate::getPublication (const std::string &key) const
{
    auto &pub = vfManager->getPublication (key);
    if (!pub.isValid())
    {
        pub = vfManager->getPublication (getName () + separator_ + key);
    }
    return pub;
}

const Publication & ValueFederate::getPublication (const std::string &key, int index1) const
{
    return vfManager->getPublication (key + '_' + std::to_string (index1));
}

const Publication &ValueFederate::getPublication (const std::string &key, int index1, int index2) const
{
    return vfManager->getPublication (key + '_' + std::to_string (index1) + '_' + std::to_string (index2));
}

const std::string &ValueFederate::getInputUnits (const Input &inp) const {
    return vfManager->getInputUnits (inp);
}
const std::string &ValueFederate::getPublicationUnits (const Publication &pub) const
{
    return vfManager->getPublicationUnits (pub);
}

const std::string &ValueFederate::getInputType (const Input &inp) const {
    return vfManager->getInputType (inp);
}
const std::string &ValueFederate::getPublicationType (const Publication &pub) const
{
    return vfManager->getPublicationType (pub);
}

std::string ValueFederate::getPublicationType (const Input &inp) const
{
    return vfManager->getPublicationType (inp);
}


void ValueFederate::setPublicationOption (const Publication &pub, int32_t option, bool option_value)
{
    vfManager->setPublicationOption(pub, option, option_value);
}

void ValueFederate::setInputOption (const Input &inp, int32_t option, bool option_value)
{
    vfManager->setInputOption(inp, option, option_value);
}

bool ValueFederate::getInputOption(const Input &inp, int32_t option) const
{
    return vfManager->getInputOption(inp, option);
}

bool ValueFederate::getPublicationOption (const Publication &pub, int32_t option) const
{
    return vfManager->getPublicationOption(pub, option);
}

void ValueFederate::registerInputNotificationCallback (
  std::function<void(Input &, Time)> callback)
{
    vfManager->registerCallback (callback);
}

void ValueFederate::registerInputNotificationCallback (
  Input &inp,
  std::function<void(Input &, Time)> callback)
{
    vfManager->registerCallback (inp, callback);
}

/** get a count of the number publications registered*/
int ValueFederate::getPublicationCount () const { return vfManager->getPublicationCount (); }
/** get a count of the number subscriptions registered*/
int ValueFederate::getInputCount () const { return vfManager->getInputCount (); }
}  // namespace helics
