/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_DELAYED_DESTRUCTOR_HPP_
#define HELICS_DELAYED_DESTRUCTOR_HPP_
#pragma once

#include <algorithm>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

/** helper class to destroy objects at a late time when it is convenient and there are no more possibilities of
 * threading issues*/
template <class X>
class DelayedDestructor
{
  private:
    std::mutex destructionLock;
    std::vector<std::shared_ptr<X>> ElementsToBeDestroyed;

  public:
    DelayedDestructor () = default;
    ~DelayedDestructor ()
    {
        while (!ElementsToBeDestroyed.empty ())
        {
            destroyObjects ();
            if (!ElementsToBeDestroyed.empty ())
            {
                std::this_thread::sleep_for (std::chrono::milliseconds (50));
            }
        }
    }
    DelayedDestructor (DelayedDestructor &&) noexcept = delete;
    DelayedDestructor &operator= (DelayedDestructor &&) noexcept = delete;

    size_t destroyObjects ()
    {
        std::lock_guard<std::mutex> lock (destructionLock);
        if (!ElementsToBeDestroyed.empty ())
        {
            auto loc = std::remove_if (ElementsToBeDestroyed.begin (), ElementsToBeDestroyed.end (),
                                       [](const auto &element) { return (element.use_count () <= 1); });
            ElementsToBeDestroyed.erase (loc, ElementsToBeDestroyed.end ());
        }
        return ElementsToBeDestroyed.size ();
    }
    void addObjectsToBeDestroyed (std::shared_ptr<X> &&obj)
    {
        std::lock_guard<std::mutex> lock (destructionLock);
        ElementsToBeDestroyed.push_back (std::move (obj));
    }
    void addObjectsToBeDestroyed (std::shared_ptr<X> &obj)
    {
        std::lock_guard<std::mutex> lock (destructionLock);
        ElementsToBeDestroyed.push_back (obj);
    }
};
#endif  // HELICS_DELAYED_DESTRUCTOR_HPP_
