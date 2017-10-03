
/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_APISHARED_VALUE_FEDERATE_FUNCTIONS_H_
#define HELICS_APISHARED_VALUE_FEDERATE_FUNCTIONS_H_

#include "helics.h"

#ifdef __cplusplus
extern "C" {
#endif

HELICS_Export helics_value_federate helicsCreateValueFederate (const helics_federate_info_t fi);
HELICS_Export helics_value_federate helicsCreateValueFederateFromFile (const char *);

/* sub/pub registration */
#define HELICS_STRING_TYPE 0
#define HELICS_DOUBLE_TYPE 1
#define HELICS_INT_TYPE 2
#define HELICS_COMPLEX_TYPE 3
#define HELICS_VECTOR_TYPE 4
#define HELICS_RAW_TYPE 25

HELICS_Export helics_subscription helicsRegisterSubscription (helics_value_federate fed,
                                                              const char *name,
                                                              const char *type,
                                                              const char *units);
HELICS_Export helics_subscription helicsRegisterTypeSubscription (helics_value_federate fed,
                                                                  int type,
                                                                  const char *name,
                                                                  const char *units);

HELICS_Export helics_publication helicsRegisterPublication (helics_value_federate fed,
                                                            const char *name,
                                                            const char *type,
                                                            const char *units);
HELICS_Export helics_publication helicsRegisterTypePublication (helics_value_federate fed,
                                                                int type,
                                                                const char *name,
                                                                const char *units);

HELICS_Export helics_publication helicsRegisterGlobalPublication (helics_value_federate fed,
                                                                  const char *name,
                                                                  const char *type,
                                                                  const char *units);
HELICS_Export helics_publication helicsRegisterGlobalTypePublication (helics_value_federate fed,
                                                                      int type,
                                                                      const char *name,
                                                                      const char *units);

/* getting and publishing values */
HELICS_Export helicsStatus helicsPublish (helics_publication pubID, const char *data, int len);
HELICS_Export helicsStatus helicsPublishString (helics_publication pubID, const char *str);
HELICS_Export helicsStatus helicsPublishInteger (helics_publication pubID, int64_t val);
HELICS_Export helicsStatus helicsPublishDouble (helics_publication pubID, double val);
HELICS_Export helicsStatus helicsPublishComplex (helics_publication pubID, double real, double imag);
HELICS_Export helicsStatus helicsPublishVector (helics_publication pubID, const double data[], int len);

HELICS_Export int helicsGetValue (helics_subscription pubID, char *data, int maxlen);
HELICS_Export helicsStatus helicsGetString (helics_subscription subID, char *str, int maxlen);
HELICS_Export helicsStatus helicsGetInteger (helics_subscription subID, int64_t *val);
HELICS_Export helicsStatus helicsGetDouble (helics_subscription subID, double *val);
HELICS_Export helicsStatus helicsGetComplex (helics_subscription subID, double *real, double *imag);
HELICS_Export int helicsGetVector (helics_subscription subID, double data[], int maxlen);

HELICS_Export helicsStatus helicsSetDefaultValue (helics_subscription subID, const char *data, int len);
HELICS_Export helicsStatus helicsSetDefaultString (helics_subscription subID, const char *str);
HELICS_Export helicsStatus helicsSetDefaultInteger (helics_subscription subID, int64_t val);
HELICS_Export helicsStatus helicsSetDefaultDouble (helics_subscription subID, double val);
HELICS_Export helicsStatus helicsSetDefaultComplex (helics_subscription subID, double real, double imag);
HELICS_Export helicsStatus helicsSetDefaultVector (helics_subscription subID, const double *data, int len);

HELICS_Export helicsStatus helicsGetSubscriptionType (helics_subscription subID, char *str, int maxlen);

HELICS_Export helicsStatus helicsGetPublicationType (helics_publication pubID, char *str, int maxlen);

HELICS_Export helicsStatus helicsGetSubscriptionKey (helics_subscription subID, char *str, int maxlen);

HELICS_Export helicsStatus helicsGetPublicationKey (helics_publication pubID, char *str, int maxlen);

HELICS_Export helicsStatus helicsGetSubscriptionUnits (helics_subscription subID, char *str, int maxlen);

HELICS_Export helicsStatus helicsGetPublicationUnits (helics_publication pubID, char *str, int maxlen);

HELICS_Export void helicsClosePublication (helics_publication pubID);

HELICS_Export void helicsCloseSubscription (helics_subscription subID);
#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif /* HELICS_APISHARED_VALUE_FEDERATE_FUNCTIONS_H_*/