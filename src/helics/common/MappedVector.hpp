
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

template <class VType, class searchType = std::string>
class MappedVector
{
  public:
    template <typename... Us>
    auto &insert (const searchType &searchValue, Us &&... data)
    {
        auto fnd = lookup.find (searchValue);
        if (fnd != lookup.end ())
        {
            dataStorage_[fnd->second] = VType (std::forward<Us> (data)...);
        }
        else
        {
            dataStorage_.emplace_back (std::forward<Us> (data)...);
            lookup.emplace (searchValue, dataStorage.size () - 1);
        }
    }

    auto find (const searchType &searchValue)
    {
        auto fnd = lookup.find (searchValue);
        if (fnd != lookup.end ())
        {
            return dataStorage_.begin () + fnd.second ();
        }
        else
        {
            return dataStorage_.end ();
        }
    }

    VType &operator[] (size_t index) { return dataStorage_[index]; }

    const VType &operator[] (size_t index) const { return dataStorage_[index]; }

    auto begin () { return dataStorage_.begin (); }
    auto end () { return dataStorage_.end (); }
    auto cbegin () const { return dataStorage_.cbegin (); }
    auto cend () const { return dataStorage_.cend (); }

    auto size () const { return dataStorage_.size (); }

    void clear ()
    {
        dataStorage_.clear ();
        lookup.clear ();
    }

  private:
    std::vector<Vtype> dataStorage_;
    std::map<searchType, size_t> lookup;
};
