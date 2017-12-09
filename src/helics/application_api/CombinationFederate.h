/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _COMBINATION_FEDERATE_H_
#define _COMBINATION_FEDERATE_H_
#pragma once

#include "MessageFederate.h"
#include "ValueFederate.h"
#include "helicsTypes.hpp"

namespace helics
{
/** class defining a federate that can use both the value and message interfaces */
class CombinationFederate : public ValueFederate, public MessageFederate
{
  public:
    /** default constructor*/
    CombinationFederate ();
    /**constructor taking a federate information structure and using the default core
    @param[in] fi  a federate information structure
    */
    CombinationFederate (const FederateInfo &fi);
    /**constructor taking a federate information structure and using the given core
    @param core a pointer to core object which the federate can join
    @param[in] fi  a federate information structure
    */
    CombinationFederate (std::shared_ptr<Core> core, const FederateInfo &fi);
    /**constructor taking a file with the required information
    @param[in] file a file defining the federate information
    */
    CombinationFederate (const std::string &jsonString);

    /** move construction*/
    CombinationFederate (CombinationFederate &&fed) noexcept;
    /** destructor*/
    virtual ~CombinationFederate ();
    /** move assignment*/
    CombinationFederate &operator= (CombinationFederate &&fed) noexcept;

    virtual void disconnect() override;
  protected:
    virtual void updateTime (Time newTime, Time oldTime) override;
    virtual void StartupToInitializeStateTransition () override;
    virtual void InitializeToExecuteStateTransition () override;

  public:
    virtual void registerInterfaces (const std::string &jsonString) override;
};
} //namespace helics
#endif
