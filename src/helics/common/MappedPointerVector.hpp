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
#include <memory>
#include <string>
#include <vector>

/** class merging a vector of pointer with a map that can be used to lookup specific values
*/
template <class VType, class searchType = std::string>
class MappedPointerVector
{
public:
	MappedPointerVector() = default;
	MappedPointerVector(MappedPointerVector &&mp) = default;
	MappedPointerVector &operator=(MappedPointerVector &&mp) = default;

	size_t insert(const searchType &searchValue, std::unique_ptr<VType> &&ptr)
	{
		auto fnd = lookup.find(searchValue);
		if (fnd != lookup.end())
		{
			dataStorage_[fnd->second] = std::move(ptr);
			return fnd->second;
		}
		else
		{
			auto index = dataStorage_.size();
			dataStorage_.emplace_back(std::move(ptr));
			lookup.emplace(searchValue, index);
			return index;
		}
	}
	/** insert a new element into the vector*/
	template <typename... Us>
	size_t insert(const searchType &searchValue, Us &&... data)
	{
		auto fnd = lookup.find(searchValue);
		if (fnd != lookup.end())
		{
			dataStorage_[fnd->second] = std::make_unique<VType>(std::forward<Us>(data)...);
			return fnd->second;
		}
		else
		{
			auto index = dataStorage_.size();
			dataStorage_.emplace_back(std::make_unique<VType>(std::forward<Us>(data)...));
			lookup.emplace(searchValue, index);
			return index;
		}
	}

	/** get a pointer to the last element inserted*/
	VType *back() { return dataStorage_.back().get(); }

	/** find an element based on the search value
	@return nullptr if the element is not found
	*/
	VType *find(const searchType &searchValue)
	{
		auto fnd = lookup.find(searchValue);
		if (fnd != lookup.end())
		{
			return dataStorage_[fnd->second].get();
		}
		return nullptr;
	}

	/** find an element based on the search value
	@return nullptr if the element is not found
	*/
	const VType *find(const searchType &searchValue) const
	{
		auto fnd = lookup.find(searchValue);
		if (fnd != lookup.end())
		{
			return dataStorage_[fnd->second].get();
		}
		return nullptr;
	}

	VType *operator[] (size_t index) {return(index<dataStorage_.size())?(dataStorage_[index].get()):nullptr; }

	const VType *operator[] (size_t index) const { return(index<dataStorage_.size()) ? (dataStorage_[index].get()) : nullptr; }
	/** remove an element at a specific index
	@param[in] index the index of the element to remove*/
	void removeIndex(size_t index)
	{
		if (index >= dataStorage_.size())
		{
			return;
		}
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
	/** remove an element based on the lookup index*/
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
	/** get a const iterator to the start of the data*/
	auto begin() const { return dataStorage_.cbegin(); }
	/** get a constant iterator to the end of the data*/
	auto end() const { return dataStorage_.cend(); }
	/** get the numer of elements in the data*/
	auto size() const { return dataStorage_.size(); }
	/** remove all elements from the data*/
	void clear()
	{
		dataStorage_.clear();
		lookup.clear();
	}

private:
	std::vector<std::unique_ptr<VType>> dataStorage_; //!< storage for the pointers
	std::unordered_map<searchType, size_t> lookup;	//!< map to lookup the index
};
