/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "basic_CoreTypes.hpp"
#include "EndpointInfo.hpp"
#include "PublicationInfo.hpp"
#include "InputInfo.hpp"

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
    TransInformation(GlobalHandle gid, const std::string& key_, const std::string& type_):
        id(gid), key(key_), type(type_)
    {
    }
};

/** data class defining the information about a filter*/
class TranslatorInfo {
  public:
    /** constructor from all fields*/
    TranslatorInfo(GlobalHandle handle,
        const std::string & key_,
               const std::string& type_in_,
               const std::string& units):
        id(handle),
        key(key_),
        pub(handle, key_, "any", units), ipt(handle, key_, "any", units), ept(handle,key_,type_in_)
    {
    }
    const GlobalHandle id;  //!< id of the translator

    const std::string key;  //!< the identifier of the translator
    uint16_t flags = 0;  //!< flags for the translator
    // there is a 4 byte gap here
    std::shared_ptr<TranslatorOperator> tranOp;  //!< the callback operation of the filter

  private:
    PublicationInfo pub; //!< translator publication interface
    InputInfo ipt; //!< translator input interface
    EndpointInfo ept; //!< translator endpoint interface

  public:
    PublicationInfo* getPubInfo() { return &pub; }
    InputInfo* getInputInfo() { return &ipt; }
    EndpointInfo* getEndpointInfo() { return &ept; }

};
}  // namespace helics
