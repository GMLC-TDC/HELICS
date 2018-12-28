/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once
#include "../NetworkBroker.hpp"
namespace helics {
namespace zeromq {
class ZmqComms;
class ZmqCommsSS;

/** implementation for the core that uses zmq messages to communicate*/
class ZmqBroker final : public NetworkBroker<ZmqComms, interface_type::tcp,1> {

public:
    /** default constructor*/
    explicit ZmqBroker(bool rootbroker=false) noexcept;
    /** construct from with a core name*/
    explicit ZmqBroker(const std::string &broker_name);
    static void displayHelp(bool localOnly);

private:
    virtual bool brokerConnect() override;

};

class ZmqCommsSS;
/** implementation for the core that uses zmq messages to communicate*/
class ZmqBrokerSS final : public NetworkBroker<ZmqCommsSS, interface_type::tcp,1> {

public:
    /** default constructor*/
    explicit ZmqBrokerSS(bool rootbroker=false) noexcept;
    /** construct from with a core name*/
    explicit ZmqBrokerSS(const std::string &broker_name);
    static void displayHelp(bool localOnly);

private:
    virtual bool brokerConnect() override;

};

} // namespace zeromq
} // namespace helics

