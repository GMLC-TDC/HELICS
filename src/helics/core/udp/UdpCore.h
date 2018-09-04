/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../CommonCore.hpp"
#include "../CommsBroker.hpp"
#include "../NetworkBrokerData.hpp"
namespace helics
{
namespace udp {
class UdpComms;
/** implementation for the core that uses udp messages to communicate*/
class UdpCore final : public CommsBroker<UdpComms, CommonCore>
{
public:
    /** default constructor*/
    UdpCore() noexcept;
    UdpCore(const std::string &core_name);
    virtual void initializeFromArgs(int argc, const char *const *argv) override;

public:
    virtual std::string generateLocalAddressString () const override;

private:
    NetworkBrokerData netInfo{
      NetworkBrokerData::interface_type::udp };  //!< structure containing the networking information
    virtual bool brokerConnect() override;
};
} // namespace udp
}  // namespace helics

