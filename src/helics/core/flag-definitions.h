/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_FLAG_DEFINITIONS_
#define _HELICS_FLAG_DEFINITIONS_

/** @file
@details define statements for flag codes 
*/

#define OBSERVER_FLAG 0
#define UNINTERRUPTIBLE_FLAG 1
#define SOURCE_ONLY_FLAG 2
#define ONLY_TRANSMIT_ON_CHANGE_FLAG 3
#define ONLY_UPDATE_ON_CHANGE_FLAG 4
#define WAIT_FOR_CURRENT_TIME_UPDATE_FLAG 5

#define ROLLBACK_FLAG 8
#define FORWARD_COMPUTE_FLAG 9

#endif
