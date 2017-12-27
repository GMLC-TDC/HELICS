
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

/* sub/pub registration */
#define HELICS_STRING_TYPE 0
#define HELICS_DOUBLE_TYPE 1
#define HELICS_INT_TYPE 2
#define HELICS_COMPLEX_TYPE 3
#define HELICS_VECTOR_TYPE 4
#define HELICS_RAW_TYPE 25


/** create a subscription
@param fed the federate object in which to create a subscription
@param key the identifier matching a publication to get a subscription for
@param type a string describing the expected type of the publication may be NULL
@param units a string listing the units of the subscription maybe NULL
@return an object containing the subscription
*/
HELICS_Export helics_subscription helicsRegisterSubscription (helics_value_federate fed,
                                                              const char *key,
                                                              const char *type,
                                                              const char *units);

/** create a subscription of a specific known type
@param fed the federate object in which to create a subscription
@param key the identifier matching a publication to get a subscription for
@param type a known type identifier  HELICS_STRING_TYPE, HELICS_INT_TYPE, HELICS_DOUBLE_TYPE, HELICS_COMPLEX_TYPE, HELICS_VECTOR_TYPE, HELICS_RAW_TYPE
@param units a string listing the units of the subscription maybe NULL
@return an object containing the subscription
*/
HELICS_Export helics_subscription helicsRegisterTypeSubscription (helics_value_federate fed, const char *key, int type, const char *units);

/** create a subscription that is specifically stated to be optional
@details optional implies that there may or may not be matching publication elsewhere in the federation
@param fed the federate object in which to create a subscription
@param key the identifier matching a publication to get a subscription for
@param type a string describing the expected type of the publication may be NULL
@param units a string listing the units of the subscription maybe NULL
@return an object containing the subscription
*/
HELICS_Export helics_subscription helicsRegisterOptionalSubscription(helics_value_federate fed,
    const char *key,
    const char *type,
    const char *units);

/** create a subscription of a specific known type that is specifically stated to be optional
@details optional implies that there may or may not be matching publication elsewhere in the federation
@param fed the federate object in which to create a subscription
@param key the identifier matching a publication to get a subscription for
@param type a known type identifier HELICS_STRING_TYPE, HELICS_INT_TYPE, HELICS_DOUBLE_TYPE, HELICS_COMPLEX_TYPE, HELICS_VECTOR_TYPE, HELICS_RAW_TYPE
@param units a string listing the units of the subscription maybe NULL
@return an object containing the subscription
*/
HELICS_Export helics_subscription helicsRegisterOptionalTypeSubscription(helics_value_federate fed, const char *key, int type, const char *units);


HELICS_Export helics_publication helicsRegisterPublication (helics_value_federate fed,
                                                            const char *key,
                                                            const char *type,
                                                            const char *units);
HELICS_Export helics_publication helicsRegisterTypePublication (helics_value_federate fed, const char *key, int type, const char *units);

HELICS_Export helics_publication helicsRegisterGlobalPublication (helics_value_federate fed,
                                                                  const char *key,
                                                                  const char *type,
                                                                  const char *units);
HELICS_Export helics_publication helicsRegisterGlobalTypePublication (helics_value_federate fed,
                                                                      const char *key,
                                                                      int type,
                                                                      const char *units);

/* getting and publishing values */
HELICS_Export helicsStatus helicsPublish (helics_publication pub, const char *data, int len);
HELICS_Export helicsStatus helicsPublishString (helics_publication pub, const char *str);
HELICS_Export helicsStatus helicsPublishInteger (helics_publication pub, int64_t val);
HELICS_Export helicsStatus helicsPublishDouble (helics_publication pub, double val);
HELICS_Export helicsStatus helicsPublishComplex (helics_publication pub, double real, double imag);
HELICS_Export helicsStatus helicsPublishVector (helics_publication pub, const double data[], int len);

HELICS_Export int helicsGetValueSize (helics_subscription sub);

HELICS_Export int helicsGetValue (helics_subscription sub, char *data, int maxlen);
HELICS_Export helicsStatus helicsGetString (helics_subscription sub, char *str, int maxlen);
HELICS_Export helicsStatus helicsGetInteger (helics_subscription sub, int64_t *val);
HELICS_Export helicsStatus helicsGetDouble (helics_subscription sub, double *val);
HELICS_Export helicsStatus helicsGetComplex (helics_subscription sub, double *real, double *imag);
HELICS_Export int helicsGetVectorSize (helics_subscription sub);
HELICS_Export int helicsGetVector (helics_subscription sub, double data[], int maxlen);

HELICS_Export helicsStatus helicsSetDefaultValue (helics_subscription sub, const char *data, int len);
HELICS_Export helicsStatus helicsSetDefaultString (helics_subscription sub, const char *str);
HELICS_Export helicsStatus helicsSetDefaultInteger (helics_subscription sub, int64_t val);
HELICS_Export helicsStatus helicsSetDefaultDouble (helics_subscription sub, double val);
HELICS_Export helicsStatus helicsSetDefaultComplex (helics_subscription sub, double real, double imag);
HELICS_Export helicsStatus helicsSetDefaultVector (helics_subscription sub, const double *data, int len);

HELICS_Export helicsStatus helicsGetSubscriptionType (helics_subscription sub, char *str, int maxlen);

HELICS_Export helicsStatus helicsGetPublicationType (helics_publication pub, char *str, int maxlen);

HELICS_Export helicsStatus helicsGetSubscriptionKey (helics_subscription sub, char *str, int maxlen);

HELICS_Export helicsStatus helicsGetPublicationKey (helics_publication pub, char *str, int maxlen);

HELICS_Export helicsStatus helicsGetSubscriptionUnits (helics_subscription sub, char *str, int maxlen);

HELICS_Export helicsStatus helicsGetPublicationUnits (helics_publication pub, char *str, int maxlen);

HELICS_Export int helicsIsValueUpdated (helics_subscription sub);

HELICS_Export helics_time_t helicsGetLastUpdateTime (helics_subscription sub);

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif /* HELICS_APISHARED_VALUE_FEDERATE_FUNCTIONS_H_*/
