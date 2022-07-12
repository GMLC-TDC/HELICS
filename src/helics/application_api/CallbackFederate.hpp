/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "CombinationFederate.hpp"

#include <memory>
#include <string>

namespace helics {
/** class defining a federate that can use both the value and message interfaces */
class HELICS_CXX_EXPORT CallbackFederate: public CombinationFederate {
  public:
    /** default constructor*/
    CallbackFederate();

    /**constructor taking a federate information structure and using the default core
    @param fedName the name of the federate, may be left empty to use a default or one found in fi
    @param fi  a federate information structure
    */
    explicit CallbackFederate(std::string_view fedName, const FederateInfo& fi);

    /**constructor taking a federate information structure and using the given core
    @param fedName the name of the federate, may be left empty to use a default or one found in fi
    @param core a pointer to core object which the federate can join
    @param fi  a federate information structure
    */
    CallbackFederate(std::string_view fedName,
                        const std::shared_ptr<Core>& core,
                        const FederateInfo& fi = FederateInfo{});

    /**constructor taking a federate information structure and using the given CoreApp
    @param fedName the name of the federate, may be left empty to use a default or one found in fi
    @param core a CoreApp object representing the core to connect to
    @param fi  a federate information structure
    */
    CallbackFederate(std::string_view fedName,
                        CoreApp& core,
                        const FederateInfo& fi = FederateInfo{});

    /**constructor taking a federate name and a file with the required information
    @param fedName the name of the federate, can be empty to use the name from the configString
    @param configString can be either a JSON file a TOML file (with extension TOML) or a string
    containing JSON code or a string with command line arguments
    */
    CallbackFederate(std::string_view fedName, const std::string& configString);

    /**constructor taking a file with the required information
     @param configString can be either a JSON file a TOML file (with extension TOML) or a string
    containing JSON code or a string with command line arguments
    */
    explicit CallbackFederate(const std::string& configString);

    /** move construction*/
    CallbackFederate(CallbackFederate&& fed) noexcept;
    /** destructor*/
    virtual ~CallbackFederate();
    /** move assignment*/
    CallbackFederate& operator=(CallbackFederate&& fed) noexcept;
    /** delete the copy constructor*/
    CallbackFederate(const CallbackFederate& fed) = delete;
    /** copy assignment deleted*/
    CallbackFederate& operator=(const CallbackFederate& fed) = delete;
    
};
}  // namespace helics
