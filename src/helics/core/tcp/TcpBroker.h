/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../NetworkBroker.hpp"

namespace helics
{
namespace tcp
{
class TcpComms;
class TcpCommsSS;
/** implementation for the core that uses TCP messages to communicate*/
using TcpBroker = NetworkBroker<TcpComms, NetworkBrokerData::interface_type::tcp,6>;

class TcpBrokerSS final : public NetworkBroker<TcpCommsSS, NetworkBrokerData::interface_type::tcp, 11>
{
  public:
    /** default constructor*/ 
    explicit TcpBrokerSS (bool rootBroker = false) noexcept;
    explicit TcpBrokerSS (const std::string &broker_name);

    void initializeFromArgs (int argc, const char *const *argv) override;

  public:
    static void displayHelp (bool local_only = false);

  private:
    virtual bool brokerConnect () override;
    bool serverMode = true;
    std::vector<std::string> connections;  //!< defined connections 
};

}  // namespace tcp
}  // namespace helics

