/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_DELAYED_OBJECTS_
#define _HELICS_DELAYED_OBJECTS_
#pragma once

#include <future>
#include <map>
#include <mutex>

/** class holding a map of delayed object*/
template <class X>
class DelayedObjects
{
  private:
    std::map<int, std::promise<X>> promiseByInteger;
    std::map<std::string, std::promise<X>> promiseByString;
    std::mutex promiseLock;
    std::map<int, std::promise<X>> usedPromiseByInteger;
    std::map<std::string, std::promise<X>> usedPromiseByString;

  public:
    DelayedObjects () = default;
    ~DelayedObjects ()
    {
        std::lock_guard<std::mutex> lock (promiseLock);
        for (auto &obj : promiseByInteger)
        {
            obj.second.set_value (X ());
        }
        for (auto &obj : promiseByString)
        {
            obj.second.set_value (X ());
        }
    }
    // not movable or copyable;
    DelayedObjects (const DelayedObjects &) = delete;
    DelayedObjects (DelayedObjects &&) = delete;

    void setDelayedValue (int index, const X &val)
    {
        std::lock_guard<std::mutex> lock (promiseLock);
        auto fnd = promiseByInteger.find (index);
        if (fnd != promiseByInteger.end ())
        {
            fnd->second.set_value (val);
            usedPromiseByInteger[index] = std::move (fnd->second);
            promiseByInteger.erase (fnd);
        }
    }
    void setDelayedValue (const std::string &name, const X &val)
    {
        std::lock_guard<std::mutex> lock (promiseLock);
        auto fnd = promiseByString.find (name);
        if (fnd != promiseByString.end ())
        {
            fnd->second.set_value (val);
            usedPromiseByString[name] = std::move (fnd->second);
            promiseByString.erase (fnd);
        }
    }
    std::future<X> getFuture (int index)
    {
        auto V = std::promise<X> ();
        auto fut = V.get_future ();
        std::lock_guard<std::mutex> lock (promiseLock);
        promiseByInteger[index] = std::move (V);
        return fut;
    }
    std::future<X> getFuture (const std::string &name)
    {
        auto V = std::promise<X> ();
        auto fut = V.get_future ();
        std::lock_guard<std::mutex> lock (promiseLock);
        promiseByString[name] = std::move (V);
        return fut;
    }

    void finishedWithValue (int index)
    {
        std::lock_guard<std::mutex> lock (promiseLock);
        auto fnd = usedPromiseByInteger.find (index);
        if (fnd != usedPromiseByInteger.end ())
        {
            usedPromiseByInteger.erase (fnd);
        }
    }

    void finishedWithValue (const std::string &name)
    {
        std::lock_guard<std::mutex> lock (promiseLock);
        auto fnd = usedPromiseByString.find (name);
        if (fnd != usedPromiseByString.end ())
        {
            usedPromiseByString.erase (fnd);
        }
    }
};

#endif /* _HELICS_DELAYED_OBJECTS_ */
