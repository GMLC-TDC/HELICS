/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.
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

