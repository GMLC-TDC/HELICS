/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "../core/core-exceptions.hpp"
#include "../core/queryHelpers.hpp"
#include "Inputs.hpp"
#include "Publications.hpp"
#include "ValueFederateManager.hpp"

namespace helics
{
ValueFederateManager::ValueFederateManager (Core *coreOb, ValueFederate *vfed, federate_id_t id)
    : coreObject (coreOb),fed(vfed), fedID (id)
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

Publication &ValueFederateManager::registerPublication (const std::string &key,
                                                        const std::string &type,
                                                        const std::string &units)
{
    auto sz = getTypeSize (type);
    auto coreID = coreObject->registerPublication (fedID, key, type, units);

    auto pubHandle = publications.lock ();
    if (!key.empty ())
    {
		pubHandle->insert(key, coreID, fed, coreID, key, type, units);
	}
	else
	{
        pubHandle->insert (nullptr, coreID, fed, coreID, key, type, units);
	}
   
    return pubHandle->back ();
}

Input &
ValueFederateManager::registerInput (const std::string &key, const std::string &type, const std::string &units)
{
    auto coreID = coreObject->registerInput (fedID, key, type, units);
    auto inpHandle = inputs.lock ();
    bool active = false;
    if (!key.empty ())
    {
        active=inpHandle->insert (key, coreID, fed, coreID, key, type, units);
    }
    else
    {
        active=inpHandle->insert (nullptr, coreID,fed,coreID, key, type, units);
    }
	if (active)
	{
        auto &ref = inpHandle->back ();
        auto edat = std::make_unique<input_info> ();
        // non-owning pointer
        ref.dataReference = edat.get ();
        auto datHandle = inputData.lock ();
        datHandle->push_back (std::move (edat));
        ref.referenceIndex = static_cast<int> (datHandle->size () - 1);
        return ref;
	}
	else
	{
        throw (RegistrationFailure ("Unable to register Input"));
	}
    
}

void ValueFederateManager::addShortcut (Input &inp, const std::string &shortcutName)
{
    if (inp.isValid ())
    {
        auto inpHandle = inputs.lock ();
        inpHandle->addSearchTerm (shortcutName, inp.handle);
        targetIDs.emplace (shortcutName, inp.handle);
    }
    else
    {
        throw (InvalidIdentifier ("input id is invalid"));
    }
}

void ValueFederateManager::addTarget (Publication &pub, const std::string &target)
{
    coreObject->addDestinationTarget (pub.handle, target);
    targetIDs.emplace (pub.handle, target);
}

void ValueFederateManager::addTarget (Input &inp, const std::string &target)
{
        coreObject->addSourceTarget (inp.handle, target);
        targetIDs.emplace (target, inp.handle);
        inputTargets.emplace (inp.handle, target);
}

void ValueFederateManager::setDefaultValue (Input &inp, const data_view &block)
{
    if (inp.isValid())
    {
        auto inpData = inputData.lock ();
        /** copy the data first since we are not entirely sure of the lifetime of the data_view*/
        (*inpData)[inp.referenceIndex]->lastData = data_view (std::make_shared<data_block> (block.data (), block.size ()));
        (*inpData)[inp.referenceIndex]->lastUpdate = CurrentTime;
    }
    else
    {
        throw (InvalidIdentifier ("Input id is invalid"));
    }
}

/** we have a new message from the core*/
void ValueFederateManager::getUpdateFromCore (interface_handle updatedHandle)
{
    auto data = coreObject->getValue (updatedHandle);
    auto inpHandle = inputs.lock ();
    /** find the id*/
    auto fid = inpHandle->find (updatedHandle);
    if (fid != inpHandle->end ())
    {  // assign the data

        lastData[fid->id.value ()] = data_view (std::move (data));
        fid->lastUpdate = CurrentTime;
    }
}

data_view ValueFederateManager::getValue (Input &inp)
{
    auto iData = reinterpret_cast<input_info *> (inp.dataReference);
	if (iData != nullptr)
	{
        iData->lastQuery = CurrentTime;
        iData->hasUpdate = false;
        return iData->lastData;
	}
    return data_view ();
}

/** function to check if the size is valid for the given type*/
inline bool isBlockSizeValid (int size, const publication_info &pubI)
{
    return ((pubI.size < 0) || (pubI.size == size));
}

void ValueFederateManager::publish (Publication &pub, const data_view &block)
{
  
        auto pubHandle = publications.lock ();
        if (isBlockSizeValid (static_cast<int> (block.size ()), (*pubHandle)[id.value ()]))
        {
            coreObject->setValue ((*pubHandle)[id.value ()].coreID, block.data (), block.size ());
        }
        else
        {
            throw (InvalidIdentifier ("publication size is invalid"));
        }
  
}

bool ValueFederateManager::hasUpdate (const Input &inp) const
{
    auto iData = reinterpret_cast<input_info *> (inp.dataReference);
    if (iData != nullptr)
    {
        return iData->hasUpdate;
    }
    return false;
}

Time ValueFederateManager::getLastUpdateTime (const Input &inp) const
{
    auto iData = reinterpret_cast<input_info *> (inp.dataReference);
    if (iData != nullptr)
    {
       return iData->lastUpdate;
    }
    return false;
}

void ValueFederateManager::updateTime (Time newTime, Time /*oldTime*/)
{
    CurrentTime = newTime;
    auto handles = coreObject->getValueUpdates (fedID);
    // lock the data updates
    auto inpHandle = inputs.lock ();
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
                inpHandle.unlock ();  // need to free the lock
                // callbacks can do all sorts of things, best not to have it locked during the callback
                callbackFunction (fid->id, CurrentTime);
                inpHandle = inputs.lock ();
            }
            else if (allCallbackIndex >= 0)
            {
                // first copy the callback in case it gets changed via another operation
                auto allCallBackFunction = callbacks[allCallbackIndex];
                inpHandle.unlock ();  // need to free the lock
                // callbacks can do all sorts of strange things, best not to have it locked during the callback
                allCallBackFunction (fid->id, CurrentTime);
                inpHandle = inputs.lock ();
            }
        }
    }
}

