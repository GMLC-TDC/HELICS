/*
Copyright (c) 2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/helics.h"

#include <stdio.h>
int main()
{
    volatile HelicsFederateInfo fedinfo = helicsCreateFederateInfo();
    helicsFederateInfoFree(fedinfo);
    printf("%s\n", helicsGetVersion());
    helicsCloseLibrary();
    return (0);
}
