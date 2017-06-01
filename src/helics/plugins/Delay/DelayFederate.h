/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#pragma once
#ifndef _HELICS_DELAY_FEDERATE_API_
#define _HELICS_DELAY_FEDERATE_API_

#include "helics/application_api/Federate.h"
#include "helics/application_api/Message.h"
#include "helics/application_api/identifierTypes.h"
#include <functional>
#include <vector>
namespace helics
{
  class MessageFederateManager;
  class MessageFederate;
  class DelayFederate : public virtual Federate
  {
    public:
      DelayFederate(FederateInfo fi);
      /**constructor taking a file with the required information
      @param[in] file a file defining the federate information
      */
      DelayFederate(const std::strings &file);
  
    protected:
      DelayFederate();
  
    public:
      ~DelayFederate();
  }
}
