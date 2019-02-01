/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. 
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include "../CommonCore.hpp"
#include "../CommsBroker.hpp"
namespace helics
{
namespace mpi
{
class MpiComms;
/** implementation for the core that uses mpi messages to communicate*/
class MpiCore final : public CommsBroker<MpiComms, CommonCore>
{
  public:
    /** default constructor*/
    MpiCore () noexcept;
    explicit MpiCore (const std::string &core_name);
    ~MpiCore ();
    virtual void initializeFromArgs (int argc, const char *const *argv) override;

  public:
    virtual std::string generateLocalAddressString () const override;
  private:
    mutable std::mutex dataMutex;  //!< mutex protecting the configuration information
    std::string brokerAddress;  //!< the mpi rank:tag of the broker
    int brokerRank;
    int brokerTag;
    virtual bool brokerConnect () override;
};

} // namespace mpi
}  // namespace helics

