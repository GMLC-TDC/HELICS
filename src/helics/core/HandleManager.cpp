/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "HandleManager.hpp"

namespace helics
{
BasicHandleInfo &HandleManager::addHandle (Core::federate_id_t fed_id,
                                           handle_type_t what,
                                           const std::string &key,
                                           const std::string &type,
                                           const std::string &units)
{
    Core::handle_id_t local_id = static_cast<Core::handle_id_t> (handles.size ());
    std::string actKey = (!key.empty ()) ? key : generateName (what);
    handles.emplace_back (local_id, fed_id, what, actKey, type, units);
    addSearchFields (handles.back (), local_id);
    return handles.back ();
}

BasicHandleInfo &HandleManager::addHandle (Core::federate_id_t fed_id,
                                           handle_type_t what,
                                           const std::string &key,
                                           const std::string &target,
                                           const std::string &type_in,
                                           const std::string &type_out)
{
    Core::handle_id_t local_id = static_cast<Core::handle_id_t> (handles.size ());
    std::string actKey = (!key.empty ()) ? key : generateName (what);
    handles.emplace_back (local_id, fed_id, what, actKey, target, type_in, type_out);
    addSearchFields (handles.back (), local_id);
    return handles.back ();
}

BasicHandleInfo &HandleManager::addHandle (Core::federate_id_t fed_id,
                                           Core::handle_id_t local_id,
                                           handle_type_t what,
                                           const std::string &key,
                                           const std::string &type,
                                           const std::string &units)
{
    auto index = static_cast<int32_t> (handles.size ());
    std::string actKey = (!key.empty ()) ? key : generateName (what);
    handles.emplace_back (local_id, fed_id, what, actKey, type, units);
    addSearchFields (handles.back (), index);
    return handles.back ();
}

BasicHandleInfo &HandleManager::addHandle (Core::federate_id_t fed_id,
                                           Core::handle_id_t local_id,
                                           handle_type_t what,
                                           const std::string &key,
                                           const std::string &target,
                                           const std::string &type_in,
                                           const std::string &type_out)
{
    auto index = static_cast<int32_t> (handles.size ());
    std::string actKey = (!key.empty ()) ? key : generateName (what);
    handles.emplace_back (local_id, fed_id, what, actKey, target, type_in, type_out);
    addSearchFields (handles.back (), index);
    return handles.back ();
}

void HandleManager::addHandle(const BasicHandleInfo &otherHandle)
{
    auto index = static_cast<int32_t> (handles.size());
    handles.push_back(otherHandle);
    addSearchFields(handles.back(), index);
}

void HandleManager::addHandleAtIndex(const BasicHandleInfo &otherHandle, int32_t index)
{
    if (index == static_cast<int32_t>(handles.size()))
    {
        addHandle(otherHandle);
    }
    else if (isValidIndex(index, handles))
    {
        //use placement new to reconstruct new object
        new(&handles[index]) BasicHandleInfo(otherHandle);
        addSearchFields(handles[index], index);
    }
    else if (index>0)
    {
        handles.resize(index + 1);
        //use placement new to reconstruct new object
        new(&handles[index]) BasicHandleInfo(otherHandle);
        addSearchFields(handles[index], index);
    }
}
BasicHandleInfo *HandleManager::getHandleInfo (int32_t index)
{
    if (isValidIndex (index, handles))
    {
        return &handles[index];
    }

    return nullptr;
}

const BasicHandleInfo *HandleManager::getHandleInfo(int32_t index) const
{
    if (isValidIndex(index, handles))
    {
        return &handles[index];
    }

    return nullptr;
}

static uint64_t generateSearchKey (Core::federate_id_t fed_id, Core::handle_id_t id)
{
    auto searchKey = static_cast<uint64_t> (fed_id) << 32;
    searchKey += static_cast<uint64_t> (id) & (0x0000'0000'FFFF'FFFF);
    return searchKey;
}

BasicHandleInfo *HandleManager::findHandle (Core::federate_id_t fed_id, Core::handle_id_t id)
{
    auto key = generateSearchKey (fed_id, id);
    auto fnd = unique_ids.find (key);
    if (fnd != unique_ids.end ())
    {
        return &handles[fnd->second];
    }
    return nullptr;
}

BasicHandleInfo *HandleManager::getEndpoint (const std::string &name)
{
    auto fnd = endpoints.find (name);
    if (fnd != endpoints.end ())
    {
        return &handles[fnd->second];
    }
    return nullptr;
}

const BasicHandleInfo *HandleManager::getEndpoint(const std::string &name) const
{
    auto fnd = endpoints.find(name);
    if (fnd != endpoints.end())
    {
        return &handles[fnd->second];
    }
    return nullptr;
}

BasicHandleInfo *HandleManager::getEndpoint(int32_t index)
{
    if (isValidIndex(index, handles))
    {
        auto &hand = handles[index];
        if (hand.handle_type == handle_type_t::endpoint)
        {
            return &hand;
        }
    }

    return nullptr;
}

BasicHandleInfo *HandleManager::getPublication (const std::string &name)
{
    auto fnd = publications.find (name);
    if (fnd != publications.end ())
    {
        return &(handles[fnd->second]);
    }
    return nullptr;
}

const BasicHandleInfo *HandleManager::getPublication(const std::string &name) const
{
    auto fnd = publications.find(name);
    if (fnd != publications.end())
    {
        return &(handles[fnd->second]);
    }
    return nullptr;
}

BasicHandleInfo *HandleManager::getPublication(int32_t index)
{
    if (isValidIndex(index, handles))
    {
        auto &hand=handles[index];
        if (hand.handle_type == handle_type_t::publication)
        {
            return &hand;
        }
    }

    return nullptr;
}

BasicHandleInfo *HandleManager::getControlInput(const std::string &name)
{
    auto fnd = controlInputs.find(name);
    if (fnd != controlInputs.end())
    {
        return &(handles[fnd->second]);
    }
    return nullptr;
}

const BasicHandleInfo *HandleManager::getControlInput(const std::string &name) const
{
    auto fnd = controlInputs.find(name);
    if (fnd != controlInputs.end())
    {
        return &(handles[fnd->second]);
    }
    return nullptr;
}


BasicHandleInfo *HandleManager::getFilter(int32_t index)
{
    if (isValidIndex(index, handles))
    {
        auto &hand = handles[index];
        if ((hand.handle_type == handle_type_t::source_filter)||(hand.handle_type==handle_type_t::destination_filter))
        {
            return &hand;
        }
    }

    return nullptr;
}

int32_t HandleManager::getLocalFedID (Core::handle_id_t id_) const
{
    // only activate the lock if we not in an operating state
    return (isValidIndex (id_, handles)) ? handles[id_].local_fed_id : invalid_fed_id;
}

void HandleManager::addSearchFields (const BasicHandleInfo &handle, int32_t index)
{
    switch (handle.handle_type)
    {
    case handle_type_t::endpoint:
        endpoints.emplace (handle.key, index);
        break;
    case handle_type_t::publication:
        publications.emplace (handle.key, index);
        break;
    case handle_type_t::cloning_filter:
    case handle_type_t::destination_filter:
    case handle_type_t::source_filter:
        if (!handle.key.empty ())
        {
            filters.emplace (handle.key, index);
        }
        break;
    case handle_type_t::subscription:
        subscriptions.emplace (handle.key, index);
        break;
    default:
        break;
    }
    // generate a key of the fed and handle
    auto searchKey = generateSearchKey (handle.fed_id, handle.handle);
    unique_ids.emplace (searchKey, index);
}

std::string HandleManager::generateName (handle_type_t what) const
{
    switch (what)
    {
    case handle_type_t::endpoint:
        return std::string ("ept_") + std::to_string (handles.size ());
    case handle_type_t::publication:
        return std::string ("pub_") + std::to_string (handles.size ());
    case handle_type_t::cloning_filter:
        return std::string ("cFilter_") + std::to_string (handles.size ());
    case handle_type_t::destination_filter:
        return std::string ("dFilter_") + std::to_string (handles.size ());
    case handle_type_t::source_filter:
        return std::string ("sFilter_") + std::to_string (handles.size ());
    default:
        return std::string ("handle_") + std::to_string (handles.size ());
    }
}
}  // namespace helics
