/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
*/

/*defines left in this code as it is used in the shared library*/
#ifndef _HELICS_FLAG_DEFINITIONS_
#define _HELICS_FLAG_DEFINITIONS_
#pragma once
/** @file
@details define statements for flag codes
*/

/** flag indicating that a federate is observe only*/
#define OBSERVER_FLAG 0
/** flag indicating that a federate can only return requested times*/
#define UNINTERRUPTIBLE_FLAG 1
/** flag indicating that a federate is a signal generator only*/
#define SOURCE_ONLY_FLAG 2
/** flag indicating a federate should only transmit values if they have changed(binary equivalence)*/
#define ONLY_TRANSMIT_ON_CHANGE_FLAG 3
/** flag indicating a federate should only trigger an update if a value has changed (binary equivalence)*/
#define ONLY_UPDATE_ON_CHANGE_FLAG 4
/** flag indicating a federate should only grant time if all other federates have already passed the requested time*/
#define WAIT_FOR_CURRENT_TIME_UPDATE_FLAG 5

/** flag indicating that a federate has rollback capability*/
#define ROLLBACK_FLAG 8
/** flag indicating that a federate performs forward computation and does internal rollback*/
#define FORWARD_COMPUTE_FLAG 9

/** used to delay a core from entering initialization mode even if it would otherwise be ready*/
#define DELAY_INIT_ENTRY 45
/** used to clear the DELAY_INIT_ENTRY flag in cores*/
#define ENABLE_INIT_ENTRY 47

#endif

