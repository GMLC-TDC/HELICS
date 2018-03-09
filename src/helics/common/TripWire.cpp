/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
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

