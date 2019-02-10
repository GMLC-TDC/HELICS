/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include <future>
#include <map>
#include <mutex>

/** class holding a set of delayed object the delayed object are held by promises*/
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
    DelayedObjects &operator= (const DelayedObjects &) = delete;
    DelayedObjects &operator= (DelayedObjects &&) = delete;

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
