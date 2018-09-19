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
namespace tcp
{
class TcpComms;
class TcpCommsSS;
/** implementation for the core that uses tcp messages to communicate*/
class TcpCore final : public CommsBroker<TcpComms, CommonCore>
{
  public:
    /** default constructor*/
    TcpCore () noexcept;
    TcpCore (const std::string &core_name);

    virtual void initializeFromArgs (int argc, const char *const *argv) override;

  protected:
    virtual std::string generateLocalAddressString () const override;

  private:
    NetworkBrokerData netInfo{
      NetworkBrokerData::interface_type::tcp};  //!< structure containing the networking information
    virtual bool brokerConnect () override;
};

/** implementation for the core that uses tcp messages to communicate*/
class TcpCoreSS final : public CommsBroker<TcpCommsSS, CommonCore>
{
  public:
    /** default constructor*/
    TcpCoreSS () noexcept;
    TcpCoreSS (const std::string &core_name);

    virtual void initializeFromArgs (int argc, const char *const *argv) override;

  protected:
    virtual std::string generateLocalAddressString () const override;

  private:
    NetworkBrokerData netInfo{
      NetworkBrokerData::interface_type::tcp};  //!< structure containing the networking information
    bool serverMode = false;
    std::vector<std::string> connections;  //!< defined connections 
    virtual bool brokerConnect () override;
};

}  // namespace tcp
}  // namespace helics

