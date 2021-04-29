/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "../NetworkCore.hpp"

#include <string>
namespace helics {
namespace zeromq {
    class ZmqComms;
    class ZmqCommsSS;

    /** implementation for the core that uses zmq messages to communicate*/
    class ZmqCore final: public NetworkCore<ZmqComms, interface_type::tcp> {
      public:
        /** default constructor*/
        ZmqCore() noexcept;
        /** construct from with a core name*/
        ZmqCore(const std::string& coreName);

      private:
        virtual bool brokerConnect() override;
    };

    /** implementation for the core that uses zmq messages to communicate*/
    class ZmqCoreSS final: public NetworkCore<ZmqCommsSS, interface_type::tcp> {
      public:
        /** default constructor*/
        ZmqCoreSS() noexcept;
        /** construct from with a core name*/
        ZmqCoreSS(const std::string& coreName);

      private:
        virtual bool brokerConnect() override;
    };

}  // namespace zeromq
}  // namespace helics
