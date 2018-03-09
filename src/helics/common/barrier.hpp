/*

Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#ifndef _BARRIER_H_
#define _BARRIER_H_

#include <condition_variable>
#include <mutex>

namespace helics
{
class Barrier
{
  public:
    explicit Barrier (std::size_t iCount) : mThreshold (iCount), mCount (iCount), mGeneration (0) {}

    void Wait ()
    {
        std::unique_lock<std::mutex> lLock{mMutex};
        auto lGen = mGeneration;
        if (!--mCount)
        {
            mGeneration++;
            mCount = mThreshold;
            mCond.notify_all ();
        }
        else
        {
            mCond.wait (lLock, [this, lGen] { return lGen != mGeneration; });
        }
    }

  private:
    std::mutex mMutex;
    std::condition_variable mCond;
    std::size_t mThreshold;
    std::size_t mCount;
    std::size_t mGeneration;
};

} /* namespace helics */

#endif /* _BARRIER_H_ */

