/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "CallbackFederate.hpp"

#include <memory>
#include <string>

namespace helics {
CallbackFederate::CallbackFederate() = default;
CallbackFederate::CallbackFederate(std::string_view fedName, const FederateInfo& fi):
    CombinationFederate(fedName, fi)
{
}
CallbackFederate::CallbackFederate(std::string_view fedName,
                                         const std::shared_ptr<Core>& core,
                                         const FederateInfo& fi):
    CombinationFederate(fedName, core, fi)
{
}

CallbackFederate::CallbackFederate(std::string_view fedName,
                                         CoreApp& core,
                                         const FederateInfo& fi):
    CombinationFederate(fedName, core, fi)
{
}

CallbackFederate::CallbackFederate(const std::string& configString):
    CombinationFederate(std::string_view{}, loadFederateInfo(configString))
{

}

CallbackFederate::CallbackFederate(std::string_view fedName, const std::string& configString):
    CombinationFederate(fedName, loadFederateInfo(configString))
{
}

CallbackFederate::CallbackFederate(CallbackFederate&&) noexcept = default;
CallbackFederate::~CallbackFederate() = default;

CallbackFederate& CallbackFederate::operator=(CallbackFederate&&) noexcept = default;


}  // namespace helics
