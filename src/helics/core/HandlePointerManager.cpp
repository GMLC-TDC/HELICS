/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "HandlePointerManager.hpp"

namespace helics
{
BasicHandleInfo *HandlePointerManager::addHandle (global_federate_id_t fed_id,
                                                  handle_type_t what,
                                                  const std::string &key,
                                                  const std::string &type,
                                                  const std::string &units)
{
    Core::handle_id_t local_id = static_cast<Core::handle_id_t> (handles.size ());
    std::string actKey = (!key.empty ()) ? key : generateName (what);
    handles.push_back (std::make_unique<BasicHandleInfo> (local_id, fed_id, what, actKey, type, units));
    auto hpointer = handles.back ().get ();
    addType (hpointer, local_id);
    return hpointer;
}

BasicHandleInfo *HandlePointerManager::addHandle (global_federate_id_t fed_id,
                                                  handle_type_t what,
                                                  const std::string &key,
                                                  const std::string &target,
                                                  const std::string &type_in,
                                                  const std::string &type_out)
{
    Core::handle_id_t local_id = static_cast<Core::handle_id_t> (handles.size ());
    handles.emplace_back (
      std::make_unique<BasicHandleInfo> (local_id, fed_id, what, key, target, type_in, type_out));
    auto hpointer = handles.back ().get ();
    addType (hpointer, local_id);
    return hpointer;
}

BasicHandleInfo *HandlePointerManager::getHandleInfo (Core::handle_id_t id_) const
{
    if (isValidIndex (id_, handles))
    {
        return handles[id_].get ();
    }

    return nullptr;
}

BasicHandleInfo *HandlePointerManager::getEndpoint (const std::string &name) const
{
    auto fnd = endpoints.find (name);
    if (fnd != endpoints.end ())
    {
        return handles[fnd->second].get ();
    }
    return nullptr;
}

BasicHandleInfo *HandlePointerManager::getFilter (const std::string &name) const
{
    auto fnd = filters.find (name);
    if (fnd != filters.end ())
    {
        return handles[fnd->second].get ();
    }
    return nullptr;
}

BasicHandleInfo *HandlePointerManager::getSubscription (const std::string &name) const
{
    auto fnd = subscriptions.find (name);
    if (fnd != subscriptions.end ())
    {
        return handles[fnd->second].get ();
    }
    return nullptr;
}

BasicHandleInfo *HandlePointerManager::getPublication (const std::string &name) const
{
    auto fnd = publications.find (name);
    if (fnd != publications.end ())
    {
        return handles[fnd->second].get ();
    }
    return nullptr;
}

federate_id_t HandlePointerManager::getLocalFedID (Core::handle_id_t id_) const
{
    // only activate the lock if we not in an operating state
    return (isValidIndex (id_, handles)) ? handles[id_]->local_fed_id : federate_id_t();
}

void HandlePointerManager::addType (BasicHandleInfo *handle, int32_t index)
{
    switch (handle->handle_type)
    {
    case handle_type_t::endpoint:
        endpoints.emplace (handle->key, index);
        break;
    case handle_type_t::publication:
        publications.emplace (handle->key, index);
        break;
    case handle_type_t::cloning_filter:
    case handle_type_t::destination_filter:
    case handle_type_t::source_filter:
        if (!handle->key.empty ())
        {
            filters.emplace (handle->key, index);
        }
        break;
    case handle_type_t::subscription:
        subscriptions.emplace (handle->key, index);
        break;
    default:
        break;
    }
}

std::string HandlePointerManager::generateName (handle_type_t what)
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
