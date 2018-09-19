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
class TcpCommsSS;

class TcpBroker final : public CommsBroker<TcpComms, CoreBroker>
{
  public:
    /** default constructor*/
    TcpBroker (bool rootBroker = false) noexcept;
    TcpBroker (const std::string &broker_name);

    void initializeFromArgs (int argc, const char *const *argv) override;

  public:
    virtual std::string generateLocalAddressString () const override;
    static void displayHelp (bool local_only = false);

  private:
    virtual bool brokerConnect () override;
    mutable std::mutex dataMutex;  //!< mutex protecting the configuration information
    NetworkBrokerData netInfo{
      NetworkBrokerData::interface_type::tcp};  //!< structure containing the networking information

};

class TcpBrokerSS final : public CommsBroker<TcpCommsSS, CoreBroker>
{
  public:
    /** default constructor*/ 
    TcpBrokerSS (bool rootBroker = false) noexcept;
    TcpBrokerSS (const std::string &broker_name);

    void initializeFromArgs (int argc, const char *const *argv) override;

  public:
    virtual std::string generateLocalAddressString () const override;
    static void displayHelp (bool local_only = false);

  private:
    virtual bool brokerConnect () override;
    mutable std::mutex dataMutex;  //!< mutex protecting the configuration information
    NetworkBrokerData netInfo{
      NetworkBrokerData::interface_type::tcp};  //!< structure containing the networking information
    bool serverMode = true;
    std::vector<std::string> connections;  //!< defined connections 
};

}  // namespace tcp
}  // namespace helics

