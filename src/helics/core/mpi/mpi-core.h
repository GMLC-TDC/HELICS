/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_MPI_CORE_
#define _HELICS_MPI_CORE_

#include "helics/config.h"

#if HELICS_HAVE_MPI

#include "helics/core/helics-time.h"
#include "helics/core/core-common.h"
#include "helics/core/core.h"

#include <cstdint>
#include <utility>

namespace helics {
/** class implementing an MPI communications for a core object*/
class MpiCore : public CommonCore {

public:
	/** default constructor*/
  MpiCore() noexcept;
  MpiCore(const std::string &core_name);
  /** destructor*/
  virtual ~MpiCore();

  virtual void initializeFromArgs (int argc, char *argv[]) override;

          void terminate();

protected:
	virtual void transmit(int route_id, const ActionMessage &cmd);
	virtual void addRoute(int route_id, const std::string &routeInfo);
private:
	virtual bool brokerConnect() override;
	virtual void brokerDisconnect() override;
	std::atomic<bool> initialized_{ false };  //!< atomic protecting local initialization
  
};

} // namespace helics
 
#endif /* HELICS_HAVE_MPI */

#endif /* _HELICS_MPI_CORE_ */
