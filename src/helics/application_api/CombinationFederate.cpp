/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "CombinationFederate.hpp"

#include <memory>
#include <string>

namespace helics {
CombinationFederate::CombinationFederate() = default;
CombinationFederate::CombinationFederate(std::string_view fedName, const FederateInfo& fedInfo):
    Federate(fedName, fedInfo), ValueFederate(true), MessageFederate(true)
{
}
CombinationFederate::CombinationFederate(std::string_view fedName,
                                         const std::shared_ptr<Core>& core,
                                         const FederateInfo& fedInfo):
    Federate(fedName, core, fedInfo), ValueFederate(true), MessageFederate(true)
{
}

CombinationFederate::CombinationFederate(std::string_view fedName,
                                         CoreApp& core,
                                         const FederateInfo& fedInfo):
    Federate(fedName, core, fedInfo), ValueFederate(true), MessageFederate(true)
{
}

CombinationFederate::CombinationFederate(const std::string& configString):
    Federate(std::string_view{}, loadFederateInfo(configString)), ValueFederate(true),
    MessageFederate(true)
{
}

CombinationFederate::CombinationFederate(std::string_view fedName, const std::string& configString):
    Federate(fedName, loadFederateInfo(configString)), ValueFederate(true), MessageFederate(true)
{
}

CombinationFederate::CombinationFederate(CombinationFederate&&) noexcept = default;
CombinationFederate::~CombinationFederate() = default;

void CombinationFederate::setFlagOption(int flagIndex, bool flagValue)
{
    ValueFederate::setFlagOption(flagIndex, flagValue);
}

void CombinationFederate::disconnect()
{
    ValueFederate::disconnect();
    MessageFederate::disconnect();
}

CombinationFederate& CombinationFederate::operator=(CombinationFederate&&) noexcept = default;

void CombinationFederate::updateTime(Time newTime, Time oldTime)
{
    ValueFederate::updateTime(newTime, oldTime);
    MessageFederate::updateTime(newTime, oldTime);
}

void CombinationFederate::startupToInitializeStateTransition()
{
    ValueFederate::startupToInitializeStateTransition();
    MessageFederate::startupToInitializeStateTransition();
}

void CombinationFederate::initializeToExecuteStateTransition(iteration_time result)
{
    ValueFederate::initializeToExecuteStateTransition(result);
    MessageFederate::initializeToExecuteStateTransition(result);
}

std::string CombinationFederate::localQuery(std::string_view queryStr) const
{
    std::string res = ValueFederate::localQuery(queryStr);
    if (res.empty()) {
        res = MessageFederate::localQuery(queryStr);
    }
    return res;
}

void CombinationFederate::registerInterfaces(const std::string& configString)
{
    ValueFederate::registerValueInterfaces(configString);
    MessageFederate::registerMessageInterfaces(configString);
    Federate::registerFilterInterfaces(configString);
}
}  // namespace helics
