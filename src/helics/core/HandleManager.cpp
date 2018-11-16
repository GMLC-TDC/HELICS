/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "HandleManager.hpp"
#include "ActionMessage.hpp"
//TODO move the flags out of actionMessage

namespace helics
{
BasicHandleInfo &HandleManager::addHandle (global_federate_id_t fed_id,
                                           handle_type_t what,
                                           const std::string &key,
                                           const std::string &type,
                                           const std::string &units)
{
    interface_handle local_id(static_cast<interface_handle::base_type>(handles.size ()));
    std::string actKey = (!key.empty ()) ? key : generateName (what);
    handles.emplace_back ( fed_id, local_id, what, actKey, type, units);
    addSearchFields (handles.back (), local_id.baseValue());
    return handles.back ();
}

BasicHandleInfo &HandleManager::addHandle (global_federate_id_t fed_id,
                                           interface_handle local_id,
                                           handle_type_t what,
                                           const std::string &key,
                                           const std::string &type,
                                           const std::string &units)
{
    auto index = static_cast<int32_t> (handles.size ());
    std::string actKey = (!key.empty ()) ? key : generateName (what);
    handles.emplace_back ( fed_id, local_id, what, actKey, type, units);
    addSearchFields (handles.back (), index);
    return handles.back ();
}

//    BasicHandleInfo &HandleManager::addHandle (global_federate_id_t fed_id,
//                                               interface_handle local_id,
//                                               handle_type_t what,
//                                               const std::string &key,
//                                               const std::string &type,
//                                               const std::string &units,
//                                               const std::string &info)
//    {
//        auto index = static_cast<int32_t> (handles.size ());
//        std::string actKey = (!key.empty ()) ? key : generateName (what);
//        handles.emplace_back ( fed_id, local_id, what, actKey, type, units, info);
//        addSearchFields (handles.back (), index);
//        return handles.back ();
//    }

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


BasicHandleInfo *HandleManager::findHandle (global_handle fed_id)
{
    auto key = static_cast<uint64_t> (fed_id);
    auto fnd = unique_ids.find (key);
    if (fnd != unique_ids.end ())
    {
        return &handles[fnd->second];
    }
    return nullptr;
}


void HandleManager::setHandleOption(int32_t index, int option, bool val)
{
    if (isValidIndex(index, handles))
    {
        switch (option)
        {
        case HELICS_ONLY_UPDATE_ON_CHANGE_FLAG:
            if (val)
            {
                setActionFlag(handles[index], extra_flag1);
            }
            else
            {
                clearActionFlag(handles[index], extra_flag1);
            }
            break;
        case HELICS_ONLY_TRANSMIT_ON_CHANGE_FLAG:
            if (val)
            {
                setActionFlag(handles[index], extra_flag2);
            }
            else
            {
                clearActionFlag(handles[index], extra_flag1);
            }
            break;
        case CONNECTION_REQUIRED_OPTION:
            if (val)
            {
                setActionFlag(handles[index], required_flag);
            }
            else
            {
                clearActionFlag(handles[index], required_flag);
            }
            break;
        case CONNECTION_OPTIONAL_OPTION:
            if (val)
            {
                clearActionFlag(handles[index], required_flag);
            }
            else
            {
                setActionFlag(handles[index], required_flag);
            }
            break;
        case SINGLE_CONNECTION_ONLY_OPTION:
            if (val)
            {
                setActionFlag(handles[index], extra_flag4);
            }
            else
            {
                clearActionFlag(handles[index], extra_flag4);
            }
            break;
        }
    }
}

bool HandleManager::getHandleOption(int32_t index, int option) const
{
    if (isValidIndex(index, handles))
    {
        switch (option)
        {
        case HELICS_ONLY_UPDATE_ON_CHANGE_FLAG:
            return checkActionFlag(handles[index], extra_flag1);
        case HELICS_ONLY_TRANSMIT_ON_CHANGE_FLAG:
            return checkActionFlag(handles[index], extra_flag2);
        case CONNECTION_REQUIRED_OPTION:
           return checkActionFlag(handles[index], required_flag);
        case CONNECTION_OPTIONAL_OPTION:
            return  !checkActionFlag(handles[index], required_flag);
        case SINGLE_CONNECTION_ONLY_OPTION:
            return checkActionFlag(handles[index], extra_flag4);
        default:
            return false;
        }
    }
    return false;
}

BasicHandleInfo *HandleManager::getEndpoint (const std::string &name)
{
    auto fnd = endpoints.find (name);
    if (fnd != endpoints.end ())
    {
        return &handles[fnd->second.baseValue()];
    }
    return nullptr;
}

const BasicHandleInfo *HandleManager::getEndpoint(const std::string &name) const
{
    auto fnd = endpoints.find(name);
    if (fnd != endpoints.end())
    {
        return &handles[fnd->second.baseValue ()];
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
        return &(handles[fnd->second.baseValue ()]);
    }
    return nullptr;
}

const BasicHandleInfo *HandleManager::getPublication(const std::string &name) const
{
    auto fnd = publications.find(name);
    if (fnd != publications.end())
    {
        return &(handles[fnd->second.baseValue ()]);
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

BasicHandleInfo *HandleManager::getInput(const std::string &name)
{
    auto fnd = inputs.find(name);
    if (fnd != inputs.end())
    {
        return &(handles[fnd->second.baseValue ()]);
    }
    return nullptr;
}

const BasicHandleInfo *HandleManager::getInput(const std::string &name) const
{
    auto fnd = inputs.find(name);
    if (fnd != inputs.end())
    {
        return &(handles[fnd->second.baseValue ()]);
    }
    return nullptr;
}

BasicHandleInfo *HandleManager::getFilter(const std::string &name)
{
    auto ar = filters.equal_range(name);
    if (ar.first == ar.second)
    {
        return nullptr;
    }
    return &(handles[ar.first->second.baseValue ()]);
}

const BasicHandleInfo *HandleManager::getFilter(const std::string &name) const
{
    auto ar= filters.equal_range(name);
    if (ar.first == ar.second)
    {
        return nullptr;
    }
    return &(handles[ar.first->second.baseValue ()]);
}
BasicHandleInfo *HandleManager::getFilter(int32_t index)
{
    if (isValidIndex(index, handles))
    {
        auto &hand = handles[index];
        if (hand.handle_type == handle_type_t::filter)
        {
            return &hand;
        }
    }

    return nullptr;
}

federate_id_t HandleManager::getLocalFedID (interface_handle id_) const
{
    // only activate the lock if we not in an operating state
    auto index = id_.baseValue ();
    return (isValidIndex (index, handles)) ?
                   handles[index].local_fed_id :
                   federate_id_t ();
}

void HandleManager::addSearchFields (const BasicHandleInfo &handle, int32_t index)
{
    switch (handle.handle_type)
    {
    case handle_type_t::endpoint:
        endpoints.emplace (handle.key, interface_handle(index));
        break;
    case handle_type_t::publication:
        publications.emplace (handle.key, interface_handle(index));
        break;
    case handle_type_t::filter:
        if (!handle.key.empty ())
        {
            filters.emplace (handle.key, interface_handle(index));
        }
        break;
    case handle_type_t::input:
        inputs.emplace (handle.key, interface_handle(index));
        break;
    default:
        break;
    }
    // generate a key of the fed and handle
    unique_ids.emplace (static_cast<uint64_t>(handle.handle), index);
}

std::string HandleManager::generateName (handle_type_t what) const
{
    switch (what)
    {
    case handle_type_t::endpoint:
        return std::string ("_ept_") + std::to_string (handles.size ());
    case handle_type_t::input:
        return std::string ("_input_") + std::to_string (handles.size ());
    case handle_type_t::publication:
        return std::string ("_pub_") + std::to_string (handles.size ());
    case handle_type_t::filter:
        return std::string ("_filter_") + std::to_string (handles.size ());
    default:
        return std::string ("_handle_") + std::to_string (handles.size ());
    }
}
}  // namespace helics
