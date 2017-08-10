/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_MPI_CORE_
#define _HELICS_MPI_CORE_

#include "helics/config.h"

#if HELICS_HAVE_MPI

#include "helics/core/helics-time.h"
#include "helics/core/core-common.h"
#include "helics/core/core.h"

#include <cstdint>
#include <utility>

namespace helics {

class MpiCore : public CommonCore {

public:

  MpiCore();
  virtual ~MpiCore();
  virtual void initializeFromArgs (int argc, char *argv[]) override;

          void terminate();

protected:
	virtual void transmit(int route_id, const ActionMessage &cmd);
	virtual void addRoute(int route_id, const std::string &routeInfo);

  /*
  virtual bool isInitialized () override;
  virtual void error (federate_id_t federateID, int errorCode) override;
  virtual void finalize (federate_id_t federateID) override;
  virtual void enterInitializingState (federate_id_t federateID) override;
  virtual bool enterExecutingState(federate_id_t federateID, bool iterationCompleted = true) override;
  virtual federate_id_t registerFederate (const char *name, const CoreFederateInfo &info) override;
  virtual const char* getFederateName (federate_id_t federateId) override;
  virtual federate_id_t getFederateId (const char *name) override;
  virtual void setFederationSize (unsigned int size);
  virtual unsigned int getFederationSize () override;
  virtual Time timeRequest (federate_id_t federateId, Time next) override;
  virtual std::pair<Time, bool> requestTimeIterative (federate_id_t federateId, Time next, bool localConverged) override;
  virtual uint64_t getCurrentReiteration (federate_id_t federateId) override;
  virtual void setMaximumIterations (federate_id_t federateId, uint64_t iterations) override;
  virtual void setTimeDelta (federate_id_t federateId, Time time) override;
  virtual void setLookAhead(federate_id_t federateId, Time timeLookAhead) override;
  virtual void setImpactWindow(federate_id_t federateId, Time timeImpact) override;
  virtual Handle registerSubscription (federate_id_t federateId, const char *key, const char *type, const char *units, bool required) override;
  virtual Handle getSubscription (federate_id_t federateId, const char *key) override;
  virtual Handle registerPublication (federate_id_t federateId, const char *key, const char *type, const char *units) override;
  virtual Handle getPublication (federate_id_t federateId, const char *key) override;
  virtual const char *getUnits (Handle handle) override;
  virtual const char *getType (Handle handle) override;
  virtual void setValue (Handle handle, const char *data, uint64_t len) override;
  virtual data_t* getValue (Handle handle) override;
  virtual void dereference (data_t *data) override;
  virtual void dereference(message_t *msg) override;
  virtual const Handle* getValueUpdates (federate_id_t federateId, uint64_t *size) override;
  virtual Handle registerEndpoint (federate_id_t federateId, const char *name, const char *type) override;
  virtual Handle registerSourceFilter (federate_id_t federateId, const char *filterName, const char *source, const char *type_in) override;
  virtual Handle registerDestinationFilter (federate_id_t federateId, const char *filterName, const char *dest, const char *type_in) override;
  virtual void addDependency(federate_id_t federateId, const char *federateName) override;
  virtual void registerFrequentCommunicationsPair (const char *source, const char *dest) override;
  virtual void send (Handle sourceHandle, const char *destination, const char *data, uint64_t length) override;
  virtual void sendEvent (Time time, Handle sourceHandle, const char *destination, const char *data, uint64_t length) override;
  virtual void sendMessage (message_t *message) override;
  virtual uint64_t receiveCount (Handle destination) override;
  virtual message_t* receive (Handle destination) override;
  virtual std::pair<const Handle, message_t*> receiveAny (federate_id_t federateId) override;
  virtual uint64_t receiveCountAny (federate_id_t federateId) override;
  virtual void logMessage(federate_id_t federateId, int logCode, const char *logMessage) override;
  virtual uint64_t receiveFilterCount(federate_id_t federateID) override;
  virtual std::pair<const Handle, message_t*> receiveAnyFilter(federate_id_t federateID) override;
  virtual void setFilterOperator(Handle filter, FilterOperator* callback) override;
  */
};

} // namespace helics
 
#endif /* HELICS_HAVE_MPI */

#endif /* _HELICS_MPI_CORE_ */
