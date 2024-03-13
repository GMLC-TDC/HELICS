/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "EndpointInfo.hpp"
#include "InputInfo.hpp"
#include "PublicationInfo.hpp"
#include "basic_CoreTypes.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics {
struct TransInformation {
    GlobalHandle id;
    std::string key;
    std::string type;
    TransInformation() = default;
    TransInformation(GlobalHandle gid, std::string_view key_, std::string_view type_):
        id(gid), key(key_), type(type_)
    {
    }
};

/** data class defining the information about a translator*/
class TranslatorInfo {
  public:
    /** constructor from all fields*/
    TranslatorInfo(GlobalHandle handle,
                   std::string_view key_,
                   std::string_view endpointType,
                   std::string_view units):
        id(handle), key(key_), pub(handle, key_, "any", units), ipt(handle, key_, "any", units),
        ept(handle, key_, endpointType)
    {
    }
    const GlobalHandle id;  //!< id of the translator

    const std::string key;  //!< the identifier of the translator
    uint16_t flags = 0;  //!< flags for the translator
    // there is a 4 byte gap here
    std::shared_ptr<TranslatorOperator> tranOp;  //!< the callback operation of the translator

  private:
    PublicationInfo pub;  //!< translator publication interface
    InputInfo ipt;  //!< translator input interface
    EndpointInfo ept;  //!< translator endpoint interface

  public:
    PublicationInfo* getPubInfo() { return &pub; }
    InputInfo* getInputInfo() { return &ipt; }
    EndpointInfo* getEndpointInfo() { return &ept; }
};
}  // namespace helics
