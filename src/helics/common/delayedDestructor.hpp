/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "TripWire.hpp"
#include <algorithm>
#include <chrono>
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
        try
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
        catch (...)
        {
        }
    }
    DelayedDestructor (DelayedDestructor &&) noexcept = delete;
    DelayedDestructor &operator= (DelayedDestructor &&) noexcept = delete;

    /** destroy objects that are no longer used*/
    size_t destroyObjects () noexcept
    {
        try
        {
            std::unique_lock<std::mutex> lock (destructionLock);
            if (!ElementsToBeDestroyed.empty ())
            {
                std::vector<std::shared_ptr<X>> ecall;
                std::vector<std::string> ename;
                for (auto &element : ElementsToBeDestroyed)
                {
                    if (element.use_count () == 1)
                    {
                        ecall.push_back (element);
                        ename.push_back (element->getIdentifier ());
                    }
                }
                if (!ename.empty ())
                {
                    // so apparently remove_if can actually call the destructor for shared_ptrs so the call
                    // function needs to be before this call
                    auto loc =
                      std::remove_if (ElementsToBeDestroyed.begin (), ElementsToBeDestroyed.end (),
                                      [&ename](const auto &element) {
                                          return ((element.use_count () == 2) &&
                                                  (std::find (ename.begin (), ename.end (),
                                                              element->getIdentifier ()) != ename.end ()));
                                      });
                    ElementsToBeDestroyed.erase (loc, ElementsToBeDestroyed.end ());
                    auto deleteFunc = callBeforeDeleteFunction;
                    lock.unlock ();
                    // this needs to be done after the lock, so a destructor can never called while under the lock
                    if (deleteFunc)
                    {
                        for (auto &element : ecall)
                        {
                            deleteFunc (element);
                        }
                    }
                    ecall.clear ();  // make sure the destructors get called before returning.
                    lock.lock ();  // reengage the lock so the size is correct
                }
            }
        }
        catch (...)
        {
        }
        return ElementsToBeDestroyed.size ();
    }

    size_t destroyObjects (std::chrono::milliseconds delay)
    {
        using namespace std::literals::chrono_literals;
        std::unique_lock<std::mutex> lock (destructionLock);
        auto delayTime = (delay < 100ms) ? delay : 50ms;
        int delayCount = (delay < 100ms) ? 1 : (delay / 50).count ();

        int cnt = 0;
        while ((!ElementsToBeDestroyed.empty ()) && (cnt < delayCount))
        {
            if (cnt > 0)  // don't sleep on the first loop
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
                lock.unlock ();
                destroyObjects ();
                lock.lock ();
            }
        }
        return ElementsToBeDestroyed.size ();
    }

    void addObjectsToBeDestroyed (std::shared_ptr<X> obj)
    {
        std::lock_guard<std::mutex> lock (destructionLock);
        ElementsToBeDestroyed.push_back (std::move (obj));
    }
};
