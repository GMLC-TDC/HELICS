/*
Copyright © 2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/chelics.h"

int main ()
{
    volatile helics_federate_info fedinfo = helicsCreateFederateInfo();
	helicsFederateInfoFree(fedinfo);
    helicsCloseLibrary ();
    return (0);
}

