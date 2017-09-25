
/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_APISHARED_VALUE_FEDERATE_FUNCTIONS_H_
#define HELICS_APISHARED_VALUE_FEDERATE_FUNCTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "helics.h"

	HELICS_Export helics_value_federate helicsCreateValueFederate(federate_info_t *fi);
	HELICS_Export helics_value_federate helicsCreateValueFederateFromFile(const char *);

	/* sub/pub registration */
	HELICS_Export helics_subscription helicsRegisterSubscription(helics_value_federate fedID, const char *name, const char *type, const char *units);
	HELICS_Export helics_subscription helicsRegisterIntegerSubscription(helics_value_federate fedID, const char *name, const char *units);
	HELICS_Export helics_subscription helicsRegisterDoubleSubscription(helics_value_federate fedID, const char *name, const char *units);
	HELICS_Export helics_subscription helicsRegisterStringSubscription(helics_value_federate fedID, const char *name, const char *units);
	HELICS_Export helics_subscription helicsRegisterComplexSubscription(helics_value_federate fedID, const char *name, const char *units);
	HELICS_Export helics_subscription helicsRegisterVectorSubscription(helics_value_federate fedID, const char *name, const char *units);

	HELICS_Export helics_publication  helicsRegisterPublication(helics_value_federate fedID, const char *name, const char *type, const char *units);
	HELICS_Export helics_publication  helicsRegisterIntegerPublication(helics_value_federate fedID, const char *name, const char *units);
	HELICS_Export helics_publication  helicsRegisterDoublePublication(helics_value_federate fedID, const char *name, const char *units);
	HELICS_Export helics_publication  helicsRegisterStringPublication(helics_value_federate fedID, const char *name, const char *units);
	HELICS_Export helics_publication  helicsRegisterComplexPublication(helics_value_federate fedID, const char *name, const char *units);
	HELICS_Export helics_publication  helicsRegisterVectorPublication(helics_value_federate fedID, const char *name, const char *units);

	HELICS_Export helics_publication  helicsRegisterGlobalPublication(helics_value_federate fedID, const char *name, const char *type, const char *units);
	HELICS_Export helics_publication  helicsRegisterGlobalIntegerPublication(helics_value_federate fedID, const char *name, const char *units);
	HELICS_Export helics_publication  helicsRegisterGlobalDoublePublication(helics_value_federate fedID, const char *name, const char *units);
	HELICS_Export helics_publication  helicsRegisterGlobalStringPublication(helics_value_federate fedID, const char *name, const char *units);
	HELICS_Export helics_publication  helicsRegisterGlobalComplexPublication(helics_value_federate fedID, const char *name, const char *units);
	HELICS_Export helics_publication  helicsRegisterGlobalVectorPublication(helics_value_federate fedID, const char *name, const char *units);

	/* getting and publishing values */
	HELICS_Export helicsStatus helicsPublish(helics_publication pubID, const char *data, uint64_t len);
	HELICS_Export helicsStatus helicsPublishString(helics_publication pubID, const char *str);
	HELICS_Export helicsStatus helicsPublishInteger( helics_publication pubID, int64_t val);
	HELICS_Export helicsStatus helicsPublishDouble( helics_publication pubID, double val);
	HELICS_Export helicsStatus helicsPublishComplex( helics_publication pubID, double real, double imag);
	HELICS_Export helicsStatus helicsPublishVector( helics_publication pubID, const double data[], uint64_t len);
	
	HELICS_Export uint64_t helicsGetValue( helics_subscription pubID, char *data, uint64_t maxlen);
	HELICS_Export helicsStatus helicsGetString( helics_subscription pubID, char *str, uint64_t maxlen);
	HELICS_Export helicsStatus helicsGetInteger( helics_subscription pubID, int64_t *val);
	HELICS_Export helicsStatus helicsGetDouble( helics_subscription pubID, double *val);
	HELICS_Export helicsStatus helicsGetComplex( helics_subscription pubID, double *real, double *imag);
	HELICS_Export helicsStatus helicsGetVector( helics_subscription pubID, double data[], uint64_t len);

	HELICS_Export uint64_t helicsSetDefaultValue( helics_subscription pubID, char *data, uint64_t maxlen);
	HELICS_Export helicsStatus helicsSetDefaultString( helics_subscription pubID, char *str, uint64_t maxlen);
	HELICS_Export helicsStatus helicsSetDefaultInteger( helics_subscription pubID, int64_t *val);
	HELICS_Export helicsStatus helicsSetDefaultDouble(helics_subscription pubID, double *val);
	HELICS_Export helicsStatus helicsSetDefaultComplex( helics_subscription pubID, double *real, double *imag);
	HELICS_Export helicsStatus helicsSetDefaultVector( helics_subscription pubID, double *data, uint64_t len);



#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif /* HELICS_APISHARED_VALUE_FEDERATE_FUNCTIONS_H_*/