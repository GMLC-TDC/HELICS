/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "ValueFederateManager.h"

namespace helics
{
ValueFederateManager::ValueFederateManager (std::shared_ptr<Core> &coreOb, Core::federate_id_t id)
    : coreObject (coreOb), fedID (id)
{
}
ValueFederateManager::~ValueFederateManager () = default;

void ValueFederateManager::disconnect ()
{
    // checks for the calls are handled in the MessageFederate itself
    coreObject = nullptr;
}

static const std::map<std::string, int> typeSizes = {
  {"char", 2},      {"uchar", 2},     {"block_4", 5},  {"block_8", 9},   {"block_12", 13}, {"block_16", 17},
  {"block_20", 24}, {"block_24", 30}, {"double", 9},   {"float", 5},     {"int32", 5},     {"uint32", 5},
  {"int64", 9},     {"uint64", 9},    {"complex", 17}, {"complex_f", 9},
};

int getTypeSize (const std::string type)
{
    auto ret = typeSizes.find (type);
    return (ret == typeSizes.end ()) ? (-1) : ret->second;
}

publication_id_t ValueFederateManager::registerPublication (const std::string &name,
                                                            const std::string &type,
                                                            const std::string &units)
{
    auto sz = getTypeSize (type);
    std::lock_guard<std::mutex> publock (publication_mutex);
    publication_id_t id = static_cast<identifier_type> (pubs.size ());
    pubs.emplace_back (name, type, units);
    pubs.back ().id = id;
    pubs.back ().size = sz;
    publicationNames.emplace (name, id);
    pubs.back ().coreID = coreObject->registerPublication (fedID, name, type, units);
    return id;
}

subscription_id_t ValueFederateManager::registerRequiredSubscription (const std::string &name,
                                                                      const std::string &type,
                                                                      const std::string &units)
{
    std::lock_guard<std::mutex> sublock (subscription_mutex);
    subscription_id_t id = static_cast<identifier_type> (subs.size ());
    subs.emplace_back (name, type, units);
    subs.back ().id = id;
    subscriptionNames.emplace (name, id);
    subs.back ().coreID = coreObject->registerSubscription (fedID, name, type, units, handle_check_mode::required);
    handleLookup.emplace (subs.back ().coreID, id);
    lastData.resize (id.value () + 1);
    return id;
}

subscription_id_t ValueFederateManager::registerOptionalSubscription (const std::string &name,
                                                                      const std::string &type,
                                                                      const std::string &units)
{
    std::lock_guard<std::mutex> sublock (subscription_mutex);
    subscription_id_t id = static_cast<identifier_type> (subs.size ());
    subs.emplace_back (name, type, units);
    subs.back ().id = id;
    subscriptionNames.emplace (name, id);
    subs.back ().coreID = coreObject->registerSubscription (fedID, name, type, units, handle_check_mode::optional);
    handleLookup.emplace (subs.back ().coreID, id);
    lastData.resize (id.value () + 1);
    return id;
}

void ValueFederateManager::addSubscriptionShortcut (subscription_id_t subid, const std::string &shortcutName)
{
    if (subid.value () < subs.size ())
    {
        std::lock_guard<std::mutex> sublock (subscription_mutex);
        subscriptionNames.emplace (shortcutName, subid);
    }
    else
    {
        throw (std::invalid_argument ("subscription id is invalid"));
    }
}

void ValueFederateManager::setDefaultValue (subscription_id_t id, data_view block)
{
    if (id.value () < subs.size ())
    {
        std::lock_guard<std::mutex> sublock (subscription_mutex);
        /** copy the data first since we are not entirely sure of the lifetime of the data_view*/
        lastData[id.value ()] = data_view (std::make_shared<data_block> (block.data (), block.size ()));
        subs[id.value ()].lastUpdate = CurrentTime;
    }
    else
    {
        throw (std::invalid_argument ("subscription id is invalid"));
    }
}

/** we have a new message from the core*/
void ValueFederateManager::getUpdateFromCore (Core::Handle updatedHandle)
{
    auto data = coreObject->getValue (updatedHandle);

    /** find the id*/
    auto fid = handleLookup.find (updatedHandle);
    if (fid != handleLookup.end ())
    {  // assign the data
        std::lock_guard<std::mutex> sublock (subscription_mutex);
        lastData[fid->second.value ()] = data_view (std::move (data));
        subs[fid->second.value ()].lastUpdate = CurrentTime;
    }
}

data_view ValueFederateManager::getValue (subscription_id_t id)
{
    if (id.value () < subs.size ())
    {
        std::lock_guard<std::mutex> sublock (subscription_mutex);
        subs[id.value ()].lastQuery = CurrentTime;
        return lastData[id.value ()];
    }
    else
    {
        throw (std::invalid_argument ("subscription id is invalid"));
    }
}

/** function to check if the size is valid for the given type*/
inline bool isBlockSizeValid (int size, const publication_info &pubI)
{
    return ((pubI.size < 0) || (pubI.size == size));
}

void ValueFederateManager::publish (publication_id_t id, data_view block)
{
    if (id.value () < pubs.size ())
    {  // send directly to the core
        if (isBlockSizeValid (static_cast<int> (block.size ()), pubs[id.value ()]))
        {
            coreObject->setValue (pubs[id.value ()].coreID, block.data (), block.size ());
        }
        else
        {
            throw (std::invalid_argument ("publication size is invalid"));
        }
    }
    else
    {
        throw (std::invalid_argument ("publication id is invalid"));
    }
}

bool ValueFederateManager::queryUpdate (subscription_id_t sub_id) const
{
    if (sub_id.value () < subs.size ())
    {
        std::lock_guard<std::mutex> sublock (subscription_mutex);
        return subs[sub_id.value ()].lastQuery < subs[sub_id.value ()].lastUpdate;
    }
    return false;
}

Time ValueFederateManager::queryLastUpdate (subscription_id_t sub_id) const
{
    if (sub_id.value () < subs.size ())
    {
        std::lock_guard<std::mutex> sublock (subscription_mutex);
        return subs[sub_id.value ()].lastUpdate;
    }
    return false;
}

void ValueFederateManager::updateTime (Time newTime, Time /*oldTime*/)
{
    CurrentTime = newTime;
    auto handles = coreObject->getValueUpdates (fedID);
    // lock the data updates
    std::unique_lock<std::mutex> sublock (subscription_mutex);
    for (auto handle : handles)
    {
        /** find the id*/
        auto fid = handleLookup.find (handle);
        if (fid != handleLookup.end ())
        {  // assign the data
            auto data = coreObject->getValue (handle);

            auto subIndex = fid->second.value ();
            // move the data into the container
            lastData[subIndex] = std::move (data);
            subs[subIndex].lastUpdate = CurrentTime;
            if (subs[subIndex].callbackIndex >= 0)
            {
                // first copy the callback in case it gets changed via another operation
                auto callbackFunction = callbacks[subs[subIndex].callbackIndex];
                sublock.unlock ();
                // callbacks can do all sorts of things, best not to have it locked during the callback
                callbackFunction (fid->second, CurrentTime);
                sublock.lock ();
            }
            else if (allCallbackIndex >= 0)
            {
                // first copy the callback in case it gets changed via another operation
                auto allCallBackFunction = callbacks[allCallbackIndex];
                sublock.unlock ();
                // callbacks can do all sorts of strange things, best not to have it locked during the callback
                allCallBackFunction (fid->second, CurrentTime);
                sublock.lock ();
            }
        }
    }
}

void ValueFederateManager::StartupToInitializeStateTransition ()
{
    lastData.resize (subs.size ());
    // get the actual publication types
    for (auto &sub : subs)
    {
        sub.pubtype = coreObject->getType (sub.coreID);
    }
}

void ValueFederateManager::InitializeToExecuteStateTransition () { updateTime (0.0, 0.0); }

std::vector<subscription_id_t> ValueFederateManager::queryUpdates ()
{
    std::vector<subscription_id_t> updates;
    std::lock_guard<std::mutex> sublock (subscription_mutex);
    for (auto &sub : subs)
    {
        if (sub.lastUpdate > sub.lastQuery)
        {
            updates.push_back (sub.id);
        }
    }
    return updates;
}

static const std::string nullStr;

std::string ValueFederateManager::getSubscriptionName (subscription_id_t sub_id) const
{
    std::lock_guard<std::mutex> sublock (subscription_mutex);
    return (sub_id.value () < subs.size ()) ? subs[sub_id.value ()].name : nullStr;
}

subscription_id_t ValueFederateManager::getSubscriptionId (const std::string &name) const
{
    std::lock_guard<std::mutex> sublock (subscription_mutex);
    auto sub = subscriptionNames.find (name);
    if (sub != subscriptionNames.end ())
    {
        return sub->second;
    }
    return invalid_id_value;
}

std::string ValueFederateManager::getPublicationName (publication_id_t pub_id) const
{
    std::lock_guard<std::mutex> publock (publication_mutex);
    return (pub_id.value () < pubs.size ()) ? pubs[pub_id.value ()].name : nullStr;
}

publication_id_t ValueFederateManager::getPublicationId (const std::string &name) const
{
    std::lock_guard<std::mutex> publock (publication_mutex);
    auto pub = publicationNames.find (name);
    if (pub != publicationNames.end ())
    {
        return pub->second;
    }

    return invalid_id_value;
}

std::string ValueFederateManager::getSubscriptionUnits (subscription_id_t sub_id) const
{
    std::lock_guard<std::mutex> sublock (subscription_mutex);
    return (sub_id.value () < subs.size ()) ? subs[sub_id.value ()].units : nullStr;
}

std::string ValueFederateManager::getPublicationUnits (publication_id_t pub_id) const
{
    std::lock_guard<std::mutex> publock (publication_mutex);
    return (pub_id.value () < pubs.size ()) ? pubs[pub_id.value ()].units : nullStr;
}

std::string ValueFederateManager::getSubscriptionType (subscription_id_t sub_id) const
{
    std::lock_guard<std::mutex> sublock (subscription_mutex);
    return (sub_id.value () < subs.size ()) ? subs[sub_id.value ()].type : nullStr;
}

std::string ValueFederateManager::getPublicationType (subscription_id_t sub_id) const
{
    std::lock_guard<std::mutex> sublock (subscription_mutex);
    return (sub_id.value () < subs.size ()) ? subs[sub_id.value ()].pubtype : nullStr;
}

std::string ValueFederateManager::getPublicationType (publication_id_t pub_id) const
{
    std::lock_guard<std::mutex> publock (publication_mutex);
    return (pub_id.value () < pubs.size ()) ? pubs[pub_id.value ()].type : nullStr;
}

void ValueFederateManager::registerCallback (std::function<void(subscription_id_t, Time)> callback)
{
    std::lock_guard<std::mutex> sublock (subscription_mutex);
    if (allCallbackIndex >= 0)
    {
        callbacks[allCallbackIndex] = std::move (callback);
    }
    else
    {
        allCallbackIndex = static_cast<int> (callbacks.size ());
        callbacks.emplace_back (std::move (callback));
    }
}

void ValueFederateManager::registerCallback (subscription_id_t id,
                                             std::function<void(subscription_id_t, Time)> callback)
{
    if (id.value () < subs.size ())
    {
        std::lock_guard<std::mutex> sublock (subscription_mutex);
        subs[id.value ()].callbackIndex = static_cast<int> (callbacks.size ());
        callbacks.emplace_back (std::move (callback));
    }
    else
    {
        throw (std::invalid_argument ("subscription id is invalid"));
    }
}

void ValueFederateManager::registerCallback (const std::vector<subscription_id_t> &ids,
                                             std::function<void(subscription_id_t, Time)> callback)
{
    std::lock_guard<std::mutex> sublock (subscription_mutex);
    int ind = static_cast<int> (callbacks.size ());
    callbacks.emplace_back (std::move (callback));
    for (auto id : ids)
    {
        if (id.value () < subs.size ())
        {
            subs[id.value ()].callbackIndex = ind;
        }
    }
}
}  // namespace helics
