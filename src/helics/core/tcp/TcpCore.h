/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_TCP_CORE_
#define _HELICS_TCP_CORE_
#pragma once

#include "../CommonCore.hpp"
#include "../CommsBroker.hpp"
#include "../NetworkBrokerData.hpp"
namespace helics {
namespace tcp {
class TcpComms;
/** implementation for the core that uses tcp messages to communicate*/
class TcpCore final : public CommsBroker<TcpComms, CommonCore> {

public:
    /** default constructor*/
    TcpCore() noexcept;
    TcpCore(const std::string &core_name);
    ~TcpCore();
    virtual void initializeFromArgs(int argc, const char * const *argv) override;

public:
    virtual std::string getAddress() const override;
private:

    NetworkBrokerData netInfo{ NetworkBrokerData::interface_type::tcp }; //!< structure containing the networking information
    virtual bool brokerConnect() override;

};

} // namespace tcp
} // namespace helics
 
#endif /* _HELICS_TCP_CORE_ */
