/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../NetworkBroker.hpp"
#include "../core-types.hpp"
namespace helics
{
namespace tcp
{
class TcpComms;
class TcpCommsSS;
/** implementation for the core that uses TCP messages to communicate*/
using TcpBroker = NetworkBroker<TcpComms, interface_type::tcp, static_cast<int> (core_type::TCP)>;

/** single socket version of the TCP broker*/
class TcpBrokerSS final
    : public NetworkBroker<TcpCommsSS, interface_type::tcp, static_cast<int> (core_type::TCP_SS)>
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
    bool no_outgoing_connections = false;  //!< disable outgoing connections if true;
    std::vector<std::string> connections;  //!< defined connections These are connections that the comm section
                                           //!< reaches out to regardless of whether it is a broker/core/ or server
};

}  // namespace tcp
}  // namespace helics
