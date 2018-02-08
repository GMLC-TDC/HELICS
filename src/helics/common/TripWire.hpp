/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/
#pragma once
#include <atomic>
#include <memory>

/** namespace for the global variable in tripwire*/
namespace tripwire
{
/** singleton class containing the actual trip line*/

using triplineType = std::shared_ptr<std::atomic<bool>>;
class TripWire
{
private:
    static triplineType getline();

    friend class TripWireDetector;
    friend class TripWireTrigger;
};

/** class to check if a trip line was tripped*/
class TripWireDetector
{
public:
    TripWireDetector();
    bool isTripped() const;
private:
    std::shared_ptr<const std::atomic<bool>> lineDetector;
};

/** class to trigger a tripline on destruction */
class TripWireTrigger
{
public:
    TripWireTrigger();
    ~TripWireTrigger();
private:
    triplineType lineTrigger;
};
} //namespace tripwire
