/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
*/
#ifndef HELICS_SEARCHABLE_OBJECT_HOLDER_HPP_
#define HELICS_SEARCHABLE_OBJECT_HOLDER_HPP_
#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include "TripWire.hpp"

/** helper class to destroy objects at a late time when it is convenient and there are no more possibilities of
 * threading issues*/
template <class X>
class SearchableObjectHolder
{
  private:
    std::mutex mapLock;
    std::map<std::string, std::shared_ptr<X>> ObjectMap;
    tripwire::TripWireDetector trippedDetect;
  public:
    SearchableObjectHolder () = default;
    SearchableObjectHolder (SearchableObjectHolder &&) noexcept = delete;
    SearchableObjectHolder &operator= (SearchableObjectHolder &&) noexcept = delete;
    ~SearchableObjectHolder ()
    {
        //this is a short circuit used to detect shutdown
        if (trippedDetect.isTripped())
        {
            return;
        }
        std::unique_lock<std::mutex> lock (mapLock);
        int cntr = 0;
        while (true)
        {
            if (!ObjectMap.empty ())
            {  // wait for the objectMap to be cleared
                ++cntr;
                lock.unlock ();
                // don't leave things locked while sleeping or yielding
                if (cntr % 2 != 0)
                {
                    std::this_thread::yield ();
                }
                else
                {
                    std::this_thread::sleep_for (std::chrono::milliseconds (100));
                }

                lock.lock ();
                if (cntr > 6)
                {
                    break;
                }
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

