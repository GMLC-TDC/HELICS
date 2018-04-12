/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once
#include <unordered_map>
#include <map>
#include <type_traits>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include "MapTraits.hpp"
#include "helics_includes/optional.hpp"

/** class merging a vector of pointer with a map that can be used to lookup specific values
*/
template <class VType, class searchType1, class searchType2>
class DualMappedPointerVector
{
	static_assert(!std::is_same<searchType1, searchType2>::value, "searchType1 and searchType2 cannot be the same type");
public:
	DualMappedPointerVector() = default;
	DualMappedPointerVector(DualMappedPointerVector &&mp) = default;
	DualMappedPointerVector &operator=(DualMappedPointerVector &&mp) = default;
	/** insert a new element into the vector directly from an existing unique ptr*/
    stx::optional<size_t> insert(const searchType1 &searchValue1,const searchType2 &searchValue2, std::unique_ptr<VType> &&ptr)
	{
		auto fnd = lookup1.find(searchValue1);
		if (fnd != lookup1.end())
		{
            return stx::nullopt;
		}
		else
		{
			auto index = dataStorage.size();
			dataStorage.emplace_back(std::move(ptr));
			lookup1.emplace(searchValue1, index);
			lookup2.emplace(searchValue2, index);
			return index;
		}
	}
	/** insert a new element into the vector*/
	template <typename... Us>
    stx::optional<size_t> insert(const searchType1 &searchValue1, const searchType2 &searchValue2, Us &&... data)
	{
		auto fnd = lookup1.find(searchValue1);
		if (fnd != lookup1.end())
		{
            return stx::nullopt;
		}
		else
		{
			auto index = dataStorage.size();
			dataStorage.emplace_back(std::make_unique<VType>(std::forward<Us>(data)...));
			lookup1.emplace(searchValue1, index);
			lookup2.emplace(searchValue2, index);
			return index;
		}
	}

    /** insert a new element into the vector*/
    template <typename... Us>
    stx::optional<size_t> insert(const searchType1 &searchValue1, std::nullptr_t /*unused*/, Us &&... data)
    {
        auto fnd = lookup1.find(searchValue1);
        if (fnd != lookup1.end())
        {
            return stx::nullopt;
        }
        else
        {
            auto index = dataStorage.size();
            dataStorage.emplace_back(std::make_unique<VType>(std::forward<Us>(data)...));
            lookup1.emplace(searchValue1, index);
            return index;
        }
    }

    /** insert a new element into the vector*/
    template <typename... Us>
    stx::optional<size_t> insert(std::nullptr_t /*unused*/, const searchType2 &searchValue2, Us &&... data)
    {
        auto fnd = lookup2.find(searchValue2);
        if (fnd != lookup2.end())
        {
            return stx::nullopt;
        }
        else
        {
            auto index = dataStorage.size();
            dataStorage.emplace_back(std::make_unique<VType>(std::forward<Us>(data)...));
            lookup2.emplace(searchValue2, index);
            return index;
        }
    }

    /** insert a new element into the vector directly from an existing unique ptr*/
    size_t insert_or_assign(const searchType1 &searchValue1, const searchType2 &searchValue2, std::unique_ptr<VType> &&ptr)
    {
        auto fnd = lookup1.find(searchValue1);
        if (fnd != lookup1.end())
        {
            dataStorage[fnd->second] = std::move(ptr);
            lookup2[searchValue2] = fnd->second;
            return fnd->second;
        }
        else
        {
            auto index = dataStorage.size();
            dataStorage.emplace_back(std::move(ptr));
            lookup1.emplace(searchValue1, index);
            lookup2.emplace(searchValue2, index);
            return index;
        }
    }
    /** insert a new element into the vector*/
    template <typename... Us>
    size_t insert_or_assign(const searchType1 &searchValue1, const searchType2 &searchValue2, Us &&... data)
    {
        auto fnd = lookup1.find(searchValue1);
        if (fnd != lookup1.end())
        {
            dataStorage[fnd->second] = std::make_unique<VType>(std::forward<Us>(data)...);
            lookup2[searchValue2] = fnd->second;
            return fnd->second;
        }
        else
        {
            auto index = dataStorage.size();
            dataStorage.emplace_back(std::make_unique<VType>(std::forward<Us>(data)...));
            lookup1.emplace(searchValue1, index);
            lookup2.emplace(searchValue2, index);
            return index;
        }
    }

    /** insert a new element into the vector*/
    template <typename... Us>
    size_t insert_or_assign(const searchType1 &searchValue1, std::nullptr_t /*unused*/, Us &&... data)
    {
        auto fnd = lookup1.find(searchValue1);
        if (fnd != lookup1.end())
        {
            dataStorage[fnd->second] = std::make_unique<VType>(std::forward<Us>(data)...);
            return fnd->second;
        }
        else
        {
            auto index = dataStorage.size();
            dataStorage.emplace_back(std::make_unique<VType>(std::forward<Us>(data)...));
            lookup1.emplace(searchValue1, index);
            return index;
        }
    }

