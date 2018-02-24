/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "CombinationFederate.hpp"

namespace helics
{
CombinationFederate::CombinationFederate () = default;
CombinationFederate::CombinationFederate (const FederateInfo &fi)
    : Federate (fi), ValueFederate (true), MessageFederate (true)
{
}
CombinationFederate::CombinationFederate (const std::shared_ptr<Core> &core, const FederateInfo &fi)
    : Federate (core, fi), ValueFederate (true), MessageFederate (true)
{
}

CombinationFederate::CombinationFederate (const std::string &jsonString)
    : Federate (loadFederateInfo (jsonString)), ValueFederate (true), MessageFederate (true)
{
    registerInterfaces (jsonString);
}

CombinationFederate::CombinationFederate (CombinationFederate &&) noexcept = default;
CombinationFederate::~CombinationFederate () = default;

void CombinationFederate::disconnect ()
{
    ValueFederate::disconnect ();
    MessageFederate::disconnect ();
}

CombinationFederate &CombinationFederate::operator= (CombinationFederate &&) noexcept = default;

void CombinationFederate::updateTime (Time newTime, Time oldTime)
{
    ValueFederate::updateTime (newTime, oldTime);
    MessageFederate::updateTime (newTime, oldTime);
}

void CombinationFederate::startupToInitializeStateTransition ()
{
    ValueFederate::startupToInitializeStateTransition ();
    MessageFederate::startupToInitializeStateTransition ();
}

void CombinationFederate::initializeToExecuteStateTransition ()
{
    ValueFederate::initializeToExecuteStateTransition ();
    MessageFederate::initializeToExecuteStateTransition ();
}

void CombinationFederate::registerInterfaces (const std::string &jsonString)
{
    ValueFederate::registerValueInterfaces (jsonString);
    MessageFederate::registerMessageInterfaces (jsonString);
    Federate::registerFilterInterfaces (jsonString);
}
}  // namespace helics