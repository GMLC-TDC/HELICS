/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once

#include "TripWire.hpp"
#include <algorithm>
#include <functional>
#include <iostream>
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
    std::function<void(std::shared_ptr<X> &ptr)> callBeforeDeleteFunction;
    tripwire::TripWireDetector tripDetect;

  public:
    DelayedDestructor () = default;
    explicit DelayedDestructor (std::function<void(std::shared_ptr<X> &ptr)> callFirst)
        : callBeforeDeleteFunction (std::move (callFirst))
    {
    }
    ~DelayedDestructor ()
    {
        int ii = 0;
        while (!ElementsToBeDestroyed.empty ())
        {
            ++ii;
            destroyObjects ();
            if (!ElementsToBeDestroyed.empty ())
            {
                // short circuit if the tripline was triggered
                if (tripDetect.isTripped ())
                {
                    return;
                }
                if (ii > 4)
                {
                    destroyObjects ();
                    break;
                }
                if (ii % 2 == 0)
                {
                    std::this_thread::sleep_for (std::chrono::milliseconds (100));
                }
                else
                {
                    std::this_thread::yield ();
                }
            }
        }
    }
    DelayedDestructor (DelayedDestructor &&) noexcept = delete;
    DelayedDestructor &operator= (DelayedDestructor &&) noexcept = delete;

    /** destroy objects that are now longer used*/
    size_t destroyObjects ()
    {
        std::lock_guard<std::mutex> lock (destructionLock);
        if (!ElementsToBeDestroyed.empty ())
        {
            if (callBeforeDeleteFunction)
            {
                for (auto &element : ElementsToBeDestroyed)
                {
                    if (element.use_count () == 1)
                    {
                        callBeforeDeleteFunction (element);
                    }
                }
            }
            // so apparently remove_if can actually call the destructor for shared_ptrs so the call function needs
            // to be before this call
            auto loc = std::remove_if (ElementsToBeDestroyed.begin (), ElementsToBeDestroyed.end (),
                                       [](const auto &element) { return (element.use_count () <= 1); });
            ElementsToBeDestroyed.erase (loc, ElementsToBeDestroyed.end ());
        }
        return ElementsToBeDestroyed.size ();
    }

    size_t destroyObjects (int delay)
    {
        std::unique_lock<std::mutex> lock (destructionLock);
        auto delayTime = std::chrono::milliseconds ((delay < 100) ? delay : 50);
        int delayCount = (delay < 100) ? 1 : (delay / 50);

        int cnt = 0;
        while ((!ElementsToBeDestroyed.empty ()) && (cnt < delayCount))
        {
            if (cnt > 0)
            {
                lock.unlock ();
                std::this_thread::sleep_for (delayTime);
                ++cnt;
                lock.lock ();
            }
            else
            {
                ++cnt;
            }

            if (!ElementsToBeDestroyed.empty ())
            {
                auto loc = std::remove_if (ElementsToBeDestroyed.begin (), ElementsToBeDestroyed.end (),
                                           [](const auto &element) { return (element.use_count () <= 1); });
                if (callBeforeDeleteFunction)
                {
                    auto locIt = loc;
                    while (locIt != ElementsToBeDestroyed.end ())
                    {
                        if (*locIt)
                        {
                            callBeforeDeleteFunction (*locIt);
                        }
                        ++locIt;
                    }
                }
                ElementsToBeDestroyed.erase (loc, ElementsToBeDestroyed.end ());
            }
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
