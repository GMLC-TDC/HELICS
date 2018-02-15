/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

#include "TripWire.hpp"
namespace tripwire
{
triplineType TripWire::getline ()
{
    static triplineType staticline = std::make_shared<std::atomic<bool>> (false);
    return staticline;
}

TripWireDetector::TripWireDetector () : lineDetector (TripWire::getline ()) {}
bool TripWireDetector::isTripped () const { return lineDetector->load (); }

TripWireTrigger::TripWireTrigger () : lineTrigger (TripWire::getline ()) {}

TripWireTrigger::~TripWireTrigger () { lineTrigger->store (true); }
}