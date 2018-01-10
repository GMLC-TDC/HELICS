/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "CombinationFederate.h"

namespace helics
{
CombinationFederate::CombinationFederate () = default;
CombinationFederate::CombinationFederate (const FederateInfo &fi)
    : Federate (fi), ValueFederate (true), MessageFederate (true)
{
}
CombinationFederate::CombinationFederate (std::shared_ptr<Core> core, const FederateInfo &fi)
    : Federate (std::move (core), fi), ValueFederate (true), MessageFederate (true)
{
}

CombinationFederate::CombinationFederate (const std::string &jsonString)
    : Federate (jsonString), ValueFederate (true), MessageFederate (true)
{
    ValueFederate::registerInterfaces(jsonString);
    MessageFederate::registerInterfaces(jsonString);
}

CombinationFederate::CombinationFederate (CombinationFederate &&fed) noexcept = default;
CombinationFederate::~CombinationFederate () = default;

void CombinationFederate::disconnect ()
{
    ValueFederate::disconnect ();
    MessageFederate::disconnect ();
}

CombinationFederate &CombinationFederate::operator= (CombinationFederate &&fed) noexcept = default;

void CombinationFederate::updateTime (Time newTime, Time oldTime)
{
    ValueFederate::updateTime (newTime, oldTime);
    MessageFederate::updateTime (newTime, oldTime);
}

void CombinationFederate::StartupToInitializeStateTransition ()
{
    ValueFederate::StartupToInitializeStateTransition ();
    MessageFederate::StartupToInitializeStateTransition ();
}

void CombinationFederate::InitializeToExecuteStateTransition ()
{
    ValueFederate::InitializeToExecuteStateTransition ();
    MessageFederate::InitializeToExecuteStateTransition ();
}

void CombinationFederate::registerInterfaces (const std::string &jsonString)
{
    ValueFederate::registerInterfaces (jsonString);
    MessageFederate::registerInterfaces (jsonString);
}
}  // namespace helics