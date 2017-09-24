
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

	HELICS_Export helics_value_federate_id_t helicsCreateValueFederate(federate_info_t *fi);
	HELICS_Export helics_value_federate_id_t helicsCreateValueFederateFromFile(const char *);

	/* sub/pub registration */
	HELICS_Export helics_subscription_id_t helicsRegisterSubscription(helics_value_federate_id_t fedID, const char *name, const char *type, const char *units);
	HELICS_Export helics_subscription_id_t helicsRegisterIntegerSubscription(helics_value_federate_id_t fedID, const char *name, const char *units);
	HELICS_Export helics_subscription_id_t helicsRegisterDoubleSubscription(helics_value_federate_id_t fedID, const char *name, const char *units);
	HELICS_Export helics_subscription_id_t helicsRegisterStringSubscription(helics_value_federate_id_t fedID, const char *name, const char *units);
	HELICS_Export helics_subscription_id_t helicsRegisterComplexSubscription(helics_value_federate_id_t fedID, const char *name, const char *units);
	HELICS_Export helics_subscription_id_t helicsRegisterVectorSubscription(helics_value_federate_id_t fedID, const char *name, const char *units);

	HELICS_Export helics_publication_id_t  helicsRegisterPublication(helics_value_federate_id_t fedID, const char *name, const char *type, const char *units);
	HELICS_Export helics_publication_id_t  helicsRegisterIntegerPublication(helics_value_federate_id_t fedID, const char *name, const char *units);
	HELICS_Export helics_publication_id_t  helicsRegisterDoublePublication(helics_value_federate_id_t fedID, const char *name, const char *units);
	HELICS_Export helics_publication_id_t  helicsRegisterStringPublication(helics_value_federate_id_t fedID, const char *name, const char *units);
	HELICS_Export helics_publication_id_t  helicsRegisterComplexPublication(helics_value_federate_id_t fedID, const char *name, const char *units);
	HELICS_Export helics_publication_id_t  helicsRegisterVectorPublication(helics_value_federate_id_t fedID, const char *name, const char *units);

	HELICS_Export helics_publication_id_t  helicsRegisterGlobalPublication(helics_value_federate_id_t fedID, const char *name, const char *type, const char *units);
	HELICS_Export helics_publication_id_t  helicsRegisterGlobalIntegerPublication(helics_value_federate_id_t fedID, const char *name, const char *units);
	HELICS_Export helics_publication_id_t  helicsRegisterGlobalDoublePublication(helics_value_federate_id_t fedID, const char *name, const char *units);
	HELICS_Export helics_publication_id_t  helicsRegisterGlobalStringPublication(helics_value_federate_id_t fedID, const char *name, const char *units);
	HELICS_Export helics_publication_id_t  helicsRegisterGlobalComplexPublication(helics_value_federate_id_t fedID, const char *name, const char *units);
	HELICS_Export helics_publication_id_t  helicsRegisterGlobalVectorPublication(helics_value_federate_id_t fedID, const char *name, const char *units);

	/* getting and publishing values */
	HELICS_Export helicsStatus helicsPublish(helics_publication_id_t pubID, const char *data, uint64_t len);
	HELICS_Export helicsStatus helicsPublishString(helics_publication_id_t pubID, const char *str);
	HELICS_Export helicsStatus helicsPublishInteger( helics_publication_id_t pubID, int64_t val);
	HELICS_Export helicsStatus helicsPublishDouble( helics_publication_id_t pubID, double val);
	HELICS_Export helicsStatus helicsPublishComplex( helics_publication_id_t pubID, double real, double imag);
	HELICS_Export helicsStatus helicsPublishVector( helics_publication_id_t pubID, const double data[], uint64_t len);
	
	HELICS_Export uint64_t helicsGetValue( helics_subscription_id_t pubID, char *data, uint64_t maxlen);
	HELICS_Export helicsStatus helicsGetString( helics_subscription_id_t pubID, char *str, uint64_t maxlen);
	HELICS_Export helicsStatus helicsGetInteger( helics_subscription_id_t pubID, int64_t *val);
	HELICS_Export helicsStatus helicsGetDouble( helics_subscription_id_t pubID, double *val);
	HELICS_Export helicsStatus helicsGetComplex( helics_subscription_id_t pubID, double *real, double *imag);
	HELICS_Export helicsStatus helicsGetVector( helics_subscription_id_t pubID, double data[], uint64_t len);

	HELICS_Export uint64_t helicsSetDefaultValue( helics_subscription_id_t pubID, char *data, uint64_t maxlen);
	HELICS_Export helicsStatus helicsSetDefaultString( helics_subscription_id_t pubID, char *str, uint64_t maxlen);
	HELICS_Export helicsStatus helicsSetDefaultInteger( helics_subscription_id_t pubID, int64_t *val);
	HELICS_Export helicsStatus helicsSetDefaultDouble(helics_subscription_id_t pubID, double *val);
	HELICS_Export helicsStatus helicsSetDefaultComplex( helics_subscription_id_t pubID, double *real, double *imag);
	HELICS_Export helicsStatus helicsSetDefaultVector( helics_subscription_id_t pubID, double *data, uint64_t len);



#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif /* HELICS_APISHARED_VALUE_FEDERATE_FUNCTIONS_H_*/