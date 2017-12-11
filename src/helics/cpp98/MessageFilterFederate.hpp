/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_CPP98_MESSAGE_FILTER_FEDERATE_HPP_
#define HELICS_CPP98_MESSAGE_FILTER_FEDERATE_HPP_
#pragma once

#include "helics.hpp"
#include "MessageFederate.hpp"

namespace helics
{
class MessageFilterFederate : public virtual MessageFederate
{
  public:
    MessageFilterFederate (FederateInfo &fi)
    {
        fed = helicsCreateMessageFilterFederate (fi.getInfo());
    }

    MessageFilterFederate (const std::string &jsonString)
    {
        fed = helicsCreateMessageFilterFederateFromFile (jsonString.c_str());
    }

    virtual ~MessageFilterFederate ()
    {
        for (unsigned int i = 0; i < source_filters.size(); i++)
        {
            helicsFreeSourceFilter (source_filters[i]);
        }

        for (unsigned int i = 0; i < destination_filters.size(); i++)
        {
            helicsFreeDestinationFilter (destination_filters[i]);
        }
    }

    helics_source_filter registerSourceFilter (const std::string &filterName,
                                      const std::string &sourceEndpoint,
                                      const std::string &inputType = "",
                                      const std::string &outputType = "")
    {
        // C api might be missing an argument?
        helics_source_filter filter = helicsRegisterSourceFilter (fed, filterName.c_str(), inputType.c_str(), outputType.c_str());
        //helics_source_filter filter = helicsRegisterSourceFilter (fed, filterName.c_str(), sourceEndpoint.c_str(), inputType.c_str(), outputType.c_str());
        source_filters.push_back(filter);
        return filter;
    }

    helics_destination_filter registerDestinationFilter (const std::string &filterName,
                                           const std::string &destEndpoint,
                                           const std::string &inputType = "",
                                           const std::string &outputType = "")
    {
        // C api might be missing an argument?
        helics_destination_filter filter = helicsRegisterDestinationFilter (fed, filterName.c_str(), inputType.c_str(), outputType.c_str());
        //helics_destination_filter filter = helicsRegisterDestinationFilter (fed, filterName.c_str(), destEndpoint.c_str(), inputType.c_str(), outputType.c_str());
        destination_filters.push_back(filter);
        return filter;
    }

    bool hasMessageToFilter () const
    {
        // returns int, 1 = true, 0 = false
        return helicsFederateHasMessageToFilter (fed) > 0;
    }

    bool hasMessageToFilter (helics_source_filter filter) const
    {
        // returns int, 1 = true, 0 = false
        return helicsFilterHasMessage (filter) > 0;
    }
    
    message_t getMessageToFilter (helics_source_filter filter)
    {
        return helicsFilterGetMessage (filter);
    }

  private:
    std::vector<helics_source_filter> source_filters;
    std::vector<helics_destination_filter> destination_filters;
    int filterCount = 0;
};
} //namespace helics
#endif
