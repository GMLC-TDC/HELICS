/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "../NetworkBroker.hpp"

#include <string>
namespace helics {
namespace zeromq {
    class ZmqComms;
    class ZmqCommsSS;

    /** implementation for the core that uses zmq messages to communicate*/
    class ZmqBroker final: public NetworkBroker<ZmqComms, interface_type::tcp, 1> {
      public:
        /** default constructor*/
        explicit ZmqBroker(bool rootbroker = false) noexcept;
        /** construct using a particular name*/
        explicit ZmqBroker(const std::string& brokerName);

      private:
        virtual bool brokerConnect() override;
    };

    class ZmqCommsSS;
    /** implementation for the core that uses zmq messages to communicate*/
    class ZmqBrokerSS final: public NetworkBroker<ZmqCommsSS, interface_type::tcp, 1> {
      public:
        /** default constructor*/
        explicit ZmqBrokerSS(bool rootbroker = false) noexcept;
        /** construct from with a core name*/
        explicit ZmqBrokerSS(const std::string& broker_name);

      private:
        virtual bool brokerConnect() override;
    };

}  // namespace zeromq
}  // namespace helics
