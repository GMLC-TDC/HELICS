/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
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
    MpiCore (const std::string &core_name);
    ~MpiCore ();
    virtual void initializeFromArgs (int argc, const char *const *argv) override;

  public:
    virtual std::string getAddress () const override;

  private:
    std::string brokerAddress;  //!< the mpi rank:tag of the broker
    int brokerRank;
    int brokerTag;
    virtual bool brokerConnect () override;
};

} // namespace mpi
}  // namespace helics

