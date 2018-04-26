/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once

#include "../CommsBroker.hpp"
#include "../CoreBroker.hpp"
#include "../NetworkBrokerData.hpp"

namespace helics
{
namespace tcp
{
class TcpComms;

class TcpBroker final : public CommsBroker<TcpComms, CoreBroker>
{
  public:
    /** default constructor*/
    TcpBroker (bool rootBroker = false) noexcept;
    TcpBroker (const std::string &broker_name);

    void initializeFromArgs (int argc, const char *const *argv) override;

    virtual std::string getAddress () const override;
    static void displayHelp (bool local_only = false);

  private:
    virtual bool brokerConnect () override;

    NetworkBrokerData netInfo{
      NetworkBrokerData::interface_type::tcp};  //!< structure containing the networking information
};
}  // namespace tcp
}  // namespace helics

