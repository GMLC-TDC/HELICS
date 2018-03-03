/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "HandleManager.hpp"

namespace helics
{
BasicHandleInfo *HandleManager::addHandle(
    Core::federate_id_t fed_id,
    BasicHandleType what,
    const std::string &key,
    const std::string &type,
    const std::string &units)
{
    Core::handle_id_t local_id = static_cast<Core::handle_id_t>(handles.size());
    std::string actKey = (!key.empty()) ? key : generateName(what);
    handles.push_back(std::make_unique<BasicHandleInfo>(local_id, fed_id, what, actKey, type, units));
    auto hpointer = handles.back().get();
    addType(hpointer, local_id);
    return hpointer;
}

BasicHandleInfo *HandleManager::addHandle(
    Core::federate_id_t fed_id,
    BasicHandleType what,
    const std::string &key,
    const std::string &target,
    const std::string &type_in,
    const std::string &type_out)
{
    Core::handle_id_t local_id = static_cast<Core::handle_id_t>(handles.size());
    handles.emplace_back(std::make_unique<BasicHandleInfo>(local_id, fed_id, what, key, target, type_in, type_out));
    auto hpointer = handles.back().get();
    addType(hpointer, local_id);
    return hpointer;
}


BasicHandleInfo *HandleManager::getHandleInfo(Core::handle_id_t id_) const
{
    if (isValidIndex(id_, handles))
    {
        return handles[id_].get();
    }

    return nullptr;
}

BasicHandleInfo *HandleManager::getEndpoint(const std::string &name) const
{

    auto fnd = endpoints.find(name);
    if (fnd != endpoints.end())
    {
        return handles[fnd->second].get();
    }
    return nullptr;
}

BasicHandleInfo *HandleManager::getFilter(const std::string &name) const
{

    auto fnd = filters.find(name);
    if (fnd != filters.end())
    {
        return handles[fnd->second].get();
    }
    return nullptr;
}

BasicHandleInfo *HandleManager::getSubscription(const std::string &name) const
{

    auto fnd = subscriptions.find(name);
    if (fnd != subscriptions.end())
    {
        return handles[fnd->second].get();
    }
    return nullptr;
}

BasicHandleInfo *HandleManager::getPublication(const std::string &name) const
{

    auto fnd = publications.find(name);
    if (fnd != publications.end())
    {
        return handles[fnd->second].get();
    }
    return nullptr;
}

int32_t HandleManager::getLocalFedID(Core::handle_id_t id_) const
{
    // only activate the lock if we not in an operating state
    return (isValidIndex(id_, handles)) ? handles[id_]->local_fed_id : invalid_fed_id;
}

void HandleManager::addType(BasicHandleInfo *handle, int32_t index)
{
    switch (handle->what)
    {
        case BasicHandleType::HANDLE_END:
            endpoints.emplace(handle->key, index);
            break;
        case BasicHandleType::HANDLE_PUB:
            publications.emplace(handle->key, index);
            break;
        case BasicHandleType::HANDLE_CLONE_FILTER:
        case BasicHandleType::HANDLE_DEST_FILTER:
        case BasicHandleType::HANDLE_SOURCE_FILTER:
            if (!handle->key.empty())
            {
                filters.emplace(handle->key, index);
            }
            break;
        case BasicHandleType::HANDLE_SUB:
            subscriptions.emplace(handle->key, index);
            break;
        default:
            break;
    }
}

std::string HandleManager::generateName(BasicHandleType what)
{
    switch (what)
    {
    case BasicHandleType::HANDLE_END:
        return std::string("ept_") + std::to_string(handles.size());
    case BasicHandleType::HANDLE_PUB:
        return std::string("pub_") + std::to_string(handles.size());
    case BasicHandleType::HANDLE_CLONE_FILTER:
        return std::string("cFilter_") + std::to_string(handles.size());
    case BasicHandleType::HANDLE_DEST_FILTER:
        return std::string("dFilter_") + std::to_string(handles.size());
    case BasicHandleType::HANDLE_SOURCE_FILTER:
        return std::string("sFilter_") + std::to_string(handles.size());
    default:
        return std::string("handle_") + std::to_string(handles.size());
    }
}
} // namespace helics