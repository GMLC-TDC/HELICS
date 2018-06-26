/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once
#include "Core.hpp"
#include "helics-time.hpp"
#include <unordered_map>
#include <deque>
#include <utility>
#include "BasicHandleInfo.hpp"
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
      just iterators, so these properties outweigh the slight decrease in overall performance, otherwise we would need two classes that do basicall
      the same thing just with different container types so using deque reduce the amount of the code to maintain as well*/
    std::deque<BasicHandleInfo> handles;  //!< local handle information
    std::unordered_map<std::string, Core::handle_id_t> publications;  //!< map of all local publications
    std::unordered_map<std::string, Core::handle_id_t> endpoints;  //!< map of all local endpoints
    std::unordered_map<std::string, Core::handle_id_t> controlInputs;  //!< map of all local controlInputs
    std::unordered_multimap<std::string, Core::handle_id_t> subscriptions;  //!< multimap of subscriptions
    std::unordered_multimap<std::string, Core::handle_id_t> filters;  //!< multimap for all the filters
    std::unordered_multimap<std::string, Core::handle_id_t> controlOutputs;  //!< multimap for all the control outputs
    std::unordered_map<std::uint64_t, int32_t> unique_ids;  //!< map of identifiers
  public:
    /** default constructor*/
    HandleManager () = default;
    /** add a handle to manage*/
    BasicHandleInfo &addHandle (Core::federate_id_t fed_id,
                                handle_type_t what,
                                const std::string &key,
                                const std::string &type,
                                const std::string &units);
    /** add a handle to manage*/
    BasicHandleInfo &addHandle (Core::federate_id_t fed_id,
                                handle_type_t what,
                                const std::string &key,
                                const std::string &target,
                                const std::string &type_in,
                                const std::string &type_out);
    /** add a handle to manage*/
    BasicHandleInfo &addHandle (Core::federate_id_t fed_id,
                                Core::handle_id_t local_id,
                                handle_type_t what,
                                const std::string &key,
                                const std::string &type,
                                const std::string &units);
    /** add a handle to manage*/
    BasicHandleInfo &addHandle (Core::federate_id_t fed_id,
                                Core::handle_id_t local_id,
                                handle_type_t what,
                                const std::string &key,
                                const std::string &target,
                                const std::string &type_in,
                                const std::string &type_out);

    void addHandle(const BasicHandleInfo &otherHandle);
    /** add a handle at the specified index*/
    void addHandleAtIndex(const BasicHandleInfo &otherHandle,int32_t index);
    /** get a handle by index*/
    BasicHandleInfo *getHandleInfo (int32_t index);
    /** get a const handle by index*/
    const BasicHandleInfo *getHandleInfo(int32_t index) const;
    /** find a handle from both the federate and local id*/
    BasicHandleInfo *findHandle (Core::federate_id_t fed_id, Core::handle_id_t id);

    BasicHandleInfo *getEndpoint (const std::string &name);
    const BasicHandleInfo *getEndpoint(const std::string &name) const;
    /** get an endpoint by index
    @return nullptr if the index doesn't point to a valid endpoint*/
    BasicHandleInfo *getEndpoint(int32_t index);
    auto getFilters (const std::string &name) const { return filters.equal_range (name); }
    /** get a filter by index
    @return nullptr if the index doesn't point to a valid filter*/
    BasicHandleInfo *getFilter(int32_t index);
    auto getSubscribers (const std::string &name) const { return subscriptions.equal_range (name); }
    auto getControlOutputs(const std::string &name) const { return controlOutputs.equal_range(name); }
    BasicHandleInfo *getPublication (const std::string &name);
    const BasicHandleInfo *getPublication(const std::string &name) const;
    /** get a publication by index
    @return nullptr if the index doesn't point to a valid publication*/
    BasicHandleInfo *getPublication(int32_t index);
    BasicHandleInfo *getControlInput(const std::string &name);
    int32_t getLocalFedID (Core::handle_id_t id_) const;

    BasicHandleInfo &operator[] (size_t index) { return handles[index]; }
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
