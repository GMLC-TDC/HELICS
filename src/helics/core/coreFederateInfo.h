/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_CORE_FEDERATE_INFO_
#define HELICS_CORE_FEDERATE_INFO_
#pragma once

#include "helics-time.h"


namespace helics
{
    /** class defining some required information about the federate*/
    class CoreFederateInfo
    {
    public:
        Time timeDelta = timeEpsilon;  // the minimum time advance allowed by the federate
                                       // federate
        Time lookAhead = timeZero;  //!< the lookahead value, the window of time between the time request return and the availability of values
        Time impactWindow = timeZero;  //!< the time it takes values to propagate to the Federate
        Time period = timeZero; //!< a period value,  all granted times must be on this period
        Time offset = timeZero;  //!< offset to the time period
        int logLevel;	//!< the logging level above which not to log to file
        bool observer = false;  //!< flag indicating that the federate is an observer
        bool uninterruptible =
            false;  //!< flag indicating that the federate should never return a time other than requested
        bool source_only = false;   //!< flag indicating that the federate does not receive or do anything with received information.  											  
        bool only_transmit_on_change = false; //!< flag indicating that values should only updated if the number has actually changes
        bool only_update_on_change = false;  //!< flag indicating values should be discarded if they are not changed from previous values
        bool wait_for_current_time_updates = false; //!< flag indicating that the federate should only grant when no more messages can be received at the current time
        int16_t max_iterations = 3;	//!< the maximum number of iterations allowed for the federate

    };

} //namespace helics
#endif //HELICS_CORE_FEDERATE_INFO_
