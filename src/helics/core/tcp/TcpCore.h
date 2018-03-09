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
/** implementation for the core that uses tcp messages to communicate*/
class TcpCore final : public CommsBroker<TcpComms, CommonCore>
{
  public:
    /** default constructor*/
    TcpCore () noexcept;
    TcpCore (const std::string &core_name);
    ~TcpCore ();
    virtual void initializeFromArgs (int argc, const char *const *argv) override;

  public:
    virtual std::string getAddress () const override;

  private:
    NetworkBrokerData netInfo{
      NetworkBrokerData::interface_type::tcp};  //!< structure containing the networking information
    virtual bool brokerConnect () override;
};

}  // namespace tcp
}  // namespace helics