void ValueFederateManager::startupToInitializeStateTransition ()
{
    // get the actual publication types
    auto inpHandle = inputs.lock ();
    inpHandle->apply ([this](auto &inp) { inp.pubtype = coreObject->getType (inp.coreID); });
}

void ValueFederateManager::initializeToExecuteStateTransition () { updateTime (0.0, 0.0); }

std::string ValueFederateManager::localQuery (const std::string &queryStr) const
{
    std::string ret;
    if (queryStr == "inputs")
    {
        ret = generateStringVector_if (inputs.lock_shared (), [](const auto &info) { return info.name; },
                                       [](const auto &info) { return (!info.name.empty ()); });
    }
    else if (queryStr == "publications")
    {
        ret = generateStringVector_if (publications.lock_shared (), [](const auto &info) { return info.name; },
                                       [](const auto &info) { return (!info.name.empty ()); });
    }
    else if (queryStr == "subscriptions")
    {
        ret = generateStringVector (targetIDs, [](const auto &target) { return target.first; });
    }
    return ret;
}

std::vector<input_id_t> ValueFederateManager::queryUpdates ()
{
    std::vector<input_id_t> updates;
    auto inpHandle = inputs.lock_shared ();
    int ii = 0;
    for (auto &sub : *inpHandle)
    {
        if (sub.hasUpdate)
        {
            updates.push_back (input_id_t(ii));
        }
        ++ii;
    }
    return updates;
}

static const std::string nullStr;

const std::string &ValueFederateManager::getTarget (const Input &inp) const
{
    auto inpHandle = inputs.lock_shared ();
    auto fnd = inputTargets.find (inp.handle);
    if (fnd != inputTargets.end ())
    {
        return fnd->second;
    }
    return nullStr;
}

const std::string &ValueFederateManager::getInputKey (const Input &inp) const
{return inp.getName();}

static const Input invalidIpt;
static Input invalidIptNC;

const Input &ValueFederateManager::getInput (const std::string &key) const
{
    auto inpHandle = inputs.lock_shared ();
    auto inpF = inpHandle->find (key);
    if (inpF != inpHandle->end ())
    {
        return *inpF;
    }
    return invalidIpt;
}

Input &ValueFederateManager::getInput (const std::string &key)
{
    auto inpHandle = inputs.lock();
    auto inpF = inpHandle->find (key);
    if (inpF != inpHandle->end ())
    {
        return *inpF;
    }
    return invalidIptNC;
}

