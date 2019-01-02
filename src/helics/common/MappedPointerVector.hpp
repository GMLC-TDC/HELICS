/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once
#include "MapTraits.hpp"
#include "helics_includes/optional.hpp"
#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/** class merging a vector of pointer with a map that can be used to lookup specific values
 */
template <class VType, class searchType = std::string>
class MappedPointerVector
{
  public:
    MappedPointerVector () = default;
    MappedPointerVector (MappedPointerVector &&mp) = default;
    MappedPointerVector &operator= (MappedPointerVector &&mp) = default;

    stx::optional<size_t> insert (const searchType &searchValue, std::unique_ptr<VType> &&ptr)
    {
        auto fnd = lookup.find (searchValue);
        if (fnd != lookup.end ())
        {
            return stx::nullopt;
        }
        auto index = dataStorage.size ();
        dataStorage.emplace_back (std::move (ptr));
        lookup.emplace (searchValue, index);
        return index;
    }
    /** insert a new element into the vector*/
    template <typename... Us>
    stx::optional<size_t> insert (const searchType &searchValue, Us &&... data)
    {
        auto fnd = lookup.find (searchValue);
        if (fnd != lookup.end ())
        {
            return stx::nullopt;
        }
        auto index = dataStorage.size ();
        dataStorage.emplace_back (std::make_unique<VType> (std::forward<Us> (data)...));
        lookup.emplace (searchValue, index);
        return index;
    }

    size_t insert_or_assign (const searchType &searchValue, std::unique_ptr<VType> &&ptr)
    {
        auto fnd = lookup.find (searchValue);
        if (fnd != lookup.end ())
        {
            dataStorage[fnd->second] = std::move (ptr);
            return fnd->second;
        }
        auto index = dataStorage.size ();
        dataStorage.emplace_back (std::move (ptr));
        lookup.emplace (searchValue, index);
        return index;
    }
    /** insert a new element into the vector*/
    template <typename... Us>
    size_t insert_or_assign (const searchType &searchValue, Us &&... data)
    {
        auto fnd = lookup.find (searchValue);
        if (fnd != lookup.end ())
        {
            dataStorage[fnd->second] = std::make_unique<VType> (std::forward<Us> (data)...);
            return fnd->second;
        }
        auto index = dataStorage.size ();
        dataStorage.emplace_back (std::make_unique<VType> (std::forward<Us> (data)...));
        lookup.emplace (searchValue, index);
        return index;
    }

    /** get a pointer to the last element inserted*/
    VType *back () { return dataStorage.back ().get (); }

    /** find an element based on the search value
    @return nullptr if the element is not found
    */
    VType *find (const searchType &searchValue) const
    {
        auto fnd = lookup.find (searchValue);
        if (fnd != lookup.end ())
        {
            return dataStorage[fnd->second].get ();
        }
        return nullptr;
    }

    VType *operator[] (size_t index) const
    {
        return (index < dataStorage.size ()) ? (dataStorage[index].get ()) : nullptr;
    }

    /** remove an element at a specific index
    @param[in] index the index of the element to remove*/
    void removeIndex (size_t index)
    {
        if (index >= dataStorage.size ())
        {
            return;
        }
        dataStorage.erase (dataStorage.begin () + index);
        searchType ind;
        for (auto &el2 : lookup)
        {
            if (el2.second > index)
            {
                el2.second -= 1;
            }
            else if (el2.second == index)
            {
                ind = el2.first;
            }
        }
        auto fnd = lookup.find (ind);
        if (fnd != lookup.end ())
        {
            lookup.erase (fnd);
        }
    }
    /** remove an element based on the lookup index*/
    void remove (const searchType &search)
    {
        auto el = lookup.find (search);
        if (el == lookup.end ())
        {
            return;
        }
        auto index = el->second;
        dataStorage.erase (dataStorage.begin () + index);
        for (auto &el2 : lookup)
        {
            if (el2.second > index)
            {
                el2.second -= 1;
            }
        }
        lookup.erase (el);
    }

    /** apply a function to all the values
    @param F must be a function with signature like void fun(VType *a);*/
    template <class UnaryFunction>
    void apply (UnaryFunction F)
    {
        for (auto &vp : dataStorage)
        {
            F (vp.get ());
        }
    }

    /** apply a function to all the values
    @param F must be a function with signature like void fun(const VType *a);*/
    template <class UnaryFunction>
    void apply (UnaryFunction F) const
    {
        for (auto &vp : dataStorage)
        {
            F (vp.get ());
        }
    }
    /** get a const iterator to the start of the data*/
    auto begin () const { return dataStorage.cbegin (); }
    /** get a constant iterator to the end of the data*/
    auto end () const { return dataStorage.cend (); }
    /** get the number of elements in the data*/
    auto size () const { return dataStorage.size (); }
    /** remove all elements from the data*/
    void clear ()
    {
        dataStorage.clear ();
        lookup.clear ();
    }

  private:
    std::vector<std::unique_ptr<VType>> dataStorage;  //!< storage for the pointers
    std::conditional_t<is_easily_hashable<searchType>::value,
                       std::unordered_map<searchType, size_t>,
                       std::map<searchType, size_t>>
      lookup;  //!< map to lookup the index lookup;
};
