/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "CommsBroker.hpp"
#include "NetworkBrokerData.hpp"
#include "helics/core/CommonCore.hpp"

#include <memory>
#include <string>

namespace helics {
template<class COMMS, interface_type baseline = interface_type::ip>
class NetworkCore: public CommsBroker<COMMS, CommonCore> {
  public:
    /** default constructor*/
    NetworkCore() noexcept;
    explicit NetworkCore(const std::string& broker_name);

  public:
    virtual std::string generateLocalAddressString() const override;

  protected:
    virtual std::shared_ptr<helicsCLI11App> generateCLI() override;
    virtual bool brokerConnect() override;
    mutable std::mutex dataMutex;  //!< mutex protecting the configuration information
    NetworkBrokerData netInfo{baseline};  //!< structure containing the networking information
};

}  // namespace helics
