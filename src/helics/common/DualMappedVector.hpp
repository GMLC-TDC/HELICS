#pragma once


/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <type_traits>

/** class to create a searchable vector by defined unique indices.
The result object can be indexed multiple ways both by searching using indices or by numerical index
*/
template <class VType, class searchType1, class searchType2>
class DualMappedVector
{
public:
    static_assert(!std::is_same<searchType1, searchType2>::value, "searchType1 and searchType2 cannot be the same type");
    /** insert a new element into the vector
	@param searchValue1 the primary unique index of the vector
	@param searchValue2 the secondary unique index of the vector*/
	template <typename... Us>
    void insert(const searchType1 &searchValue1, const searchType2 &searchValue2, Us &&... data)
    {
        auto fnd = lookup1.find(searchValue1);
        if (fnd != lookup1.end())
        {
            dataStorage_[fnd->second] = VType(std::forward<Us>(data)...);
			lookup2[searchValue2] = fnd->second;
        }
        else
        {
			auto index = dataStorage_.size();
            dataStorage_.emplace_back(std::forward<Us>(data)...);
            lookup1.emplace(searchValue1,  index);
            lookup2.emplace(searchValue2, index);
        }
    }
	/** add an additional index term for searching*/
	bool addSearchTermForIndex(const searchType1 &searchValue, size_t index)
	{
		if (index < dataStorage_.size())
		{
			auto res = lookup1.emplace(searchValue, index);
			return res.second;
		}
		return false;

	}

	/** add an additional index term for searching*/
	auto addSearchTerm(const searchType1 &searchValue, const searchType1 &existingValue)
	{
		auto fnd = lookup1.find(existingValue);
		if (fnd != lookup1.end())
		{
			auto res=lookup1.emplace(searchValue, fnd->second);
			return res.second;
		}
		return false;

	}

	/** add an additional index term for searching*/
	bool addSearchTermForIndex(const searchType2 &searchValue, size_t index)
	{
		if (index < dataStorage_.size())
		{
			auto res = lookup2.emplace(searchValue, index);
			return res.second;
		}
		return false;

	}

	/** add an additional index term for searching*/
	auto addSearchTerm(const searchType2 &searchValue, const searchType2 &existingValue)
	{
		auto fnd = lookup2.find(existingValue);
		if (fnd != lookup2.end())
		{
			auto res = lookup2.emplace(searchValue, fnd->second);
			return res.second;
		}
		return false;

	}

	/** add an additional index term for searching*/
	auto addSearchTerm(const searchType2 &searchValue, const searchType1 &existingValue)
	{
		auto fnd = lookup1.find(existingValue);
		if (fnd != lookup1.end())
		{
			auto res = lookup2.emplace(searchValue, fnd->second);
			return res.second;
		}
		return false;

	}

	/** add an additional index term for searching*/
	auto addSearchTerm(const searchType1 &searchValue, const searchType2 &existingValue)
	{
		auto fnd = lookup2.find(existingValue);
		if (fnd != lookup2.end())
		{
			auto res = lookup1.emplace(searchValue, fnd->second);
			return res.second;
		}
		return false;

	}
    auto find(const searchType1 &searchValue) const
    {
        auto fnd = lookup1.find(searchValue);
        if (fnd != lookup1.end())
        {
            return dataStorage_.begin() + fnd->second;
        }
        return dataStorage_.end();
    }

    auto find(const searchType2 &searchValue) const
    {
        auto fnd = lookup2.find(searchValue);
        if (fnd != lookup2.end())
        {
            return dataStorage_.begin() + fnd->second;
        }
        return dataStorage_.end();
    }

    auto find(const searchType1 &searchValue)
    {
        auto fnd = lookup1.find(searchValue);
        if (fnd != lookup1.end())
        {
            return dataStorage_.begin() + fnd->second;
        }
        return dataStorage_.end();
    }

    auto find(const searchType2 &searchValue)
    {
        auto fnd = lookup2.find(searchValue);
        if (fnd != lookup2.end())
        {
            return dataStorage_.begin() + fnd->second;
        }
        return dataStorage_.end();
    }

	void removeIndex(size_t index)
	{
		if (index >= dataStorage_.size())
		{
			return;
		}
		dataStorage_.erase(dataStorage_.begin() + index);
		std::vector<searchType1> ind1(2);
		std::vector<searchType2> ind2(2);
		for (auto &el2 : lookup1)
		{
			if (el2.second > index)
			{
				el2.second -= 1;
			}
			else if (el2.second == index)
			{
				ind1.push_back(el2.first);
			}
		}
		for (auto &el2 : lookup2)
		{
			if (el2.second > index)
			{
				el2.second -= 1;
			}
			else if (el2.second == index)
			{
				ind2.push_back(el2.first);
			}
		}
		for (auto &ind : ind1)
		{
			auto fnd1 = lookup1.find(ind);
			if (fnd1 != lookup1.end())
			{
				lookup1.erase(fnd1);
			}
		}
		
		for (auto &ind : ind2)
		{
			auto fnd2 = lookup2.find(ind);
			if (fnd2 != lookup2.end())
			{
				lookup2.erase(fnd2);
			}
		}
	}

	void remove(const searchType1 &search)
	{
		auto el = lookup1.find(search);
		if (el == lookup1.end())
		{
			return;
		}
		auto index = el->second;
		removeIndex(index);
	}

	void remove(const searchType2 &search)
	{
		auto el = lookup2.find(search);
		if (el == lookup2.end())
		{
			return;
		}
		auto index = el->second;
		removeIndex(index);
	}
    VType &operator[] (size_t index) { return dataStorage_[index]; }

    const VType &operator[] (size_t index) const { return dataStorage_[index]; }

	VType &back() { return dataStorage_.back(); }

	const VType &back() const{ return dataStorage_.back(); }
	/** apply a function to all the values
	@param F must be a function with signature like void fun(const VType &a);*/
	template<class UnaryFunction >
	void apply(UnaryFunction F)
	{
		std::for_each(dataStorage_.begin(), dataStorage_.end(), F);
	}

	/** transform all the values
	F must be a function with signature like void VType(const VType &a);*/
	template<class UnaryFunction >
	void transform(UnaryFunction F)
	{
		std::transform(dataStorage_.begin(), dataStorage_.end(), dataStorage_.begin(), F);
	}
    auto begin() const { return dataStorage_.cbegin(); }
    auto end() const { return dataStorage_.cend(); }

    auto size() const { return dataStorage_.size(); }

    void clear()
    {
        dataStorage_.clear();
        lookup1.clear();
        lookup2.clear();
    }

private:
    std::vector<VType> dataStorage_; //!< primary storage for data
    std::unordered_map<searchType1, size_t> lookup1;  //!< lookup with searchType1
    std::unordered_map<searchType2, size_t> lookup2;  //!< lookup with searchType2
};


