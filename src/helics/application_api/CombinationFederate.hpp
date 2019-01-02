/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
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
    explicit CombinationFederate (const std::string &fedName, const FederateInfo &fi);
    /**constructor taking a federate information structure and using the given core
    @param core a pointer to core object which the federate can join
    @param[in] fi  a federate information structure
    */
    CombinationFederate (const std::string &fedName, const std::shared_ptr<Core> &core, const FederateInfo &fi);
    /**constructor taking a file with the required information
    @param[in] file a file defining the federate information
    */
    explicit CombinationFederate (const std::string &configString);
    /**constructor taking a federate name and a file with the required information
    @param[in] file a file defining the federate information
    */
    CombinationFederate (const std::string &name, const std::string &configString);

    /** move construction*/
    CombinationFederate (CombinationFederate &&fed) noexcept;
    /** destructor*/
    virtual ~CombinationFederate ();
    /** move assignment*/
    CombinationFederate &operator= (CombinationFederate &&fed) noexcept;
    /** delete the copy constructor*/
    CombinationFederate (const CombinationFederate &fed) = delete;
    /** copy assignment deleted*/
    CombinationFederate &operator= (const CombinationFederate &fed) = delete;
    virtual void disconnect () override;

  protected:
    virtual void updateTime (Time newTime, Time oldTime) override;
    virtual void startupToInitializeStateTransition () override;
    virtual void initializeToExecuteStateTransition () override;
    virtual std::string localQuery (const std::string &queryStr) const override;

  public:
    virtual void registerInterfaces (const std::string &jsonString) override;
};
}  // namespace helics
