/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../CommsBroker.hpp"
#include "../CoreBroker.hpp"

namespace helics
{
namespace mpi
{
class MpiComms;
/** broker object implementing mpi communications for HELICS*/
class MpiBroker final : public CommsBroker<MpiComms, CoreBroker>
{
  public:
    /** default constructor*/
    explicit MpiBroker (bool rootBroker = false) noexcept;
    explicit MpiBroker (const std::string &broker_name);

    virtual void initializeFromArgs (int argc, const char *const *argv) override;

    /**destructor*/
    virtual ~MpiBroker ();

    virtual std::string generateLocalAddressString () const override;
    static void displayHelp (bool local_only = false);

  private:
    virtual bool brokerConnect () override;
    mutable std::mutex dataMutex;  //!< mutex protecting the configuration information
    std::string brokerAddress;  //!< the mpi rank:tag of the parent broker
    int brokerRank;
    int brokerTag;
};
}  // namespace mpi
}  // namespace helics
