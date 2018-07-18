/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
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
    CombinationFederate::registerInterfaces (jsonString);
}

CombinationFederate::CombinationFederate (const std::string &name, const std::string &jsonString)
    : Federate (loadFederateInfo (name, jsonString)), ValueFederate (true), MessageFederate (true)
{
    CombinationFederate::registerInterfaces (jsonString);
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
