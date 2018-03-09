/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../CommsBroker.hpp"
#include "../CoreBroker.hpp"

namespace helics
{
namespace mpi
{
class MpiComms;

class MpiBroker final : public CommsBroker<MpiComms, CoreBroker>
{
  public:
    /** default constructor*/
    MpiBroker (bool rootBroker = false) noexcept;
    MpiBroker (const std::string &broker_name);

    void initializeFromArgs (int argc, const char *const *argv) override;

    /**destructor*/
    virtual ~MpiBroker ();

    virtual std::string getAddress () const override;
    static void displayHelp (bool local_only = false);

  private:
    virtual bool brokerConnect () override;
    std::string brokerAddress;  //!< the mpi rank:tag of the parent broker
    int brokerRank;
    int brokerTag;
};
} // namespace mpi
} // namespace helics

