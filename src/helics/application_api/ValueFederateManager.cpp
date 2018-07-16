/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "ValueFederateManager.hpp"

namespace helics
{
ValueFederateManager::ValueFederateManager (Core *coreOb, federate_id_t id) : coreObject (coreOb), fedID (id)
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
    auto coreID = coreObject->registerPublication (fedID, key, type, units);

    auto pubHandle = publications.lock();
    publication_id_t id = static_cast<identifier_type> (pubHandle->size ());
    ++publicationCount;
    pubHandle->insert (key,coreID, key, type, units);
    pubHandle->back ().id = id;
    pubHandle->back ().size = sz;
    pubHandle->back ().coreID = coreID;
    return id;
}

input_id_t ValueFederateManager::registerInput (const std::string &key,
                                                                      const std::string &type,
                                                                      const std::string &units)
{
    auto coreID = coreObject->registerInput (fedID, key, type, units);
    auto inpHandle = inputs.lock();
    input_id_t id = static_cast<identifier_type> (inpHandle->size ());
    ++inputCount;
	if (!key.empty())
	{
        inpHandle->insert (key, coreID, key, type, units);
	}
	else
	{
        inpHandle->insert (nullptr, coreID, key, type, units);
	}
    
    inpHandle->back ().id = id;
    inpHandle->back ().coreID = coreID;
    lastData.resize (id.value () + 1);
    return id;
}

void ValueFederateManager::addShortcut (input_id_t subid, const std::string &shortcutName)
{
    if (subid.value () < inputCount)
    {
        auto inpHandle = inputs.lock();
        inpHandle->addSearchTermForIndex (shortcutName, subid.value ());
    }
    else
    {
        throw (std::invalid_argument ("subscription id is invalid"));
    }
}

void ValueFederateManager::setDefaultValue (input_id_t id, const data_view &block)
{
    if (id.value () < inputCount)
    {
        auto inpHandle = inputs.lock();
        /** copy the data first since we are not entirely sure of the lifetime of the data_view*/
        lastData[id.value ()] = data_view (std::make_shared<data_block> (block.data (), block.size ()));
        (*inpHandle)[id.value ()].lastUpdate = CurrentTime;
    }
    else
    {
        throw (std::invalid_argument ("subscription id is invalid"));
    }
}

/** we have a new message from the core*/
void ValueFederateManager::getUpdateFromCore (interface_handle updatedHandle)
{
    auto data = coreObject->getValue (updatedHandle);
    auto inpHandle = inputs.lock();
    /** find the id*/
    auto fid = inpHandle->find (updatedHandle);
    if (fid != inpHandle->end ())
    {  // assign the data
        
        lastData[fid->id.value ()] = data_view (std::move (data));
        fid->lastUpdate = CurrentTime;
    }
}

