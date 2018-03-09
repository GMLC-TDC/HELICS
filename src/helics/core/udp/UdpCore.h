/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
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

