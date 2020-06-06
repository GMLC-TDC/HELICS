/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC. See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../../core/CoreBroker.hpp"
#include "../CommsBroker.hpp"

#include <memory>
#include <string>

namespace helics {
namespace mpi {
    class MpiComms;
    /** broker object implementing mpi communications for HELICS*/
    class MpiBroker final: public CommsBroker<MpiComms, CoreBroker> {
      public:
        /** default constructor*/
        explicit MpiBroker(bool rootBroker = false) noexcept;
        explicit MpiBroker(const std::string& broker_name);

      protected:
        virtual std::shared_ptr<helicsCLI11App> generateCLI() override;

      public:
        /**destructor*/
        virtual ~MpiBroker();

        virtual std::string generateLocalAddressString() const override;
        static void displayHelp(bool local_only = false);

      private:
        virtual bool brokerConnect() override;
        mutable std::mutex dataMutex;  //!< mutex protecting the configuration information
        std::string brokerAddress;  //!< the mpi rank:tag of the parent broker
        int brokerRank{0};
        int brokerTag{0};
    };
}  // namespace mpi
}  // namespace helics
