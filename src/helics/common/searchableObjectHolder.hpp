/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_SEARCHABLE_OBJECT_HOLDER_HPP_
#define HELICS_SEARCHABLE_OBJECT_HOLDER_HPP_
#pragma once

#include <map>
#include <memory>
#include <mutex>

/** helper class to destroy objects at a late time when it is convenient and there are no more possibilities of
 * threading issues*/
template <class X>
class SearchableObjectHolder
{
  private:
    std::mutex mapLock;
    std::map<std::string, std::shared_ptr<X>> ObjectMap;

  public:
    SearchableObjectHolder () = default;
    ~SearchableObjectHolder ()
    {
        std::unique_lock<std::mutex> lock (mapLock);
        while (true)
        {
            if (!ObjectMap.empty ())
            {  // wait for the objectMap to be cleared
                lock.unlock ();
                std::this_thread::sleep_for (std::chrono::milliseconds (50));
                lock.lock ();
            }
            else
            {
                break;
            }
        }
    }
    bool addObject (const std::string &name, std::shared_ptr<X> &obj)
    {
        std::lock_guard<std::mutex> lock (mapLock);
        auto res = ObjectMap.emplace (name, obj);
        return res.second;
    }

    bool removeObject (const std::string &name)
    {
        std::lock_guard<std::mutex> lock (mapLock);
        auto fnd = ObjectMap.find (name);
        if (fnd != ObjectMap.end ())
        {
            ObjectMap.erase (fnd);
            return true;
        }
        return false;
    }

    bool removeObject (std::function<bool(const std::shared_ptr<X> &)> operand)
    {
        std::lock_guard<std::mutex> lock (mapLock);
        for (auto obj = ObjectMap.begin (); obj != ObjectMap.end (); ++obj)
        {
            if (operand (obj->second))
            {
                ObjectMap.erase (obj);
                return true;
            }
        }
        return false;
    }

    bool copyObject (const std::string &copyFromName, const std::string &copyToName)
    {
        std::lock_guard<std::mutex> lock (mapLock);
        auto fnd = ObjectMap.find (copyFromName);
        if (fnd != ObjectMap.end ())
        {
            auto newObjectPtr = fnd->second;
            auto ret = ObjectMap.emplace (copyToName, std::move (newObjectPtr));
            return ret.second;
        }
        return false;
    }

    std::shared_ptr<X> findObject (const std::string &name)
    {
        std::lock_guard<std::mutex> lock (mapLock);
        auto fnd = ObjectMap.find (name);
        if (fnd != ObjectMap.end ())
        {
            return fnd->second;
        }
        return nullptr;
    }

    std::shared_ptr<X> findObject (std::function<bool(const std::shared_ptr<X> &)> operand)
    {
        std::lock_guard<std::mutex> lock (mapLock);
        for (auto obj = ObjectMap.begin (); obj != ObjectMap.end (); ++obj)
        {
            if (operand (obj->second))
            {
                return obj->second;
            }
        }
        return nullptr;
    }
};
#endif /*HELICS_SEARCHABLE_OBJECT_HOLDER_HPP_*/