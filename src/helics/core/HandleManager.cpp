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
BasicHandleInfo *HandleManager::addHandle(Core::handle_id_t id,
    Core::federate_id_t fed_id,
    BasicHandleType what,
    const std::string &key,
    const std::string &type,
    const std::string &units)
{
    Core::handle_id_t local_id = static_cast<Core::handle_id_t>(handles.size());
    if (id == invalid_handle)
    {
        id = local_id;
    }
    handles.push_back(std::make_unique<BasicHandleInfo>(id, fed_id, what, key, type, units));
    return handles.back().get();
}

BasicHandleInfo *HandleManager::addHandle(Core::handle_id_t id,
    Core::federate_id_t fed_id,
    BasicHandleType what,
    const std::string &key,
    const std::string &target,
    const std::string &type_in,
    const std::string &type_out)
{
    Core::handle_id_t local_id = static_cast<Core::handle_id_t>(handles.size());
    if (id == invalid_handle)
    {
        id = local_id;
    }
    handles.emplace_back(std::make_unique<BasicHandleInfo>(id, fed_id, what, key, target, type_in, type_out));
    return handles.back().get();
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

} // namespace helics