/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "MapTraits.hpp"
#include "helics_includes/optional.hpp"
#include <algorithm>
#include <deque>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

/** class combining a vector of objects with a map to search them by a separate index term
the main use case is a bunch of inserts then searching with limited to no removal since removal is a rather
expensive operation
*/
template <class VType,
          class searchType = std::string,
          reference_stability STABILITY = reference_stability::unstable>
class MappedVector
{
  public:
    /** insert an element into the mapped vector
    @param searchValue the unique index to use for the value if it exists the existing value is replaced
    @param data all other arguments required to create a new data value
    @return an optional with the index of the value placed if it was placed
    */
    template <typename... Us>
    stx::optional<size_t> insert (const searchType &searchValue, Us &&... data)
    {
        auto fnd = lookup.find (searchValue);
        if (fnd != lookup.end ())
        {
            return stx::nullopt;
        }
        auto index = dataStorage.size ();
        dataStorage.emplace_back (std::forward<Us> (data)...);
        lookup.emplace (searchValue, index);
        return index;
    }
    /** insert an element into the mapped vector
    @return an optional with the index of the value placed if it was placed
    */
    template <typename... Us>
    stx::optional<size_t> insert (std::nullptr_t /*searchValue*/, Us &&... data)
    {
        auto index = dataStorage.size ();
        dataStorage.emplace_back (std::forward<Us> (data)...);
        return index;
    }

    /** insert an element into the mapped vector
    @param searchValue the unique index to use for the value if it exists the existing value is replaced
    @param data all other values and parameters required to create a new searchType object
    @return the index of the value placed
    */
    template <typename... Us>
    size_t insert_or_assign (const searchType &searchValue, Us &&... data)
    {
        auto fnd = lookup.find (searchValue);
        if (fnd != lookup.end ())
        {
            dataStorage[fnd->second] = VType (std::forward<Us> (data)...);
            return fnd->second;
        }
        auto index = dataStorage.size ();
        dataStorage.emplace_back (std::forward<Us> (data)...);
        lookup.emplace (searchValue, index);
        return index;
    }

    auto find (const searchType &searchValue)
    {
        auto fnd = lookup.find (searchValue);
        if (fnd != lookup.end ())
        {
            return dataStorage.begin () + fnd->second;
        }
        return dataStorage.end ();
    }

    auto find (const searchType &searchValue) const
    {
        auto fnd = lookup.find (searchValue);
        if (fnd != lookup.end ())
        {
            return dataStorage.begin () + fnd->second;
        }
        return dataStorage.end ();
    }

    VType &operator[] (size_t index) { return dataStorage[index]; }

    const VType &operator[] (size_t index) const { return dataStorage[index]; }

    /** get the last element of the vector*/
    VType &back () { return dataStorage.back (); }

    /** get a const reference to the last element of the vector*/
    const VType &back () const { return dataStorage.back (); }
    /** remove an element by its index*/
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
    @param F must be a function with signature like void fun(const VType &a);*/
    template <class UnaryFunction>
    void apply (UnaryFunction F)
    {
        std::for_each (dataStorage.begin (), dataStorage.end (), F);
    }

    /** transform all the values
    F must be a function with signature like void VType(const VType &a);*/
    template <class UnaryFunction>
    void transform (UnaryFunction F)
    {
        std::transform (dataStorage.begin (), dataStorage.end (), dataStorage.begin (), F);
    }
    /*NOTE:: only constant iterators allowed since this would introduce the possibility
    of using iterators for various algorithms which could cause the object to go to a indeterminate state
    therefore constant iterators are allowed but not modifiable iterators
    someone determined to screw it up could still easily do so*/

    /** get a const iterator to the beginning of the data vector*/
    auto begin () const { return dataStorage.cbegin (); }
    /** the a constant iterator to the end of the vector*/
    auto end () const { return dataStorage.cend (); }

    /** get the size of the vector*/
    auto size () const { return dataStorage.size (); }

    /** clear the vector of all data*/
    void clear ()
    {
        dataStorage.clear ();
        lookup.clear ();
    }

  private:
    std::conditional_t<STABILITY == reference_stability::unstable, std::vector<VType>, std::deque<VType>>
      dataStorage;  //!< primary storage for data
    std::conditional_t<is_easily_hashable<searchType>::value,
                       std::unordered_map<searchType, size_t>,
                       std::map<searchType, size_t>>
      lookup;  //!< map to lookup the index
};
