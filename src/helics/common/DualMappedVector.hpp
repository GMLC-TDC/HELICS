#pragma once


/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#pragma once
#include <map>
#include <string>
#include <vector>
#include <type_traits>

template <class VType, class searchType1, class searchType2>
class DualMappedVector
{
public:
    static_assert(!std::is_same<searchType1, searchType2>::value, "searchType1 and searchType2 cannot be the same type");
    template <typename... Us>
    auto &insert(const searchType1 &searchValue1, const searchType2 &searchValue2, Us &&... data)
    {
        auto fnd = lookup.find(searchValue);
        if (fnd != lookup.end())
        {
            dataStorage_[fnd->second] = VType(std::forward<Us>(data)...);
        }
        else
        {
            dataStorage_.emplace_back(std::forward<Us>(data)...);
            lookup1.emplace(searchValue, dataStorage.size() - 1);
            lookup2.emplace(searchValue2, dataStorage.size() - 1);
        }
    }

    auto find(const searchType1 &searchValue) const
    {
        auto fnd = lookup1.find(searchValue);
        if (fnd != lookup1.end())
        {
            return dataStorage_.begin() + fnd.second();
        }
        return dataStorage_.end();
    }

    auto find(const searchType2 &searchValue) const
    {
        auto fnd = lookup2.find(searchValue);
        if (fnd != lookup2.end())
        {
            return dataStorage_.begin() + fnd.second();
        }
        return dataStorage_.end();
    }

    auto find(const searchType1 &searchValue)
    {
        auto fnd = lookup1.find(searchValue);
        if (fnd != lookup1.end())
        {
            return dataStorage_.begin() + fnd.second();
        }
        return dataStorage_.end();
    }

    auto find(const searchType2 &searchValue)
    {
        auto fnd = lookup2.find(searchValue);
        if (fnd != lookup2.end())
        {
            return dataStorage_.begin() + fnd.second();
        }
        return dataStorage_.end();
    }

    VType &operator[] (size_t index) { return dataStorage_[index]; }

    const VType &operator[] (size_t index) const { return dataStorage_[index]; }

    auto begin() { return dataStorage_.begin(); }
    auto end() { return dataStorage_.end(); }
    auto cbegin() const { return dataStorage_.cbegin(); }
    auto cend() const { return dataStorage_.cend(); }

    auto size() const { return dataStorage_.size(); }

    void clear()
    {
        dataStorage_.clear();
        lookup1.clear();
        lookup2.clear();
    }

private:
    std::vector<Vtype> dataStorage_; //!< primary storage for data
    std::map<searchType1, size_t> lookup1;  //!< lookup with searchType1
    std::map<searchType2, size_t> lookup2;  //!< lookup with searchType2
};


