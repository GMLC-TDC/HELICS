
/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

/** @file
@brief functions related the value federates for the C api
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
@details the subscription becomes part of the federate and is destroyed when the federate is freed so there are no separate free functions
for subscriptions and publications
@param fed the federate object in which to create a subscription must have been create with helicsCreateValueFederate or
helicsCreateCombinationFederate
@param key the identifier matching a publication to get a subscription for
@param type a string describing the expected type of the publication may be NULL
@param units a string listing the units of the subscription maybe NULL
@return an object containing the subscription
*/
HELICS_Export helics_subscription helicsFederateRegisterSubscription (helics_federate fed,
                                                                      const char *key,
                                                                      const char *type,
                                                                      const char *units);

/** create a subscription of a specific known type
@details the subscription becomes part of the federate and is destroyed when the federate is freed so there are no separate free functions
for subscriptions and publications
@param fed the federate object in which to create a subscription
@param key the identifier matching a publication to get a subscription for
@param type a known type identifier  HELICS_STRING_TYPE, HELICS_INT_TYPE, HELICS_DOUBLE_TYPE, HELICS_COMPLEX_TYPE, HELICS_VECTOR_TYPE,
HELICS_RAW_TYPE
@param units a string listing the units of the subscription maybe NULL
@return an object containing the subscription
*/
HELICS_Export helics_subscription helicsFederateRegisterTypeSubscription (helics_federate fed,
                                                                          const char *key,
                                                                          int type,
                                                                          const char *units);

/** create a subscription that is specifically stated to be optional
@details the subscription becomes part of the federate and is destroyed when the federate is freed so there are no separate free functions
for subscriptions and publications
@details optional implies that there may or may not be matching publication elsewhere in the federation
@param fed the federate object in which to create a subscription
@param key the identifier matching a publication to get a subscription for
@param type a string describing the expected type of the publication may be NULL
@param units a string listing the units of the subscription maybe NULL
@return an object containing the subscription
*/
HELICS_Export helics_subscription helicsFederateRegisterOptionalSubscription (helics_federate fed,
                                                                              const char *key,
                                                                              const char *type,
                                                                              const char *units);

/** create a subscription of a specific known type that is specifically stated to be optional
@details the subscription becomes part of the federate and is destroyed when the federate is freed so there are no separate free functions
for subscriptions and publications optional implies that there may or may not be matching publication elsewhere in the federation
@param fed the federate object in which to create a subscription
@param key the identifier matching a publication to get a subscription for
@param type a known type identifier HELICS_STRING_TYPE, HELICS_INT_TYPE, HELICS_DOUBLE_TYPE, HELICS_COMPLEX_TYPE, HELICS_VECTOR_TYPE,
HELICS_RAW_TYPE
@param units a string listing the units of the subscription maybe NULL
@return an object containing the subscription
*/
HELICS_Export helics_subscription helicsFederateRegisterOptionalTypeSubscription (helics_federate fed,
                                                                                  const char *key,
                                                                                  int type,
                                                                                  const char *units);

HELICS_Export helics_publication helicsFederateRegisterPublication (helics_federate fed,
                                                                    const char *key,
                                                                    const char *type,
                                                                    const char *units);
HELICS_Export helics_publication helicsFederateRegisterTypePublication (helics_federate fed, const char *key, int type, const char *units);

HELICS_Export helics_publication helicsFederateRegisterGlobalPublication (helics_federate fed,
                                                                          const char *key,
                                                                          const char *type,
                                                                          const char *units);
HELICS_Export helics_publication helicsFederateRegisterGlobalTypePublication (helics_federate fed,
                                                                              const char *key,
                                                                              int type,
                                                                              const char *units);

/* getting and publishing values */
HELICS_Export helics_status helicsPublicationPublish (helics_publication pub, const char *data, int len);
HELICS_Export helics_status helicsPublicationPublishString (helics_publication pub, const char *str);
HELICS_Export helics_status helicsPublicationPublishInteger (helics_publication pub, int64_t val);
HELICS_Export helics_status helicsPublicationPublishDouble (helics_publication pub, double val);
HELICS_Export helics_status helicsPublicationPublishComplex (helics_publication pub, double real, double imag);
HELICS_Export helics_status helicsPublicationPublishVector (helics_publication pub, const double data[], int len);

HELICS_Export int helicsSubscriptionGetValueSize (helics_subscription sub);

HELICS_Export helics_status helicsSubscriptionGetValue (helics_subscription sub, char *data, int maxlen, int *actualLength);
HELICS_Export helics_status helicsSubscriptionGetString (helics_subscription sub, char *str, int maxlen);
HELICS_Export helics_status helicsSubscriptionGetInteger (helics_subscription sub, int64_t *val);
HELICS_Export helics_status helicsSubscriptionGetDouble (helics_subscription sub, double *val);
HELICS_Export helics_status helicsSubscriptionGetComplex (helics_subscription sub, double *real, double *imag);
HELICS_Export int helicsSubscriptionGetVectorSize (helics_subscription sub);

/** get a vector from a subscription
@param sub the subscription to get the result for
@param[out] data the location to store the data
@param maxlen the maximum size of the vector
@param[out] actualSize pointer to variable to store the actual size
*/
HELICS_Export helics_status helicsSubscriptionGetVector (helics_subscription sub, double data[], int maxlen, int *actualSize);

HELICS_Export helics_status helicsSubscriptionSetDefault (helics_subscription sub, const char *data, int len);
HELICS_Export helics_status helicsSubscriptionSetDefaultString (helics_subscription sub, const char *str);
HELICS_Export helics_status helicsSubscriptionSetDefaultInteger (helics_subscription sub, int64_t val);
HELICS_Export helics_status helicsSubscriptionSetDefaultDouble (helics_subscription sub, double val);
HELICS_Export helics_status helicsSubscriptionSetDefaultComplex (helics_subscription sub, double real, double imag);
HELICS_Export helics_status helicsSubscriptionSetDefaultVector (helics_subscription sub, const double *data, int len);

HELICS_Export helics_status helicsSubscriptionGetType (helics_subscription sub, char *str, int maxlen);

HELICS_Export helics_status helicsPublicationGetType (helics_publication pub, char *str, int maxlen);

HELICS_Export helics_status helicsSubscriptionGetKey (helics_subscription sub, char *str, int maxlen);

HELICS_Export helics_status helicsPublicationGetKey (helics_publication pub, char *str, int maxlen);

HELICS_Export helics_status helicsSubscriptionGetUnits (helics_subscription sub, char *str, int maxlen);

HELICS_Export helics_status helicsPublicationGetUnits (helics_publication pub, char *str, int maxlen);

HELICS_Export helics_bool_t helicsSubscriptionIsUpdated (helics_subscription sub);

HELICS_Export helics_time_t helicsSubscriptionLastUpdateTime (helics_subscription sub);

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif /* HELICS_APISHARED_VALUE_FEDERATE_FUNCTIONS_H_*/
