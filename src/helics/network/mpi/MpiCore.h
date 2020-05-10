/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC. See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "../../core/CommonCore.hpp"
#include "../CommsBroker.hpp"

#include <memory>
#include <string>

namespace helics {
namespace mpi {
    class MpiComms;
    /** implementation for the core that uses mpi messages to communicate*/
    class MpiCore final: public CommsBroker<MpiComms, CommonCore> {
      public:
        /** default constructor*/
        MpiCore() noexcept;
        explicit MpiCore(const std::string& core_name);
        ~MpiCore();

      protected:
        virtual std::shared_ptr<helicsCLI11App> generateCLI() override;

      public:
        virtual std::string generateLocalAddressString() const override;

      private:
        mutable std::mutex dataMutex;  //!< mutex protecting the configuration information
        std::string brokerAddress;  //!< the mpi rank:tag of the broker
        int brokerRank{0};
        int brokerTag{0};
        virtual bool brokerConnect() override;
    };

}  // namespace mpi
}  // namespace helics
