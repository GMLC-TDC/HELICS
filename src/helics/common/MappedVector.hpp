
/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
/** class combining a vector of objects with a map to search them by a separate index term
the main use case is a bunch of inserts then searching with limited to no removal since removal is a rather
expensive operation
*/
template <class VType, class searchType = std::string>
class MappedVector
{
  public:
	  /** insert an element into the mapped vector
	  @param searchValue the unique index to use for the value if it exists the existing value is replaced
	  @return the index of the value placed
	  */
    template <typename... Us>
    size_t insert (const searchType &searchValue, Us &&... data)
    {
        auto fnd = lookup.find (searchValue);
        if (fnd != lookup.end ())
        {
            dataStorage[fnd->second] = VType (std::forward<Us> (data)...);
			return fnd->second;
        }
        else
        {
			auto index = dataStorage.size();
            dataStorage.emplace_back (std::forward<Us> (data)...);
            lookup.emplace (searchValue, index);
			return index;
        }
    }

    auto find (const searchType &searchValue)
    {
        auto fnd = lookup.find (searchValue);
        if (fnd != lookup.end ())
        {
            return dataStorage.begin () + fnd->second;
        }
        else
        {
            return dataStorage.end ();
        }
    }

    auto find (const searchType &searchValue) const
    {
        auto fnd = lookup.find (searchValue);
        if (fnd != lookup.end ())
        {
            return dataStorage.begin () + fnd->second;
        }
        else
        {
            return dataStorage.end ();
        }
    }

    VType &operator[] (size_t index) { return dataStorage[index]; }

    const VType &operator[] (size_t index) const { return dataStorage[index]; }

	/** get the last element of the vector*/
	VType &back() { return dataStorage.back(); }

	/** get a const reference to the last element of the vector*/
	const VType &back() const { return dataStorage.back(); }
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
	/*NOTE:: only constant iterators allowed since this would introduce the possibilty
	of using iterators for various algorithms which could cause the object to go to a indeterminate state
	therefore constant iterators are allowed but not modifable iterators
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
    std::vector<VType> dataStorage;
    std::unordered_map<searchType, size_t> lookup;
};
