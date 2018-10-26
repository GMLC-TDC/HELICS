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
    : coreObject (coreOb), fed (vfed), fedID (id)
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
    auto coreID = coreObject->registerPublication (fedID, key, type, units);

    auto pubHandle = publications.lock ();
    stx::optional<size_t> active;
    if (!key.empty ())
    {
        active = pubHandle->insert (key, coreID, fed, coreID, key, type, units);
    }
    else
    {
        active = pubHandle->insert (nullptr, coreID, fed, coreID, key, type, units);
    }

    if (active)
    {
        return pubHandle->back ();
    }
    else
    {
        throw (RegistrationFailure ("Unable to register Publication"));
    }
}

Input &
ValueFederateManager::registerInput (const std::string &key, const std::string &type, const std::string &units)
{
    auto coreID = coreObject->registerInput (fedID, key, type, units);
    auto inpHandle = inputs.lock ();
    stx::optional<size_t> active;
    if (!key.empty ())
    {
        active = inpHandle->insert (key, coreID, fed, coreID, key);
    }
    else
    {
        active = inpHandle->insert (nullptr, coreID, fed, coreID, key);
    }
    if (active)
    {
        auto &ref = inpHandle->back ();
        auto edat = std::make_unique<input_info> (key, type, units);
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

void ValueFederateManager::addShortcut (const Input &inp, const std::string &shortcutName)
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

void ValueFederateManager::addTarget (const Publication &pub, const std::string &target)
{
    coreObject->addDestinationTarget (pub.handle, target);
    targetIDs.emplace (target, pub.handle);
}

void ValueFederateManager::addTarget (const Input &inp, const std::string &target)
{
    coreObject->addSourceTarget (inp.handle, target);
    targetIDs.emplace (target, inp.handle);
    inputTargets.emplace (inp.handle, target);
}

void ValueFederateManager::setDefaultValue (const Input &inp, const data_view &block)
{
    if (inp.isValid ())
    {
        input_info *info = reinterpret_cast<input_info *> (inp.dataReference);

        /** copy the data first since we are not entirely sure of the lifetime of the data_view*/
        info->lastData = data_view (std::make_shared<data_block> (block.data (), block.size ()));
        info->lastUpdate = CurrentTime;
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

        input_info *info = reinterpret_cast<input_info *> (fid->dataReference);
        info->lastData = data_view (std::move (data));
        info->lastUpdate = CurrentTime;
    }
}

data_view ValueFederateManager::getValue (const Input &inp)
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

void ValueFederateManager::publish (const Publication &pub, const data_view &block)
{
    coreObject->setValue (pub.handle, block.data (), block.size ());
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
	if (handles.empty())
	{
        return;
	}
    // lock the data updates
    auto inpHandle = inputs.lock ();
    auto allCall = allCallback.load ();
    for (auto handle : handles)
    {
        /** find the id*/
        auto fid = inpHandle->find (handle);
        if (fid != inpHandle->end ())
        {  // assign the data
            auto data = coreObject->getValue (handle);
            auto iData = reinterpret_cast<input_info *> (fid->dataReference);
            iData->lastData = std::move (data);
            iData->lastUpdate = CurrentTime;
            iData->hasUpdate = true;
            if (iData->callback)
            {
                Input &inp = *fid;

                inpHandle.unlock ();  // need to free the lock

                // callbacks can do all sorts of things, best not to have it locked during the callback
                iData->callback (inp, CurrentTime);
                inpHandle = inputs.lock ();
            }
            else if (allCall)
            {
                Input &inp = *fid;
                inpHandle.unlock ();  // need to free the lock
                // callbacks can do all sorts of strange things, best not to have it locked during the callback
                allCall (inp, CurrentTime);
                inpHandle = inputs.lock ();
            }
        }
    }
}

void ValueFederateManager::startupToInitializeStateTransition ()
{
    // get the actual publication types
    auto inpHandle = inputs.lock ();
    inpHandle->apply ([this](auto &inp) { inp.type = getTypeFromString (coreObject->getType (inp.handle)); });
}

void ValueFederateManager::initializeToExecuteStateTransition () { updateTime (0.0, 0.0); }

std::string ValueFederateManager::localQuery (const std::string &queryStr) const
{
    std::string ret;
    if (queryStr == "inputs")
    {
        ret = generateStringVector_if (inputs.lock_shared (), [](const auto &info) { return info.actualName; },
                                       [](const auto &info) { return (!info.actualName.empty ()); });
    }
    else if (queryStr == "publications")
    {
        ret = generateStringVector_if (publications.lock_shared (), [](const auto &info) { return info.key_; },
                                       [](const auto &info) { return (!info.key_.empty ()); });
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
            updates.push_back (input_id_t (ii));
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

const std::string &ValueFederateManager::getInputKey (const Input &inp) const { return inp.getName (); }

static const Input invalidIpt{};
static Input invalidIptNC{};

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
    auto inpHandle = inputs.lock ();
    auto inpF = inpHandle->find (key);
    if (inpF != inpHandle->end ())
    {
        return *inpF;
    }
    return invalidIptNC;
}

const Input &ValueFederateManager::getInput (int index) const
{
    auto inpHandle = inputs.lock_shared ();
    if (isValidIndex (index, *inpHandle))
    {
        return (*inpHandle)[index];
    }
    return invalidIpt;
}

Input &ValueFederateManager::getInput (int index)
{
    auto inpHandle = inputs.lock ();
    if (isValidIndex (index, *inpHandle))
    {
        return (*inpHandle)[index];
    }
    return invalidIptNC;
}

const Input &ValueFederateManager::getSubscription (const std::string &key) const
{
    auto res = targetIDs.equal_range (key);
    if (res.first != res.second)
    {
        auto inps = inputs.lock_shared ();
        auto ret = inps->find (res.first->second);
        if (ret != inps->end ())
        {
            return *ret;
        }
    }
    return invalidIpt;
}

Input &ValueFederateManager::getSubscription (const std::string &key)
{
    auto res = targetIDs.equal_range (key);
    if (res.first != res.second)
    {
        auto inps = inputs.lock ();
        auto ret = inps->find (res.first->second);
        if (ret != inps->end ())
        {
            return *ret;
        }
    }
    return invalidIptNC;
}

const std::string &ValueFederateManager::getPublicationKey (const Publication &pub) const
{
    return pub.getName ();
}

static const Publication invalidPub{};
static Publication invalidPubNC{};

const Publication &ValueFederateManager::getPublication (const std::string &key) const
{
    auto pubHandle = publications.lock_shared ();
    auto pubF = pubHandle->find (key);
    if (pubF != pubHandle->end ())
    {
        return *pubF;
    }
    return invalidPub;
}

Publication &ValueFederateManager::getPublication (const std::string &key)
{
    auto pubHandle = publications.lock ();
    auto pubF = pubHandle->find (key);
    if (pubF != pubHandle->end ())
    {
        return *pubF;
    }
    return invalidPubNC;
}

const Publication &ValueFederateManager::getPublication (int index) const
{
    auto pubHandle = publications.lock_shared ();
    if (isValidIndex (index, *pubHandle))
    {
        return (*pubHandle)[index];
    }
    return invalidPub;
}

Publication &ValueFederateManager::getPublication (int index)
{
    auto pubHandle = publications.lock ();
    if (isValidIndex (index, *pubHandle))
    {
        return (*pubHandle)[index];
    }
    return invalidPubNC;
}

const std::string &ValueFederateManager::getInputUnits (const Input &inp) const
{
    return coreObject->getUnits (inp.handle);
}

const std::string &ValueFederateManager::getPublicationUnits (const Publication &pub) const
{
    return coreObject->getUnits (pub.handle);
}

const std::string &ValueFederateManager::getInputType (const Input &inp) const
{
    return coreObject->getType (inp.handle);
}

std::string ValueFederateManager::getPublicationType (const Input &inp) const
{
    return coreObject->getType (inp.handle);
}

const std::string &ValueFederateManager::getPublicationType (const Publication &pub) const
{
    return coreObject->getType (pub.handle);
}

void ValueFederateManager::setPublicationOption (const Publication &pub, int32_t option, bool option_value)
{
    coreObject->setHandleOption (pub.handle, option, option_value);
}

void ValueFederateManager::setInputOption (const Input &inp, int32_t option, bool option_value)
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
int ValueFederateManager::getPublicationCount () const
{
    return static_cast<int> (publications.lock_shared ()->size ());
}
/** get a count of the number inputs registered*/
int ValueFederateManager::getInputCount () const { return static_cast<int> (inputs.lock_shared ()->size ()); }

void ValueFederateManager::setInputNotificationCallback (std::function<void(Input &, Time)> callback)
{
    allCallback.store (std::move (callback));
}

void ValueFederateManager::setInputNotificationCallback (const Input &inp,
                                                         std::function<void(Input &, Time)> callback)
{
    auto data = reinterpret_cast<input_info *> (inp.dataReference);
    if (data != nullptr)
    {
        data->callback = std::move (callback);
    }
    else
    {
        throw (InvalidIdentifier ("Input is not valid"));
    }
}

}  // namespace helics
