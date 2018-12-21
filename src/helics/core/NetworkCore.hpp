/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "CommsBroker.hpp"
#include "NetworkBrokerData.hpp"
#include "CommsBroker.hpp"
#include "CommonCore.hpp"

namespace helics
{
template <class COMMS, interface_type baseline = interface_type::ip>
class NetworkCore :public CommsBroker<COMMS, CommonCore>
{
public:
    /** default constructor*/
    NetworkCore() noexcept;
    explicit NetworkCore(const std::string &broker_name);

    void initializeFromArgs(int argc, const char *const *argv) override;

public:
    virtual std::string generateLocalAddressString() const override;

protected:
    virtual bool brokerConnect() override;
    mutable std::mutex dataMutex;  //!< mutex protecting the configuration information
    NetworkBrokerData netInfo{
        baseline };  //!< structure containing the networking information
};

} //namespace helics