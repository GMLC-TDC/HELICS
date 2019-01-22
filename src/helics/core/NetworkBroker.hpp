/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "CommsBroker.hpp"
#include "NetworkBrokerData.hpp"
#include "CommsBroker.hpp"
#include "CoreBroker.hpp"

namespace helics
{
template <class COMMS, interface_type baseline,int tcode=0>
class NetworkBroker :public CommsBroker<COMMS, CoreBroker>
{
public:
    /** default constructor*/
    explicit NetworkBroker(bool rootBroker = false) noexcept;
    explicit NetworkBroker(const std::string &broker_name);

    void initializeFromArgs(int argc, const char *const *argv) override;

public:
    virtual std::string generateLocalAddressString() const override;
    static void displayHelp(bool local_only = false);

protected:
    virtual bool brokerConnect() override;
    mutable std::mutex dataMutex;  //!< mutex protecting the configuration information
    NetworkBrokerData netInfo{
        baseline };  //!< structure containing the networking information
};

} //namespace helics
