/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_ZEROMQ_CORE_
#define _HELICS_ZEROMQ_CORE_
#pragma once

#include "helics/core/core-common.h"



namespace helics {


class ZeroMQCore : public CommonCore {

public:

  ZeroMQCore() {};
  virtual ~ZeroMQCore();
  virtual void initialize (const std::string &initializationString) override;
         
  virtual void transmit(int route_id, ActionMessage &cmd) override;
  virtual void addRoute(int route_id, const std::string &routeInfo) override;
private:
  
 
};


} // namespace helics
 
#endif /* _HELICS_ZEROMQ_CORE_ */
