/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/
#pragma once

#include "MessageFederate.hpp"
#include "ValueFederate.hpp"
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
    @param fi  a federate information structure
    */
    explicit CombinationFederate (const FederateInfo &fi);
    /**constructor taking a federate information structure and using the given core
    @param core a pointer to core object which the federate can join
    @param[in] fi  a federate information structure
    */
    CombinationFederate (const std::shared_ptr<Core> &core, const FederateInfo &fi);
    /**constructor taking a file with the required information
    @param[in] file a file defining the federate information
    */
    explicit CombinationFederate (const std::string &jsonString);

    /** move construction*/
    CombinationFederate (CombinationFederate &&fed) noexcept;
    /** destructor*/
    virtual ~CombinationFederate ();
    /** move assignment*/
    CombinationFederate &operator= (CombinationFederate &&fed) noexcept;

    virtual void disconnect () override;

  protected:
    virtual void updateTime (Time newTime, Time oldTime) override;
    virtual void startupToInitializeStateTransition () override;
    virtual void initializeToExecuteStateTransition () override;

  public:
    virtual void registerInterfaces (const std::string &jsonString) override;
};
}  // namespace helics
