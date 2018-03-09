/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
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
    ~UdpCore();
    virtual void initializeFromArgs(int argc, const char *const *argv) override;

public:
    virtual std::string getAddress() const override;

private:
    NetworkBrokerData netInfo{
      NetworkBrokerData::interface_type::udp };  //!< structure containing the networking information
    virtual bool brokerConnect() override;
};
} // namespace udp
}  // namespace helics

