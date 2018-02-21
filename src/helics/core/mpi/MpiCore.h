/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_MPI_CORE_
#define _HELICS_MPI_CORE_
#pragma once

#include "../CommonCore.hpp"
#include "../CommsBroker.hpp"
namespace helics {

class MpiComms;
/** implementation for the core that uses mpi messages to communicate*/
class MpiCore final: public CommsBroker<MpiComms,CommonCore> {

public:
	/** default constructor*/
  MpiCore() noexcept;
  MpiCore(const std::string &core_name);
  ~MpiCore();
  virtual void initializeFromArgs (int argc, const char * const *argv) override;
         
public:
	virtual std::string getAddress() const override;
private:
    std::string brokerAddress; //!< the mpi rank:tag of the broker
	virtual bool brokerConnect() override;
 
};


} // namespace helics
 
#endif /* _HELICS_MPI_CORE_ */
