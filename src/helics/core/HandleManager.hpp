/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once
#include "BasicHandleInfo.hpp"
#include "Core.hpp"
#include "helics-time.hpp"
#include <deque>
#include <unordered_map>
#include <utility>
namespace helics
{
/** class for managing a coordinating the different types of handles used in helics
@details this class is not designed to be thread safe that would require a wrapper around it
*/
class HandleManager
{
  private:
    /** use deque here as there are several use cases which use two properties of a deque vs vector
    namely that references are not invalidated by emplace back, which is unlike a vector
    and that the memory growth is a stable and not subject to large copy operations
    this allows much more stable growth to bigger sizes if need be since we don't need contiguous memory,
    just iterators, so these properties outweigh the slight decrease in overall performance, otherwise we would
    need two classes that do basically
    the same thing just with different container types so using deque reduce the amount of the code to maintain as
    well*/
    std::deque<BasicHandleInfo> handles;  //!< local handle information
    std::unordered_map<std::string, interface_handle> publications;  //!< map of all local publications
    std::unordered_map<std::string, interface_handle> endpoints;  //!< map of all local endpoints
    std::unordered_map<std::string, interface_handle> inputs;  //!< map of all local endpoints
    std::unordered_map<std::string, interface_handle> filters;  //!< map of all local endpoints
    std::unordered_map<std::uint64_t, int32_t> unique_ids;  //!< map of identifiers
  public:
    /** default constructor*/
    HandleManager () = default;
    /** add a handle to manage*/
    BasicHandleInfo &addHandle (global_federate_id fed_id,
                                handle_type what,
                                const std::string &key,
                                const std::string &type,
                                const std::string &units);
    /** add a handle to manage*/
    BasicHandleInfo &addHandle (global_federate_id fed_id,
                                interface_handle local_id,
                                handle_type what,
                                const std::string &key,
                                const std::string &type,
                                const std::string &units);

    void addHandle (const BasicHandleInfo &otherHandle);
    /** add a handle at the specified index*/
    void addHandleAtIndex (const BasicHandleInfo &otherHandle, int32_t index);
    /** remove the information at the specified handle*/
    void removeHandle (global_handle handle);
    /** get a handle by index*/
    BasicHandleInfo *getHandleInfo (int32_t index);
    /** get a const handle by index*/
    const BasicHandleInfo *getHandleInfo (int32_t index) const;
    /** get a handle by index*/
    BasicHandleInfo *getHandleInfo (interface_handle handle);
    /** get a const handle by index*/
    const BasicHandleInfo *getHandleInfo (interface_handle handle) const;
    /** find a handle from both the federate and local id*/
    BasicHandleInfo *findHandle (global_handle id);

    void setHandleOption (interface_handle handle, int option, bool val);

    bool getHandleOption (interface_handle handle, int option) const;
    /** get an endpoint from its name*/
    BasicHandleInfo *getEndpoint (const std::string &name);
    /** get an endpoint from its name*/
    const BasicHandleInfo *getEndpoint (const std::string &name) const;
    /** get an endpoint by index
    @return nullptr if the index doesn't point to a valid endpoint*/
    BasicHandleInfo *getEndpoint (interface_handle index);
    const BasicHandleInfo *getFilter (const std::string &name) const;
    BasicHandleInfo *getFilter (const std::string &name);
    // auto getFilters (const std::string &name) const { return filters.equal_range (name); }
    /** get a filter by index
    @return nullptr if the index doesn't point to a valid filter*/
    BasicHandleInfo *getFilter (interface_handle index);
    BasicHandleInfo *getPublication (const std::string &name);
    const BasicHandleInfo *getPublication (const std::string &name) const;
    /** get a publication by index
    @return nullptr if the index doesn't point to a valid publication*/
    BasicHandleInfo *getPublication (interface_handle index);
    BasicHandleInfo *getInput (const std::string &name);
    const BasicHandleInfo *getInput (const std::string &name) const;
    local_federate_id getLocalFedID (interface_handle id_) const;

    BasicHandleInfo &operator[] (size_t index) { return handles[index]; }
    const BasicHandleInfo &operator[] (size_t index) const { return handles[index]; }
    auto begin () { return handles.begin (); }
    auto end () { return handles.end (); }
    auto begin () const { return handles.begin (); }
    auto end () const { return handles.end (); }
    auto size () const { return handles.size (); }

  private:
    void addSearchFields (const BasicHandleInfo &handle, int32_t index);
    std::string generateName (handle_type what) const;
};

}  // namespace helics
