#pragma once

/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

template <class VType, class searchType = std::string>
class MappedPointerVector
{
public:
		MappedPointerVector() = default;
	MappedPointerVector(MappedPointerVector &&mp) = default;
	MappedPointerVector &operator=(MappedPointerVector &&mp) = default;
    template <typename... Us>
    void insert (const searchType &searchValue, Us &&... data)
    {
        auto fnd = lookup.find (searchValue);
        if (fnd != lookup.end ())
        {
            dataStorage_[fnd->second] = std::make_unique<VType> (std::forward<Us> (data)...);
        }
        else
        {
            dataStorage_.emplace_back (std::make_unique<VType> (std::forward<Us> (data)...));
            lookup.emplace (searchValue, dataStorage_.size () - 1);
        }
    }

    VType *find (const searchType &searchValue)
    {
        auto fnd = lookup.find (searchValue);
        if (fnd != lookup.end ())
        {
            return dataStorage_[fnd->second].get ();
        }
        else
        {
            return nullptr;
        }
    }

    VType *operator[] (size_t index) { return dataStorage_[index].get (); }

    const VType *operator[] (size_t index) const { return dataStorage_[index].get (); }

	void removeIndex(size_t index)
	{
		dataStorage_.erase(dataStorage_.begin() + index);
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
		auto fnd = lookup.find(ind);
		if (fnd != lookup.end())
		{
			lookup.erase(fnd);
		}
	}

	void remove(const searchType &search)
	{
		auto el = lookup.find(search);
		if (el == lookup.end())
		{
			return;
		}
		auto index = el->second;
		dataStorage_.erase(dataStorage_.begin() + index);
		for (auto &el2 : lookup)
		{
			if (el2.second > index)
			{
				el2.second -= 1;
			}
		}
		lookup.erase(el);
	}

	/** apply a function to all the values
	@param F must be a function with signature like void fun(VType *a);*/
	template<class UnaryFunction >
	void apply(UnaryFunction F)
	{
		for (auto &vp : dataStorage_)
		{
			F(vp.get());
		}
	}

	/** apply a function to all the values
	@param F must be a function with signature like void fun(const VType *a);*/
	template<class UnaryFunction >
	void apply(UnaryFunction F) const
	{
		for (auto &vp : dataStorage_)
		{
			F(vp.get());
		}
	}

    auto cbegin () const { return dataStorage_.cbegin (); }
    auto cend () const { return dataStorage_.cend (); }

    auto size () const { return dataStorage_.size (); }

    void clear ()
    {
        dataStorage_.clear ();
        lookup.clear ();
    }

  private:
    std::vector<std::unique_ptr<VType>> dataStorage_;
    std::map<searchType, size_t> lookup;
};
