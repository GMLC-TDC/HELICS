/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once
#include <atomic>
#include <memory>

/** namespace for the global variable in tripwire*/
namespace tripwire
{
/** the actual tripwire type*/
using triplineType = std::shared_ptr<std::atomic<bool>>;

/** singleton class containing the actual trip line*/
class TripWire
{
  private:
    /** get the tripwire*/
    static triplineType getline ();

    friend class TripWireDetector;
    friend class TripWireTrigger;
};

/** class to check if a trip line was tripped*/
class TripWireDetector
{
  public:
    TripWireDetector ();
    /** check if the line was tripped*/
    bool isTripped () const noexcept;

  private:
    std::shared_ptr<const std::atomic<bool>> lineDetector;  //!< const pointer to the tripwire
};

/** class to trigger a tripline on destruction */
class TripWireTrigger
{
  public:
    TripWireTrigger ();
    ~TripWireTrigger ();
    TripWireTrigger (TripWireTrigger &&twt) = default;
    TripWireTrigger (const TripWireTrigger &twt) = delete;
    TripWireTrigger &operator= (TripWireTrigger &&twt) = default;
    TripWireTrigger &operator= (const TripWireTrigger &twt) = delete;

  private:
    triplineType lineTrigger;  //!< the tripwire
};
}  // namespace tripwire
