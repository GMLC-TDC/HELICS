/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
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
bool TripWireDetector::isTripped () const noexcept { return lineDetector->load (std::memory_order_acquire); }

TripWireTrigger::TripWireTrigger () : lineTrigger (TripWire::getline ()) {}

TripWireTrigger::~TripWireTrigger () { lineTrigger->store (true, std::memory_order_release); }
}  // namespace tripwire