data_view ValueFederateManager::getValue (input_id_t id)
{
    if (id.value () < inputCount)
    {
        auto inpHandle = inputs.lock();
        (*inpHandle)[id.value ()].lastQuery = CurrentTime;
        (*inpHandle)[id.value ()].hasUpdate = false;
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
        auto pubHandle = publications.lock();
        if (isBlockSizeValid (static_cast<int> (block.size ()), (*pubHandle)[id.value ()]))
        {
            coreObject->setValue ((*pubHandle)[id.value ()].coreID, block.data (), block.size ());
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

bool ValueFederateManager::hasUpdate (input_id_t input_id) const
{
    if (input_id.value () < inputCount)
    {
        auto inpHandle = inputs.lock_shared();
        return (*inpHandle)[input_id.value ()].hasUpdate;
    }
    return false;
}

Time ValueFederateManager::getLastUpdateTime (input_id_t input_id) const
{
    if (input_id.value () < inputCount)
    {
        auto inpHandle = inputs.lock_shared();
        return (*inpHandle)[input_id.value ()].lastUpdate;
    }
    return false;
}

void ValueFederateManager::updateTime (Time newTime, Time /*oldTime*/)
{
    CurrentTime = newTime;
    auto handles = coreObject->getValueUpdates (fedID);
    // lock the data updates
    auto inpHandle = inputs.lock();
    for (auto handle : handles)
    {
        /** find the id*/
        auto fid = inpHandle->find (handle);
        if (fid != inpHandle->end ())
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
                inpHandle.unlock();  //need to free the lock
                // callbacks can do all sorts of things, best not to have it locked during the callback
                callbackFunction (fid->id, CurrentTime);
                inpHandle = inputs.lock();
            }
            else if (allCallbackIndex >= 0)
            {
                // first copy the callback in case it gets changed via another operation
                auto allCallBackFunction = callbacks[allCallbackIndex];
                inpHandle.unlock();  //need to free the lock
                // callbacks can do all sorts of strange things, best not to have it locked during the callback
                allCallBackFunction (fid->id, CurrentTime);
                inpHandle = inputs.lock();
            }
        }
    }
}

void ValueFederateManager::startupToInitializeStateTransition ()
{
    lastData.resize (inputCount);
    // get the actual publication types
    auto inpHandle = inputs.lock();
    inpHandle->apply ([this](auto &sub) { sub.pubtype = coreObject->getType (sub.coreID); });
}

void ValueFederateManager::initializeToExecuteStateTransition () { updateTime (0.0, 0.0); }

std::vector<input_id_t> ValueFederateManager::queryUpdates ()
{
    std::vector<input_id_t> updates;
    auto inpHandle = inputs.lock_shared();
    for (auto &sub : *inpHandle)
    {
        if (sub.hasUpdate)
        {
            updates.push_back (sub.id);
        }
    }
    return updates;
}

static const std::string nullStr;

std::string ValueFederateManager::getInputKey (input_id_t input_id) const
{
    auto inpHandle = inputs.lock_shared();
    return (input_id.value () < inpHandle->size ()) ? (*inpHandle)[input_id.value ()].name : nullStr;
}

input_id_t ValueFederateManager::getInputId (const std::string &key) const
{
    auto inpHandle = inputs.lock_shared();
    auto sub = inpHandle->find (key);
    if (sub != inpHandle->end ())
    {
        return sub->id;
    }
    return invalid_id_value;
}

std::string ValueFederateManager::getPublicationKey (publication_id_t pub_id) const
{
    auto pubHandle = publications.lock_shared();
    return (pub_id.value () < pubHandle->size ()) ? (*pubHandle)[pub_id.value ()].name : nullStr;
}

publication_id_t ValueFederateManager::getPublicationId (const std::string &key) const
{
    auto pubHandle = publications.lock_shared();
    auto pub = pubHandle->find (key);
    if (pub != pubHandle->end ())
    {
        return pub->id;
    }

    return invalid_id_value;
}

std::string ValueFederateManager::getInputUnits (input_id_t input_id) const
{
    auto inpHandle = inputs.lock_shared();
    return (input_id.value () < inpHandle->size ()) ? (*inpHandle)[input_id.value ()].units : nullStr;
}

std::string ValueFederateManager::getPublicationUnits (publication_id_t pub_id) const
{
    auto pubHandle = publications.lock_shared();
    return (pub_id.value () < pubHandle->size ()) ? (*pubHandle)[pub_id.value ()].units : nullStr;
}

std::string ValueFederateManager::getInputType (input_id_t input_id) const
{
    auto inpHandle = inputs.lock_shared();
    return (input_id.value () < inpHandle->size ()) ? (*inpHandle)[input_id.value ()].type : nullStr;
}

std::string ValueFederateManager::getPublicationType (input_id_t input_id) const
{
    auto inpHandle = inputs.lock_shared();
    return (input_id.value () < inpHandle->size ()) ? (*inpHandle)[input_id.value ()].pubtype : nullStr;
}

std::string ValueFederateManager::getPublicationType (publication_id_t pub_id) const
{
    auto pubHandle = publications.lock_shared();
    return (pub_id.value () < pubHandle->size ()) ? (*pubHandle)[pub_id.value ()].type : nullStr;
}

/** get a count of the number publications registered*/
int ValueFederateManager::getPublicationCount () const { return static_cast<int> (publicationCount); }
/** get a count of the number inputs registered*/
int ValueFederateManager::getInputCount () const { return static_cast<int> (inputCount); }

void ValueFederateManager::registerCallback (std::function<void(input_id_t, Time)> callback)
{
    auto inpHandle = inputs.lock();
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

void ValueFederateManager::registerCallback (input_id_t id,
                                             std::function<void(input_id_t, Time)> callback)
{
    if (id.value () < inputCount)
    {
        auto inpHandle = inputs.lock();
        (*inpHandle)[id.value ()].callbackIndex = static_cast<int> (callbacks.size ());
        callbacks.emplace_back (std::move (callback));
    }
    else
    {
        throw (std::invalid_argument ("subscription id is invalid"));
    }
}

void ValueFederateManager::registerCallback (const std::vector<input_id_t> &ids,
                                             std::function<void(input_id_t, Time)> callback)
{
    auto inpHandle = inputs.lock();
    int ind = static_cast<int> (callbacks.size ());
    callbacks.emplace_back (std::move (callback));
    for (auto id : ids)
    {
        if (id.value () < inpHandle->size ())
        {
            (*inpHandle)[id.value ()].callbackIndex = ind;
        }
    }
}
}  // namespace helics
