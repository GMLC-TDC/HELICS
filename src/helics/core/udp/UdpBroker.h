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
namespace udp
{
class UdpComms;

class UdpBroker final : public CommsBroker<UdpComms, CoreBroker>
{
  public:
    /** default constructor*/
    UdpBroker (bool rootBroker = false) noexcept;
    UdpBroker (const std::string &broker_name);

    void initializeFromArgs (int argc, const char *const *argv) override;

    /**destructor*/
    virtual ~UdpBroker ();

    virtual std::string getAddress () const override;
    static void displayHelp (bool local_only = false);

  private:
    virtual bool brokerConnect () override;
    NetworkBrokerData netInfo{
      NetworkBrokerData::interface_type::udp};  //!< structure containing the networking information
};

}  // namespace udp
}  // namespace helics

