/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../NetworkCore.hpp"

namespace helics
{
namespace tcp
{
class TcpComms;
class TcpCommsSS;
/** implementation for the core that uses tcp messages to communicate*/
using TcpCore = NetworkCore<TcpComms, interface_type::tcp>;


/** implementation for the core that uses tcp messages to communicate*/
class TcpCoreSS final : public NetworkCore<TcpCommsSS, interface_type::tcp>
{
  public:
    /** default constructor*/
    TcpCoreSS () noexcept;
    TcpCoreSS (const std::string &core_name);

    virtual void initializeFromArgs (int argc, const char *const *argv) override;

  private:
    std::vector<std::string> connections;  //!< defined connections 
    bool no_outgoing_connections = false; //!< disable outgoing connections if true;
    virtual bool brokerConnect () override;
};

}  // namespace tcp
}  // namespace helics

