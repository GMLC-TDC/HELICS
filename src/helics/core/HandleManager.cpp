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

} // namespace helics