    /** insert a new element into the vector*/
    template <typename... Us>
    size_t insert_or_assign(std::nullptr_t /*unused*/, const searchType2 &searchValue2, Us &&... data)
    {
        auto fnd = lookup2.find(searchValue2);
        if (fnd != lookup2.end())
        {
            dataStorage[fnd->second] = std::make_unique<VType>(std::forward<Us>(data)...);
            return fnd->second;
        }
        else
        {
            auto index = dataStorage.size();
            dataStorage.emplace_back(std::make_unique<VType>(std::forward<Us>(data)...));
            lookup2.emplace(searchValue2, index);
            return index;
        }
    }

	/** find an element based on the search value
	@return nullptr if the element is not found
	*/
	VType *find(const searchType1 &searchValue1) const
	{
		auto fnd = lookup1.find(searchValue1);
		if (fnd != lookup1.end())
		{
			return dataStorage[fnd->second].get();
		}
		return nullptr;
	}

	/** find an element based on the search value
	@return nullptr if the element is not found
	*/
	VType *find(const searchType2 &searchValue2) const
	{
		auto fnd = lookup2.find(searchValue2);
		if (fnd != lookup2.end())
		{
			return dataStorage[fnd->second].get();
		}
		return nullptr;
	}

	VType *operator[] (size_t index) const{ return(index<dataStorage.size()) ? (dataStorage[index].get()) : nullptr; }

	/** get a pointer to the last element inserted*/
	VType *back() { return dataStorage.back().get(); }
	/** remove an element at a specific index
	@param[in] index the index of the element to remove*/
	void removeIndex(size_t index)
	{
		if (index >= dataStorage.size())
		{
			return;
		}
		dataStorage.erase(dataStorage.begin() + index);
		searchType1 ind1;
		searchType2 ind2;
		for (auto &el2 : lookup1)
		{
			if (el2.second > index)
			{
				el2.second -= 1;
			}
			else if (el2.second == index)
			{
				ind1 = el2.first;
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
				ind2 = el2.first;
			}
		}
		auto fnd1 = lookup1.find(ind1);
		if (fnd1 != lookup1.end())
		{
			lookup1.erase(fnd1);
		}
		auto fnd2 = lookup2.find(ind2);
		if (fnd2 != lookup2.end())
		{
			lookup2.erase(fnd2);
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
		dataStorage.erase(dataStorage.begin() + index);
		for (auto &el2 : lookup1)
		{
			if (el2.second > index)
			{
				el2.second -= 1;
			}
		}
		lookup1.erase(el);
		searchType2 ind2;
		for (auto &el2 : lookup2)
		{
			if (el2.second > index)
			{
				el2.second -= 1;
			}
			else if (el2.second == index)
			{
				ind2 = el2.first;
			}
		}
		auto fnd2 = lookup2.find(ind2);
		if (fnd2 != lookup2.end())
		{
			lookup2.erase(fnd2);
		}
	}

	void remove(const searchType2 &search)
	{
		auto el = lookup2.find(search);
		if (el == lookup2.end())
		{
			return;
		}
		auto index = el->second;
		dataStorage.erase(dataStorage.begin() + index);
		for (auto &el2 : lookup2)
		{
			if (el2.second > index)
			{
				el2.second -= 1;
			}
		}
		lookup2.erase(el);
		searchType1 ind1;
		for (auto &el2 : lookup1)
		{
			if (el2.second > index)
			{
				el2.second -= 1;
			}
			else if (el2.second == index)
			{
				ind1 = el2.first;
			}
		}
		auto fnd1 = lookup1.find(ind1);
		if (fnd1 != lookup1.end())
		{
			lookup1.erase(fnd1);
		}
	}

	/** apply a function to all the values
	@param F must be a function with signature like void fun(VType *a);*/
	template<class UnaryFunction >
	void apply(UnaryFunction F)
	{
		for (auto &vp : dataStorage)
		{
			F(vp.get());
		}
	}

	/** apply a function to all the values
	@param F must be a function with signature like void fun(const VType *a);*/
	template<class UnaryFunction >
	void apply(UnaryFunction F) const
	{
		for (auto &vp : dataStorage)
		{
			F(vp.get());
		}
	}
	/** get a const iterator to the start of the data*/
	auto begin() const { return dataStorage.cbegin(); }
	/** get a constant iterator to the end of the data*/
	auto end() const { return dataStorage.cend(); }
	/** get the number of elements in the data*/
	auto size() const { return dataStorage.size(); }
	/** remove all elements from the data*/
	void clear()
	{
		dataStorage.clear();
		lookup1.clear();
		lookup2.clear();
	}

private:
	std::vector<std::unique_ptr<VType>> dataStorage; //!< storage for the pointers
	std::conditional_t<is_easily_hashable<searchType1>::value,std::unordered_map<searchType1,size_t>,std::map<searchType1,size_t>> lookup1;	//!< map to lookup the index
    std::conditional_t<is_easily_hashable<searchType2>::value, std::unordered_map<searchType2, size_t>, std::map<searchType2, size_t>> lookup2;	//!< map to lookup the index
};

