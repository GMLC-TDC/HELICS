/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "MessageFederate.hpp"
#include "ValueFederate.hpp"
#include "helicsTypes.hpp"

namespace helics {
/** class defining a federate that can use both the value and message interfaces */
class HELICS_CXX_EXPORT CombinationFederate: public ValueFederate, public MessageFederate {
  public:
    /** default constructor*/
    CombinationFederate();
    /**constructor taking a federate information structure and using the default core
    @param fedName the name of the federate, may be left empty to use a default or one found in fi
    @param fi  a federate information structure
    */
    explicit CombinationFederate(const std::string& fedName, const FederateInfo& fi);
    /**constructor taking a federate information structure and using the given core
    @param fedName the name of the federate, may be left empty to use a default or one found in fi
    @param core a pointer to core object which the federate can join
    @param fi  a federate information structure
    */
    CombinationFederate(
        const std::string& fedName,
        const std::shared_ptr<Core>& core,
        const FederateInfo& fi);

    /**constructor taking a federate information structure and using the given CoreApp
    @param fedName the name of the federate, may be left empty to use a default or one found in fi
    @param core a pointer to core object which the federate can join
    @param fi  a federate information structure
    */
    CombinationFederate(const std::string& fedName, CoreApp& core, const FederateInfo& fi);
    /**constructor taking a file with the required information
    @param configString a file or formatted String defining the federate information
    */
    explicit CombinationFederate(const std::string& configString);
    /**constructor taking a federate name and a file with the required information
    @param name the name of the federate, may be left empty to use a default or one found in configString
    @param configString a file or formatted String defining the federate information
    */
    CombinationFederate(const std::string& name, const std::string& configString);

    /** move construction*/
    CombinationFederate(CombinationFederate&& fed) noexcept;
    /** destructor*/
    virtual ~CombinationFederate();
    /** move assignment*/
    CombinationFederate& operator=(CombinationFederate&& fed) noexcept;
    /** delete the copy constructor*/
    CombinationFederate(const CombinationFederate& fed) = delete;
    /** copy assignment deleted*/
    CombinationFederate& operator=(const CombinationFederate& fed) = delete;
    virtual void disconnect() override;

  protected:
    virtual void updateTime(Time newTime, Time oldTime) override;
    virtual void startupToInitializeStateTransition() override;
    virtual void initializeToExecuteStateTransition() override;
    virtual std::string localQuery(const std::string& queryStr) const override;

  public:
    virtual void registerInterfaces(const std::string& configString) override;
};
} // namespace helics
