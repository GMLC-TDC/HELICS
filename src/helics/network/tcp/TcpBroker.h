/*
Copyright (c) 2017-2025,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../../core/CoreTypes.hpp"
#include "../NetworkBroker.hpp"

#include <memory>
#include <string>
#include <vector>

namespace helics {
namespace tcp {
    class TcpComms;
    class TcpCommsSS;
    /** implementation for the core that uses TCP messages to communicate*/
    using TcpBroker = NetworkBroker<TcpComms,
                                    gmlc::networking::InterfaceTypes::TCP,
                                    static_cast<int>(CoreType::TCP)>;

    /** single socket version of the TCP broker*/
    class TcpBrokerSS final:
        public NetworkBroker<TcpCommsSS,
                             gmlc::networking::InterfaceTypes::TCP,
                             static_cast<int>(CoreType::TCP_SS)> {
      public:
        /** default constructor*/
        explicit TcpBrokerSS(bool rootBroker = false) noexcept;
        explicit TcpBrokerSS(std::string_view broker_name);

      protected:
        virtual std::shared_ptr<helicsCLI11App> generateCLI() override;

      private:
        virtual bool brokerConnect() override;
        bool no_outgoing_connections = false;  //!< disable outgoing connections if true;
        std::vector<std::string>
            connections;  //!< defined connections These are connections that the comm section
        //!< reaches out to regardless of whether it is a broker/core/ or server
    };

}  // namespace tcp
}  // namespace helics
