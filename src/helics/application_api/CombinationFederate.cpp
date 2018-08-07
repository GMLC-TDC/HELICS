/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "CombinationFederate.hpp"

namespace helics
{
CombinationFederate::CombinationFederate () = default;
CombinationFederate::CombinationFederate (const std::string &fedName, const FederateInfo &fi)
    : Federate (fedName,fi), ValueFederate (true), MessageFederate (true)
{
}
CombinationFederate::CombinationFederate (const std::string &fedName,
                                          const std::shared_ptr<Core> &core,
                                          const FederateInfo &fi)
    : Federate (fedName,core, fi), ValueFederate (true), MessageFederate (true)
{
}

CombinationFederate::CombinationFederate (const std::string &configString)
    : Federate (std::string(),loadFederateInfo (configString)), ValueFederate (true), MessageFederate (true)
{
    CombinationFederate::registerInterfaces (configString);
}

CombinationFederate::CombinationFederate (const std::string &fedName, const std::string &configString)
    : Federate (fedName,loadFederateInfo (configString)), ValueFederate (true), MessageFederate (true)
{
    CombinationFederate::registerInterfaces (configString);
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

void CombinationFederate::registerInterfaces (const std::string &configString)
{
    ValueFederate::registerValueInterfaces (configString);
    MessageFederate::registerMessageInterfaces (configString);
    Federate::registerFilterInterfaces (configString);
}
}  // namespace helics
