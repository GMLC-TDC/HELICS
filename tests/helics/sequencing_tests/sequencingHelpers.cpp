/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "sequencingHelpers.hpp"

#include "helics/application_api/Federate.hpp"
#include "helics/core/CommonCore.hpp"
#include "helics/core/CoreBroker.hpp"
#include "helics/network/CommsBroker.hpp"
#include "helics/network/test/TestComms.h"

#include <memory>

namespace helics {
std::future<void> delayMessages(Federate* fed, int preDelayMessageCount, int msecDelay)
{
    auto tcore = std::dynamic_pointer_cast<
        helics::CommsBroker<helics::testcore::TestComms, helics::CommonCore>>(
        fed->getCorePointer());
    if (!tcore) {
        throw(std::invalid_argument(" given federate is not using test core"));
    }
    auto* tcomms = tcore->getCommsObjectPointer();
    tcomms->allowMessages(preDelayMessageCount);
    auto tcommAllow = std::async(std::launch::async, [tcomms, msecDelay]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(msecDelay));
        tcomms->allowMessages(-1);
    });
    return tcommAllow;
}

std::future<void> delayMessages(Broker* brk, int preDelayMessageCount, int msecDelay)
{
    auto tbrk =
        dynamic_cast<helics::CommsBroker<helics::testcore::TestComms, helics::CoreBroker>*>(brk);
    if (brk == nullptr) {
        throw(std::invalid_argument(" given broker is not using test core"));
    }
    auto* tcomms = tbrk->getCommsObjectPointer();
    tcomms->allowMessages(preDelayMessageCount);
    auto tcommAllow = std::async(std::launch::async, [tcomms, msecDelay]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(msecDelay));
        tcomms->allowMessages(-1);
    });
    return tcommAllow;
}
}  // namespace helics
