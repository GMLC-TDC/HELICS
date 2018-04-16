/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "HandleManager.hpp"

namespace helics
{
BasicHandleInfo &HandleManager::addHandle (Core::federate_id_t fed_id,
                                           BasicHandleType what,
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
                                           BasicHandleType what,
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
                                           BasicHandleType what,
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
                                           BasicHandleType what,
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

BasicHandleInfo *HandleManager::getHandleInfo (int32_t index)
{
    if (isValidIndex (index, handles))
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

BasicHandleInfo *HandleManager::getPublication (const std::string &name)
{
    auto fnd = publications.find (name);
    if (fnd != publications.end ())
    {
        return &(handles[fnd->second]);
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
    switch (handle.what)
    {
    case BasicHandleType::HANDLE_END:
        endpoints.emplace (handle.key, index);
        break;
    case BasicHandleType::HANDLE_PUB:
        publications.emplace (handle.key, index);
        break;
    case BasicHandleType::HANDLE_CLONE_FILTER:
    case BasicHandleType::HANDLE_DEST_FILTER:
    case BasicHandleType::HANDLE_SOURCE_FILTER:
        if (!handle.key.empty ())
        {
            filters.emplace (handle.key, index);
        }
        break;
    case BasicHandleType::HANDLE_SUB:
        subscriptions.emplace (handle.key, index);
        break;
    default:
        break;
    }
    // generate a key of the fed and handle
    auto searchKey = generateSearchKey (handle.fed_id, handle.id);
    unique_ids.emplace (searchKey, index);
}

std::string HandleManager::generateName (BasicHandleType what) const
{
    switch (what)
    {
    case BasicHandleType::HANDLE_END:
        return std::string ("ept_") + std::to_string (handles.size ());
    case BasicHandleType::HANDLE_PUB:
        return std::string ("pub_") + std::to_string (handles.size ());
    case BasicHandleType::HANDLE_CLONE_FILTER:
        return std::string ("cFilter_") + std::to_string (handles.size ());
    case BasicHandleType::HANDLE_DEST_FILTER:
        return std::string ("dFilter_") + std::to_string (handles.size ());
    case BasicHandleType::HANDLE_SOURCE_FILTER:
        return std::string ("sFilter_") + std::to_string (handles.size ());
    default:
        return std::string ("handle_") + std::to_string (handles.size ());
    }
}
}  // namespace helics
