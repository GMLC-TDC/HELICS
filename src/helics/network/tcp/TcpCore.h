/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../NetworkCore.hpp"

#include <memory>
#include <string>
#include <vector>

namespace helics {
namespace tcp {
    class TcpComms;
    class TcpCommsSS;
    /** implementation for the core that uses tcp messages to communicate*/
    using TcpCore = NetworkCore<TcpComms, interface_type::tcp>;

    /** implementation for the core that uses tcp messages to communicate*/
    class TcpCoreSS final: public NetworkCore<TcpCommsSS, interface_type::tcp> {
      public:
        /** default constructor*/
        TcpCoreSS() noexcept;
        TcpCoreSS(const std::string& coreName);

      protected:
        virtual std::shared_ptr<helicsCLI11App> generateCLI() override;

      private:
        std::vector<std::string> connections;  //!< defined connections
        bool no_outgoing_connections = false;  //!< disable outgoing connections if true;
        virtual bool brokerConnect() override;
    };

}  // namespace tcp
}  // namespace helics
