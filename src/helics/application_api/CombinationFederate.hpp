/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "MessageFederate.hpp"
#include "ValueFederate.hpp"
#include "helicsTypes.hpp"

#include <memory>
#include <string>

namespace helics {
/** class defining a federate that can use both the value and message interfaces */
class HELICS_CXX_EXPORT CombinationFederate: public ValueFederate, public MessageFederate {
  public:
    /** default constructor*/
    CombinationFederate();

    /**constructor taking a federate information structure and using the default core
    @param fedName the name of the federate, may be left empty to use a default or one found in
    fedInfo
    @param fedInfo  a federate information structure
    */
    explicit CombinationFederate(std::string_view fedName, const FederateInfo& fedInfo);

    /**constructor taking a federate information structure and using the given core
    @param fedName the name of the federate, may be left empty to use a default or one found in
    fedInfo
    @param core a pointer to core object which the federate can join
    @param fedInfo  a federate information structure
    */
    CombinationFederate(std::string_view fedName,
                        const std::shared_ptr<Core>& core,
                        const FederateInfo& fedInfo = FederateInfo{});

    /**constructor taking a federate information structure and using the given CoreApp
    @param fedName the name of the federate, may be left empty to use a default or one found in
    fedInfo
    @param core a CoreApp object representing the core to connect to
    @param fedInfo  a federate information structure
    */
    CombinationFederate(std::string_view fedName,
                        CoreApp& core,
                        const FederateInfo& fedInfo = FederateInfo{});

    /**constructor taking a federate name and a file with the required information
    @param fedName the name of the federate, can be empty to use the name from the configString
    @param configString can be either a JSON file a TOML file (with extension TOML) or a string
    containing JSON code or a string with command line arguments
    */
    CombinationFederate(std::string_view fedName, const std::string& configString);

    /**constructor taking a file with the required information
     @param configString can be either a JSON file a TOML file (with extension TOML) or a string
    containing JSON code or a string with command line arguments
    */
    explicit CombinationFederate(const std::string& configString);

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
    virtual void setFlagOption(int flagIndex, bool flagValue = true) override;
    virtual void disconnect() override;

  protected:
    virtual void updateTime(Time newTime, Time oldTime) override;
    virtual void startupToInitializeStateTransition() override;
    virtual void initializeToExecuteStateTransition(iteration_time result) override;
    virtual std::string localQuery(std::string_view queryStr) const override;

  public:
    virtual void registerInterfaces(const std::string& configString) override;
};
}  // namespace helics
