/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
*/
#include "ValueFederateManager.hpp"

namespace helics
{
ValueFederateManager::ValueFederateManager (Core *coreOb, Core::federate_id_t id)
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

int getTypeSize (const std::string &type)
{
    auto ret = typeSizes.find (type);
    return (ret == typeSizes.end ()) ? (-1) : ret->second;
}

publication_id_t ValueFederateManager::registerPublication (const std::string &key,
                                                            const std::string &type,
                                                            const std::string &units)
{
    auto sz = getTypeSize (type);
	auto coreID = coreObject->registerPublication(fedID, key, type, units);

    std::lock_guard<std::mutex> publock (publication_mutex);
    publication_id_t id = static_cast<identifier_type> (publications.size ());
    ++publicationCount;
	publications.insert(key, key, type,units);
    publications.back ().id = id;
    publications.back ().size = sz;
    publications.back ().coreID = coreID;
    return id;
}

subscription_id_t ValueFederateManager::registerRequiredSubscription (const std::string &key,
                                                                      const std::string &type,
                                                                      const std::string &units)
{
	auto coreID= coreObject->registerSubscription(fedID, key, type, units, handle_check_mode::required);
    std::lock_guard<std::mutex> sublock (subscription_mutex);
    subscription_id_t id = static_cast<identifier_type> (subscriptions.size ());
    ++subscriptionCount;
    subscriptions.insert(key,coreID,key, type, units);
	subscriptions.back ().id = id;
	subscriptions.back().coreID = coreID;
    lastData.resize (id.value () + 1);
    return id;
}

subscription_id_t ValueFederateManager::registerOptionalSubscription (const std::string &key,
                                                                      const std::string &type,
                                                                      const std::string &units)
{

	auto coreID = coreObject->registerSubscription(fedID, key, type, units, handle_check_mode::optional);
	std::lock_guard<std::mutex> sublock(subscription_mutex);
	subscription_id_t id = static_cast<identifier_type> (subscriptions.size());
    ++subscriptionCount;
	subscriptions.insert(key, coreID, key, type, units);
	subscriptions.back().id = id;
	subscriptions.back().coreID = coreID;
	lastData.resize(id.value() + 1);
	return id;

}

void ValueFederateManager::addSubscriptionShortcut (subscription_id_t subid, const std::string &shortcutName)
{
    if (subid.value () < subscriptionCount)
    {
        std::lock_guard<std::mutex> sublock (subscription_mutex);
       subscriptions.addSearchTermForIndex (shortcutName, subid.value());
    }
    else
    {
        throw (std::invalid_argument ("subscription id is invalid"));
    }
}

void ValueFederateManager::setDefaultValue (subscription_id_t id, const data_view &block)
{
    if (id.value () < subscriptionCount)
    {
        std::lock_guard<std::mutex> sublock (subscription_mutex);
        /** copy the data first since we are not entirely sure of the lifetime of the data_view*/
        lastData[id.value ()] = data_view (std::make_shared<data_block> (block.data (), block.size ()));
        subscriptions[id.value ()].lastUpdate = CurrentTime;
    }
    else
    {
        throw (std::invalid_argument ("subscription id is invalid"));
    }
}

/** we have a new message from the core*/
void ValueFederateManager::getUpdateFromCore (Core::handle_id_t updatedHandle)
{
    auto data = coreObject->getValue (updatedHandle);

    /** find the id*/
    auto fid = subscriptions.find (updatedHandle);
    if (fid != subscriptions.end ())
    {  // assign the data
        std::lock_guard<std::mutex> sublock (subscription_mutex);
        lastData[fid->id.value()] = data_view (std::move (data));
        fid->lastUpdate = CurrentTime;
    }
}

data_view ValueFederateManager::getValue (subscription_id_t id)
{
    if (id.value () < subscriptionCount)
    {
        std::lock_guard<std::mutex> sublock (subscription_mutex);
		subscriptions[id.value ()].lastQuery = CurrentTime;
		subscriptions[id.value ()].hasUpdate = false;
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

void ValueFederateManager::publish (publication_id_t id, const data_view &block)
{
    if (id.value () < publicationCount)
    {  // send directly to the core
        if (isBlockSizeValid (static_cast<int> (block.size ()), publications[id.value ()]))
        {
            coreObject->setValue (publications[id.value ()].coreID, block.data (), block.size ());
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
    if (sub_id.value () < subscriptionCount)
    {
        std::lock_guard<std::mutex> sublock (subscription_mutex);
        return subscriptions[sub_id.value ()].hasUpdate;
    }
    return false;
}

Time ValueFederateManager::queryLastUpdate (subscription_id_t sub_id) const
{
    if (sub_id.value () < subscriptionCount)
    {
        std::lock_guard<std::mutex> sublock (subscription_mutex);
        return subscriptions[sub_id.value ()].lastUpdate;
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
        auto fid = subscriptions.find (handle);
        if (fid != subscriptions.end ())
        {  // assign the data
            auto data = coreObject->getValue (handle);

            auto subIndex = fid->id.value ();
            // move the data into the container
            lastData[subIndex] = std::move (data);
            fid->lastUpdate = CurrentTime;
            fid->hasUpdate = true;
            if (fid->callbackIndex >= 0)
            {
                // first copy the callback in case it gets changed via another operation
                auto callbackFunction = callbacks[fid->callbackIndex];
                sublock.unlock ();
                // callbacks can do all sorts of things, best not to have it locked during the callback
                callbackFunction (fid->id, CurrentTime);
                sublock.lock ();
            }
            else if (allCallbackIndex >= 0)
            {
                // first copy the callback in case it gets changed via another operation
                auto allCallBackFunction = callbacks[allCallbackIndex];
                sublock.unlock ();
                // callbacks can do all sorts of strange things, best not to have it locked during the callback
                allCallBackFunction (fid->id, CurrentTime);
                sublock.lock ();
            }
        }
    }
}

void ValueFederateManager::startupToInitializeStateTransition ()
{
    lastData.resize (subscriptionCount);
    // get the actual publication types
	subscriptions.apply([this](auto &sub) {sub.pubtype = coreObject->getType(sub.coreID); });
}

void ValueFederateManager::initializeToExecuteStateTransition () { updateTime (0.0, 0.0); }

std::vector<subscription_id_t> ValueFederateManager::queryUpdates ()
{
    std::vector<subscription_id_t> updates;
    std::lock_guard<std::mutex> sublock (subscription_mutex);
    for (auto &sub : subscriptions)
    {
        if (sub.hasUpdate)
        {
            updates.push_back (sub.id);
        }
    }
    return updates;
}

static const std::string nullStr;

std::string ValueFederateManager::getSubscriptionKey (subscription_id_t sub_id) const
{
    std::lock_guard<std::mutex> sublock (subscription_mutex);
    return (sub_id.value () < subscriptions.size ()) ? subscriptions[sub_id.value ()].name : nullStr;
}

subscription_id_t ValueFederateManager::getSubscriptionId (const std::string &key) const
{
    std::lock_guard<std::mutex> sublock (subscription_mutex);
    auto sub = subscriptions.find (key);
    if (sub != subscriptions.end ())
    {
        return sub->id;
    }
    return invalid_id_value;
}

std::string ValueFederateManager::getPublicationKey (publication_id_t pub_id) const
{
    std::lock_guard<std::mutex> publock (publication_mutex);
    return (pub_id.value () < publications.size ()) ? publications[pub_id.value ()].name : nullStr;
}

publication_id_t ValueFederateManager::getPublicationId (const std::string &key) const
{
    std::lock_guard<std::mutex> publock (publication_mutex);
    auto pub = publications.find (key);
    if (pub != publications.end ())
    {
        return pub->id;
    }

    return invalid_id_value;
}

std::string ValueFederateManager::getSubscriptionUnits (subscription_id_t sub_id) const
{
    std::lock_guard<std::mutex> sublock (subscription_mutex);
    return (sub_id.value () < subscriptions.size ()) ? subscriptions[sub_id.value ()].units : nullStr;
}

std::string ValueFederateManager::getPublicationUnits (publication_id_t pub_id) const
{
    std::lock_guard<std::mutex> publock (publication_mutex);
    return (pub_id.value () < publications.size ()) ? publications[pub_id.value ()].units : nullStr;
}

std::string ValueFederateManager::getSubscriptionType (subscription_id_t sub_id) const
{
    std::lock_guard<std::mutex> sublock (subscription_mutex);
    return (sub_id.value () < subscriptions.size ()) ? subscriptions[sub_id.value ()].type : nullStr;
}

std::string ValueFederateManager::getPublicationType (subscription_id_t sub_id) const
{
    std::lock_guard<std::mutex> sublock (subscription_mutex);
    return (sub_id.value () < subscriptions.size ()) ? subscriptions[sub_id.value ()].pubtype : nullStr;
}

std::string ValueFederateManager::getPublicationType (publication_id_t pub_id) const
{
    std::lock_guard<std::mutex> publock (publication_mutex);
    return (pub_id.value () < publications.size ()) ? publications[pub_id.value ()].type : nullStr;
}

/** get a count of the number publications registered*/
int ValueFederateManager::getPublicationCount () const
{
    return static_cast<int> (publicationCount);
}
/** get a count of the number subscriptions registered*/
int ValueFederateManager::getSubscriptionCount () const
{
    return static_cast<int> (subscriptionCount);
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
    if (id.value () < subscriptionCount)
    {
        std::lock_guard<std::mutex> sublock (subscription_mutex);
		subscriptions[id.value ()].callbackIndex = static_cast<int> (callbacks.size ());
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
        if (id.value () < subscriptions.size ())
        {
			subscriptions[id.value ()].callbackIndex = ind;
        }
    }
}
}  // namespace helics

