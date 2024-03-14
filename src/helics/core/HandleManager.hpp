/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "BasicHandleInfo.hpp"
#include "Core.hpp"
#include "helicsTime.hpp"

#include <deque>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace helics {

/** class for managing a coordinating the different types of handles used in helics
@details this class is not designed to be thread safe that would require a wrapper around it
*/
class HandleManager {
  private:
    using MapType = std::unordered_map<std::string_view, InterfaceHandle>;
    /** use deque here as there are several use cases which use two properties of a deque vs vector
    namely that references are not invalidated by emplace back, which is unlike a vector
    and that the memory growth is a stable and not subject to large copy operations
    this allows much more stable growth to bigger sizes if need be since we don't need contiguous
    memory, just iterators, so these properties outweigh the slight decrease in overall performance,
    otherwise we would need two classes that do basically the same thing just with different
    container types so using deque reduce the amount of the code to maintain as well*/
    std::deque<BasicHandleInfo> handles;  //!< local handle information
    /// map of all local publications
    MapType publications;
    /// map of all local endpoints
    MapType endpoints;
    MapType inputs;  //!< map of all local endpoints
    MapType filters;  //!< map of all local endpoints
    std::unordered_map<std::uint64_t, int32_t> unique_ids;  //!< map of identifiers
    /// set of all valid aliases <interface_name,aliases>
    std::unordered_map<std::string_view, std::vector<std::string_view>> aliases;
    std::unordered_set<std::string> alias_names;  //!< set of actual alias strings
  public:
    /** default constructor*/
    HandleManager() = default;
    /** add a handle to manage*/
    BasicHandleInfo& addHandle(GlobalFederateId fed_id,
                               InterfaceType what,
                               std::string_view key,
                               std::string_view type,
                               std::string_view units);
    /** add a handle to manage*/
    BasicHandleInfo& addHandle(GlobalFederateId fed_id,
                               InterfaceHandle local_id,
                               InterfaceType what,
                               std::string_view key,
                               std::string_view type,
                               std::string_view units);

    void addHandle(const BasicHandleInfo& otherHandle);
    /** add a handle at the specified index*/
    void addHandleAtIndex(const BasicHandleInfo& otherHandle, int32_t index);
    /** remove the information at the specified handle*/
    void removeHandle(GlobalHandle handle);
    /** remove all handles from a specific federate*/
    void removeFederateHandles(GlobalFederateId fedToRemove);
    /** get a handle by index*/
    BasicHandleInfo* getHandleInfo(int32_t index);
    /** get a const handle by index*/
    const BasicHandleInfo* getHandleInfo(int32_t index) const;
    /** get a handle by local interface handle*/
    BasicHandleInfo* getHandleInfo(InterfaceHandle handle);
    /** get a const handle by local interface handle*/
    const BasicHandleInfo* getHandleInfo(InterfaceHandle handle) const;
    /** find a handle from both the federate and local id*/
    BasicHandleInfo* findHandle(GlobalHandle id);
    /** find a const handle from both the federate and local id*/
    const BasicHandleInfo* findHandle(GlobalHandle id) const;

    void setHandleOption(InterfaceHandle handle, int32_t option, int32_t val);

    int32_t getHandleOption(InterfaceHandle handle, int32_t option) const;
    /** get an endpoint from its name*/
    BasicHandleInfo* getInterfaceHandle(std::string_view name, InterfaceType type);
    /** get an endpoint from its name*/
    const BasicHandleInfo* getInterfaceHandle(std::string_view name, InterfaceType type) const;
    /** get an endpoint by index
    @return nullptr if the index doesn't point to a valid endpoint*/
    BasicHandleInfo* getInterfaceHandle(InterfaceHandle handle, InterfaceType type);
    /** get a const endpoint by index
    @return nullptr if the index doesn't point to a valid endpoint*/
    const BasicHandleInfo* getInterfaceHandle(InterfaceHandle handle, InterfaceType type) const;

    LocalFederateId getLocalFedID(InterfaceHandle handle) const;

    BasicHandleInfo& operator[](size_t index) { return handles[index]; }
    const BasicHandleInfo& operator[](size_t index) const { return handles[index]; }
    /** add an alias for an interface*
    @param interfaceName the name of the interface to add an alias for
    @param alias the new name by which an interface can be referenced
    @throws std::runtime_error if an alias is duplicated */
    void addAlias(std::string_view interfaceName, std::string_view alias);
    auto begin() { return handles.begin(); }
    auto end() { return handles.end(); }
    auto begin() const { return handles.begin(); }
    auto end() const { return handles.end(); }
    auto size() const { return handles.size(); }
    /* search for handles based on a regex string and type*/
    std::vector<GlobalHandle> regexSearch(const std::string& regexExpression,
                                          InterfaceType type) const;
    /** get all the aliases*/
    const std::unordered_map<std::string_view, std::vector<std::string_view>>& getAliases() const
    {
        return aliases;
    }

  private:
    void addSearchFields(const BasicHandleInfo& handle, int32_t index);
    std::string generateName(InterfaceType what) const;
    // alias must be a stable string_view here
    void addPublicationAlias(std::string_view interfaceName, std::string_view alias);
    // alias must be a stable string_view here
    void addInputAlias(std::string_view interfaceName, std::string_view alias);
    // alias must be a stable string_view here
    void addEndpointAlias(std::string_view interfaceName, std::string_view alias);
    // alias must be a stable string_view here
    void addFilterAlias(std::string_view interfaceName, std::string_view alias);
    /// @brief actually add the alias names to the data structures
    /// @param interfaceName the name of the interface to create an alias for
    /// @param alias the new name used to refer to an interface
    /// @return true if there are multiple interacting aliases
    bool addAliasName(std::string_view interfaceName, std::string_view alias);
    /** get the appropriate map based on type*/
    MapType& getMap(InterfaceType type);
    const MapType& getMap(InterfaceType type) const;
};

}  // namespace helics
