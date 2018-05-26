/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once
#include "Core.hpp"
#include "helics-time.hpp"
#include <atomic>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include "BasicHandleInfo.hpp"
#include "helics_includes/optional.hpp"
namespace helics
{
/** class for managing a coordinating the different types of handles used in helics
@details this class is not designed to be thread safe that would require a wrapper around it
*/
class HandleManager
{
  private:
    std::vector<BasicHandleInfo> handles;  //!< local handle information
    std::unordered_map<std::string, handle_id_t> publications;  //!< map of all local publications
    std::unordered_map<std::string, handle_id_t> endpoints;  //!< map of all local endpoints
    std::unordered_multimap<std::string, handle_id_t> subscriptions;  //!< multimap of subscriptions
    std::unordered_multimap<std::string, handle_id_t> filters;  //!< multimap for all the filters
    std::unordered_map<std::uint64_t, int32_t> unique_ids;  //!< map of identifiers
  public:
    /** default constructor*/
    HandleManager () = default;
    /** add a handle to manage*/
    BasicHandleInfo &addHandle (global_federate_id_t fed_id,
                                handle_type_t what,
                                const std::string &key,
                                const std::string &type,
                                const std::string &units);
    /** add a handle to manage*/
    BasicHandleInfo &addHandle (global_federate_id_t fed_id,
                                handle_type_t what,
                                const std::string &key,
                                const std::string &target,
                                const std::string &type_in,
                                const std::string &type_out);
    /** add a handle to manage*/
    BasicHandleInfo &addHandle (global_federate_id_t fed_id,
                                handle_id_t local_id,
                                handle_type_t what,
                                const std::string &key,
                                const std::string &type,
                                const std::string &units);
    /** add a handle to manage*/
    BasicHandleInfo &addHandle (global_federate_id_t fed_id,
                                handle_id_t local_id,
                                handle_type_t what,
                                const std::string &key,
                                const std::string &target,
                                const std::string &type_in,
                                const std::string &type_out);
    /** get a handle by index*/
    BasicHandleInfo *getHandleInfo (int32_t index);
    /** find a handle from both the federate and local id*/
    BasicHandleInfo *findHandle (global_federate_id_t fed_id, handle_id_t id);

    BasicHandleInfo *getEndpoint (const std::string &name);
    auto getFilters (const std::string &name) { return filters.equal_range (name); }
    auto getSubscribers (const std::string &name) { return subscriptions.equal_range (name); }
    BasicHandleInfo *getPublication (const std::string &name);

    federate_id_t getLocalFedID (handle_id_t id_) const;

    BasicHandleInfo &operator[] (size_t index) { return handles[index]; }
    const BasicHandleInfo &operator[] (size_t index) const { return handles[index]; }
    auto begin () { return handles.begin (); }
    auto end () { return handles.end (); }
    auto begin () const { return handles.begin (); }
    auto end () const { return handles.end (); }
    auto size () const { return handles.size (); }

  private:
    void addSearchFields (const BasicHandleInfo &handle, int32_t index);
    std::string generateName (handle_type_t what) const;
};

}  // namespace helics