const Input &ValueFederateManager::getSubscription (const std::string &key) const
{
    auto res = targetIDs.equal_range (key);
    if (res.first != res.second)
    {
        return res.first->second;
    }
    return input_id_t ();
}

Input &ValueFederateManager::getSubscription (const std::string &key)
{
    auto res = targetIDs.equal_range (key);
    if (res.first != res.second)
    {
        return res.first->second;
    }
    return input_id_t ();
}

const std::string &ValueFederateManager::getPublicationKey (const Publication &pub) const
{
    return pub.getName ();
}

const Publication &ValueFederateManager::getPublication (const std::string &key) const
{
    auto pubHandle = publications.lock_shared ();
    auto pub = pubHandle->find (key);
    if (pub != pubHandle->end ())
    {
        return *pub;
    }

    return publication_id_t ();
}

const Publication &ValueFederateManager::getPublication (const std::string &key)
{
    auto pubHandle = publications.lock();
    auto pub = pubHandle->find (key);
    if (pub != pubHandle->end ())
    {
        return *pub;
    }

    return publication_id_t ();
}

const std::string &ValueFederateManager::getInputUnits (const Input &inp) const
{
    auto inpHandle = inputs.lock_shared ();
    return (isValidIndex (input_id.value (), *inpHandle)) ? (*inpHandle)[input_id.value ()].units : nullStr;
}

const std::string &ValueFederateManager::getPublicationUnits (const Publication &pub) const
{
    auto pubHandle = publications.lock_shared ();
    return (isValidIndex (pub_id.value (), *pubHandle)) ? (*pubHandle)[pub_id.value ()].units : nullStr;
}

const std::string &ValueFederateManager::getInputType (const Input &inp) const
{
    auto inpHandle = inputs.lock_shared ();
    return (isValidIndex (input_id.value (), *inpHandle)) ? (*inpHandle)[input_id.value ()].type : nullStr;
}

std::string ValueFederateManager::getPublicationType (const Input &inp) const
{
    auto inpHandle = inputs.lock_shared ();
    if (isValidIndex (input_id.value (), *inpHandle))
    {
        if ((*inpHandle)[input_id.value ()].pubtype == "def")
        {
            return coreObject->getType ((*inpHandle)[input_id.value ()].coreID);
        }
        else
        {
            return (*inpHandle)[input_id.value ()].pubtype;
        }
    }
    return nullStr;
}

const std::string &ValueFederateManager::getPublicationType (const Publication &pub) const
{
    auto pubHandle = publications.lock_shared ();
    return (isValidIndex (pub_id.value (), *pubHandle)) ? (*pubHandle)[pub_id.value ()].type : nullStr;
}

void ValueFederateManager::setPublicationOption (Publication &pub, int32_t option, bool option_value)
{
        coreObject->setHandleOption (pub.handle, option, option_value);
   
}

void ValueFederateManager::setInputOption (Input &inp, int32_t option, bool option_value)
{
        coreObject->setHandleOption (inp.handle, option, option_value);
}

bool ValueFederateManager::getInputOption (const Input &inp, int32_t option) const
{
    return coreObject->getHandleOption (inp.handle, option);
  
}

bool ValueFederateManager::getPublicationOption (const Publication &pub, int32_t option) const
{
   
        return coreObject->getHandleOption (pub.handle, option);
    
}

/** get a count of the number publications registered*/
int ValueFederateManager::getPublicationCount () const { return static_cast<int> (publicationCount); }
/** get a count of the number inputs registered*/
int ValueFederateManager::getInputCount () const { return static_cast<int> (inputCount); }

void ValueFederateManager::registerCallback (std::function<void(Input &, Time)> callback)
{
    auto inpHandle = inputs.lock ();
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

void ValueFederateManager::registerCallback (Input &inp, std::function<void(Input &, Time)> callback)
{
    if (id.value () < inputCount)
    {
        auto inpHandle = inputs.lock ();
        (*inpHandle)[id.value ()].callbackIndex = static_cast<int> (callbacks.size ());
        callbacks.emplace_back (std::move (callback));
    }
    else
    {
        throw (InvalidIdentifier ("Input Id is invalid"));
    }
}

}  // namespace helics
