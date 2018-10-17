/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

/** @file
@brief The C-API function for valueFederates
*/
#ifndef HELICS_APISHARED_VALUE_FEDERATE_FUNCTIONS_H_
#define HELICS_APISHARED_VALUE_FEDERATE_FUNCTIONS_H_

#include "helics.h"

#ifdef __cplusplus
extern "C" {
#endif

/* sub/pub registration */

/** a sequence of characters*/
#define HELICS_DATA_TYPE_STRING 0
/** a double precision floating point number*/
#define HELICS_DATA_TYPE_DOUBLE 1
/** a 64 bit integer*/
#define HELICS_DATA_TYPE_INT 2
/** a pair of doubles representing a complex number*/
#define HELICS_DATA_TYPE_COMPLEX 3
/** an array of doubles*/
#define HELICS_DATA_TYPE_VECTOR 4
/** a named point consisting of a string and a double*/
#define HELICS_DATA_TYPE_NAMEDPOINT 6

/** a boolean data type*/
#define HELICS_DATA_TYPE_BOOLEAN 7
/** raw data type*/
#define HELICS_DATA_TYPE_RAW 25

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
HELICS_EXPORT helics_subscription helicsFederateRegisterSubscription (helics_federate fed,
                                                                      const char *key,
                                                                      const char *type,
                                                                      const char *units);

/** create a subscription of a specific known type
@details the subscription becomes part of the federate and is destroyed when the federate is freed so there are no separate free functions
for subscriptions and publications
@param fed the federate object in which to create a subscription
@param key the identifier matching a publication to get a subscription for
@param type a known type identifier  HELICS_DATA_TYPE_STRING, HELICS_DATA_TYPE_INT, HELICS_DATA_TYPE_DOUBLE,
HELICS_DATA_TYPE_COMPLEX, HELICS_DATA_TYPE_VECTOR, HELICS_DATA_TYPE_NAMEDPOINT, HELICS_DATA_TYPE_BOOLEAN,
HELICS_DATA_TYPE_RAW
@param units a string listing the units of the subscription maybe NULL
@return an object containing the subscription
*/
HELICS_EXPORT helics_subscription helicsFederateRegisterTypeSubscription (helics_federate fed,
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
HELICS_EXPORT helics_subscription helicsFederateRegisterOptionalSubscription (helics_federate fed,
                                                                              const char *key,
                                                                              const char *type,
                                                                              const char *units);

/** create a subscription of a specific known type that is specifically stated to be optional
@details the subscription becomes part of the federate and is destroyed when the federate is freed so there are no separate free functions
for subscriptions and publications optional implies that there may or may not be matching publication elsewhere in the federation
@param fed the federate object in which to create a subscription
@param key the identifier matching a publication to get a subscription for
@param type a known type identifier HELICS_DATA_TYPE_STRING, HELICS_DATA_TYPE_INT, HELICS_DATA_TYPE_DOUBLE, HELICS_DATA_TYPE_COMPLEX,
HELICS_DATA_TYPE_VECTOR, HELICS_DATA_TYPE_NAMEDPOINT, HELICS_DATA_TYPE_BOOLEAN HELICS_DATA_TYPE_RAW
@param units a string listing the units of the subscription maybe NULL
@return an object containing the subscription
*/
HELICS_EXPORT helics_subscription helicsFederateRegisterOptionalTypeSubscription (helics_federate fed,
                                                                                  const char *key,
                                                                                  int type,
                                                                                  const char *units);


/** get a subscription by its key typically already created via registerInterfaces file or something of that nature
@param fed the federate object in which to create a publication
@param key key can be a name, key, or a shortcut created through the interface creation process
@return a helics_subscription, which will be NULL if an invalid key is used
*/
HELICS_EXPORT helics_subscription helicsFederateGetSubscription (helics_federate fed, const char *key);

/** get a subscription by its index typically already created via registerInterfaces file or something of that nature
@param fed the federate object in which to create a publication
@param index the index of the publication to get
@return a helics_subscription, which will be NULL if an invalid index
*/
HELICS_EXPORT helics_subscription helicsFederateGetSubscriptionByIndex (helics_federate fed, int index);


/** register a publication with an arbitrary type
@details the publication becomes part of the federate and is destroyed when the federate is freed so there are no separate free functions
for subscriptions and publications
@param fed the federate object in which to create a publication
@param key the identifier for the publication the global publication key will be prepended with the federate name
@param type a string describing the expected type of the publication
@param units a string listing the units of the subscription maybe NULL
@return an object containing the publication
*/
HELICS_EXPORT helics_publication helicsFederateRegisterPublication (helics_federate fed,
                                                                    const char *key,
                                                                    const char *type,
                                                                    const char *units);

/** register a publication with a defined type
@details the publication becomes part of the federate and is destroyed when the federate is freed so there are no separate free functions
for subscriptions and publications
@param fed the federate object in which to create a publication
@param key the identifier for the publication
@param type a code identifying the type of the publication one of HELICS_DATA_TYPE_STRING, HELICS_DATA_TYPE_INT, HELICS_DATA_TYPE_DOUBLE, HELICS_DATA_TYPE_COMPLEX,
HELICS_DATA_TYPE_VECTOR, HELICS_DATA_TYPE_NAMEDPOINT, HELICS_DATA_TYPE_BOOLEAN HELICS_DATA_TYPE_RAW
@param units a string listing the units of the subscription maybe NULL
@return an object containing the publication
*/
HELICS_EXPORT helics_publication helicsFederateRegisterTypePublication (helics_federate fed, const char *key, int type, const char *units);

/** register a global named publication with an arbitrary type
@details the publication becomes part of the federate and is destroyed when the federate is freed so there are no separate free functions
for subscriptions and publications
@param fed the federate object in which to create a publication
@param key the identifier for the publication
@param type a string describing the expected type of the publication
@param units a string listing the units of the subscription maybe NULL
@return an object containing the publication
*/
HELICS_EXPORT helics_publication helicsFederateRegisterGlobalPublication (helics_federate fed,
                                                                          const char *key,
                                                                          const char *type,
                                                                          const char *units);

/** register a global publication with a defined type
@details the publication becomes part of the federate and is destroyed when the federate is freed so there are no separate free functions
for subscriptions and publications
@param fed the federate object in which to create a publication
@param key the identifier for the publication
@param type a code identifying the type of the publication one of HELICS_DATA_TYPE_STRING, HELICS_DATA_TYPE_INT, HELICS_DATA_TYPE_DOUBLE, HELICS_DATA_TYPE_COMPLEX,
HELICS_DATA_TYPE_VECTOR, HELICS_DATA_TYPE_NAMEDPOINT, HELICS_DATA_TYPE_BOOLEAN HELICS_DATA_TYPE_RAW
@param units a string listing the units of the subscription maybe NULL
@return an object containing the publication
*/
HELICS_EXPORT helics_publication helicsFederateRegisterGlobalTypePublication (helics_federate fed,
                                                                              const char *key,
                                                                              int type,
                                                                              const char *units);

/** get a subscription by its key typically already created via registerInterfaces file or something of that nature
@param fed the federate object in which to create a publication
@param key key can be a name, key, or a shortcut created through the interface creation process
@return a helics_subscription, which will be NULL if an invalid key is used
*/
HELICS_EXPORT helics_publication helicsFederateGetPublication (helics_federate fed, const char *key);

/** get a publication by its index typically already created via registerInterfaces file or something of that nature
@param fed the federate object in which to create a publication
@param index the index of the publication to get
@return a helics_publication
*/
HELICS_EXPORT helics_publication helicsFederateGetPublicationByIndex (helics_federate fed, int index);

  /**
* \defgroup publications Publication functions
@details functions for publishing data of various kinds
The data will get translated to the type specified when the publication was constructed automatically
regardless of the function used to publish the data
* @{
*/

/** publish raw data from a char * and length
@param pub the publication to publish for
@param data a pointer to the raw data
@param len the size in bytes of the data to publish
@return helics_ok if everything was OK
*/
HELICS_EXPORT helics_status helicsPublicationPublishRaw (helics_publication pub, const void *data, int inputDataLength);

/** publish a string
@param pub the publication to publish for
@param str a pointer to a NULL terminated string
@return helics_ok if everything was OK
*/
HELICS_EXPORT helics_status helicsPublicationPublishString (helics_publication pub, const char *str);

/** publish an integer value
@param pub the publication to publish for
@param val the numerical value to publish
@return helics_ok if everything was OK
*/
HELICS_EXPORT helics_status helicsPublicationPublishInteger (helics_publication pub, int64_t val);

/** publish a Boolean Value
@param pub the publication to publish for
@param val the boolean value to publish either helics_true or helics_false
@return helics_ok if everything was OK
*/
HELICS_EXPORT helics_status helicsPublicationPublishBoolean(helics_publication pub, helics_bool_t val);

/** publish a double floating point value
@param pub the publication to publish for
@param val the numerical value to publish
@return helics_ok if everything was OK
*/
HELICS_EXPORT helics_status helicsPublicationPublishDouble (helics_publication pub, double val);

/** publish a complex value (or pair of values)
@param pub the publication to publish for
@param real the real part of a complex number to publish
@param imag the imaginary part of a complex number to publish
@return helics_ok if everything was OK
*/
HELICS_EXPORT helics_status helicsPublicationPublishComplex (helics_publication pub, double real, double imag);

/** publish a vector of doubles
@param pub the publication to publish for
@param data a pointer to an array of double data
@param len the number of points to publish
@return helics_ok if everything was OK
*/
HELICS_EXPORT helics_status helicsPublicationPublishVector (helics_publication pub, const double *vectorInput, int vectorlength);

/** publish a named point
@param pub the publication to publish for
@param str a pointer a null terminated string
@param val a double val to publish
@return helics_ok if everything was OK
*/
HELICS_EXPORT helics_status helicsPublicationPublishNamedPoint(helics_publication pub, const char *str, double val);

/**@}*/

/**
* \defgroup getValue GetValue functions
@details data can be returned in number of formats,  for instance if data is published as a double it can be returned as a string
and vice versa,  not all translations make that much sense but they do work.
* @{
*/
/** get the size of the raw value for subscription
@returns the size of the raw data/string in bytes
*/
HELICS_EXPORT int helicsSubscriptionGetValueSize (helics_subscription sub);

/** get the raw data for the latest value of a subscription
@param sub the subscription to get the data for
@param[out] data the memory location of the data
@param maxlen the maximum size of information that data can hold
@param[out] actualLength  the actual length of data copied to data
@return a helics_status value, helics_ok if everything went fine
*/
HELICS_EXPORT helics_status helicsSubscriptionGetRawValue (helics_subscription sub, void *data, int maxlen, int *actualLength);

/** get the size of a value for subscription assuming return as a string
@returns the size of the string
*/
HELICS_EXPORT int helicsSubscriptionGetStringSize(helics_subscription sub);

/** get a string value from a subscription
@param sub the subscription to get the data for
@param[out] str storage for copying a null terminated string
@param maxlen the maximum size of information that str can hold
@param[out] the actual length of the string
@return a helics_status value, helics_ok if everything went fine
*/
HELICS_EXPORT helics_status helicsSubscriptionGetString (helics_subscription sub, char *outputString, int maxStringlen, int *actualLength);

/** get an integer value from a subscription
@param sub the subscription to get the data for
@param[out] val memory location to place the value
@return a helics_status value, helics_ok if everything went fine
*/
HELICS_EXPORT helics_status helicsSubscriptionGetInteger (helics_subscription sub, int64_t *val);

/** get a boolean value from a subscription
@param sub the subscription to get the data for
@param[out] val memory location to place the value
@return a helics_status value, helics_ok if everything went fine
*/
HELICS_EXPORT helics_status helicsSubscriptionGetBoolean(helics_subscription sub, helics_bool_t *val);

/** get a double value from a subscription
@param sub the subscription to get the data for
@param[out] val memory location to place the value
@return a helics_status value, helics_ok if everything went fine
*/
HELICS_EXPORT helics_status helicsSubscriptionGetDouble (helics_subscription sub, double *val);

/** get a pair of double forming a complex number from a subscriptions
@param sub the subscription to get the data for
@param[out] real memory location to place the real part of a value
@param[out] imag memory location to place the imaginary part of a value
@return a helics_status value, helics_ok if everything went fine
*/
HELICS_EXPORT helics_status helicsSubscriptionGetComplex (helics_subscription sub, double *real, double *imag);

/** get the size of a value for subscription assuming return as an array of doubles
@returns the number of double in a return vector
*/
HELICS_EXPORT int helicsSubscriptionGetVectorSize (helics_subscription sub);

/** get a vector from a subscription
@param sub the subscription to get the result for
@param[out] data the location to store the data
@param maxlen the maximum size of the vector
@param[out] actualSize pointer to variable to store the actual size
*/
HELICS_EXPORT helics_status helicsSubscriptionGetVector (helics_subscription sub, double data[], int maxlen, int *actualSize);


/** get a named point from a subscription
@param sub the subscription to get the result for
@param[out] str storage for copying a null terminated string
@param maxlen the maximum size of information that str can hold
@param[out] the actual length of the string
@param[out] val the double value for the named point
*/
HELICS_EXPORT helics_status helicsSubscriptionGetNamedPoint(helics_subscription sub, char *outputString, int maxStringlen, int *actualLength, double *val);

/**@}*/

/**
* \defgroup default_values Default Value functions
@details these functions set the default value for a subscription. That is the value returned if nothing was published from elsewhere
* @{
*/

/** set the default as a raw data array
@param sub the subscription to set the default for
@param data a pointer to the raw data to use for the default
@param len the size of the raw data
@return helics_ok if everything was OK
*/
HELICS_EXPORT helics_status helicsSubscriptionSetDefaultRaw (helics_subscription sub, const void *data, int inputDataLength);

/** set the default as a string
@param sub the subscription to set the default for
@param str a pointer to the default string
@return helics_ok if everything was OK
*/
HELICS_EXPORT helics_status helicsSubscriptionSetDefaultString (helics_subscription sub, const char *str);

/** set the default as an integer
@param sub the subscription to set the default for
@param val the default integer
@return helics_ok if everything was OK
*/
HELICS_EXPORT helics_status helicsSubscriptionSetDefaultInteger (helics_subscription sub, int64_t val);

/** set the default as a boolean
@param sub the subscription to set the default for
@param val the default boolean value
@return helics_ok if everything was OK
*/
HELICS_EXPORT helics_status helicsSubscriptionSetDefaultBoolean(helics_subscription sub, helics_bool_t val);

/** set the default as a double
@param sub the subscription to set the default for
@param val the default double value
@return helics_ok if everything was OK
*/
HELICS_EXPORT helics_status helicsSubscriptionSetDefaultDouble (helics_subscription sub, double val);

/** set the default as a complex number
@param sub the subscription to set the default for
@param real the default real value
@param imag the default imaginary value
@return helics_ok if everything was OK
*/
HELICS_EXPORT helics_status helicsSubscriptionSetDefaultComplex (helics_subscription sub, double real, double imag);

/** set the default as a vector of doubles
@param sub the subscription to set the default for
@param data a pointer to an array of double data
@param len the number of points to publish
@return helics_ok if everything was OK
*/
HELICS_EXPORT helics_status helicsSubscriptionSetDefaultVector (helics_subscription sub, const double *vectorInput, int vectorlength);


/** set the default as a named_point
@param sub the subscription to set the default for
@param str a pointer to a string representing the name
@param val a double value for the value of the named point
@return helics_ok if everything was OK
*/
HELICS_EXPORT helics_status helicsSubscriptionSetDefaultNamedPoint(helics_subscription sub, const char *str, double val);


/**@}*/

/**
 * \defgroup information retrieval
 * @{
 */

/** get the type of a subscription
@param sub the subscription to query
@param[out] outputString a pointer to a memory location to store the resulting string
@param maxlen the maximum size of string that str can store
@return a helics_status enumeration, helics_ok if everything worked*/
HELICS_EXPORT helics_status helicsSubscriptionGetType (helics_subscription sub, char *outputString, int maxlen);

/** get the type of a publication
@param pub the publication to query
@param[out] outputString a pointer to a memory location to store the resulting string
@param maxlen the maximum size of string that str can store
@return a helics_status enumeration, helics_ok if everything worked*/
HELICS_EXPORT helics_status helicsPublicationGetType (helics_publication pub, char *outputString, int maxlen);

/** get the key of a subscription
@param sub the subscription to query
@param[out] outputString a pointer to a memory location to store the resulting string
@param maxlen the maximum size of string that str can store
@return a helics_status enumeration, helics_ok if everything worked*/
HELICS_EXPORT helics_status helicsSubscriptionGetKey (helics_subscription sub, char *outputString, int maxlen);

/** get the key of a publication
@details this will be the global key used to identify the publication to the federation
@param pub the publication to query
@param[out] outputString a pointer to a memory location to store the resulting string
@param maxlen the maximum size of string that str can store
@return a helics_status enumeration, helics_ok if everything worked*/
HELICS_EXPORT helics_status helicsPublicationGetKey (helics_publication pub, char *outputString, int maxlen);

/** get the units of a subscription
@param sub the subscription to query
@param[out] outputString a pointer to a memory location to store the resulting string
@param maxlen the maximum size of string that str can store
@return a helics_status enumeration, helics_ok if everything worked*/
HELICS_EXPORT helics_status helicsSubscriptionGetUnits (helics_subscription sub, char *outputString, int maxlen);

/** get the units of a publication
@param pub the publication to query
@param[out] outputString a pointer to a memory location to store the resulting string
@param maxlen the maximum size of string that str can store
@return a helics_status enumeration, helics_ok if everything worked*/
HELICS_EXPORT helics_status helicsPublicationGetUnits (helics_publication pub, char *outputString, int maxlen);

/**@}*/

/** check if a particular subscription was updated
@return true if it has been updated since the last value retrieval*/
HELICS_EXPORT helics_bool_t helicsSubscriptionIsUpdated (helics_subscription sub);
/** get the last time a subscription was updated */
HELICS_EXPORT helics_time_t helicsSubscriptionLastUpdateTime (helics_subscription sub);
/** get the number of publications in a federate
@return (-1) if fed was not a valid federate otherwise returns the number of publications*/
HELICS_EXPORT int helicsFederateGetPublicationCount (helics_federate fed);

/** get the number of subscriptions in a federate
@return (-1) if fed was not a valid federate otherwise returns the number of subscriptions*/
HELICS_EXPORT int helicsFederateGetSubscriptionCount (helics_federate fed);

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif /* HELICS_APISHARED_VALUE_FEDERATE_FUNCTIONS_H_*/
