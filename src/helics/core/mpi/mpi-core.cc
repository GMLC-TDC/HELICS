/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/config.h"

#if HELICS_HAVE_MPI

#include "helics/core/mpi/mpi-core.h"

#define OMPI_SKIP_MPICXX
#include "mpi.h"

#include <cassert>

namespace helics
{
MpiCore::MpiCore () {}

MpiCore::~MpiCore () {}

void MpiCore::initialize (const std::string &initializationString)
{
    int provided;

    int error = MPI_Init_thread (0, 0, MPI_THREAD_FUNNELED, &provided);
    assert (!error);
    assert (provided == MPI_THREAD_FUNNELED);
}

void MpiCore::terminate()
{
}

void MpiCore::transmit(int route_id, ActionMessage & cmd)
{
}

void MpiCore::addRoute(int route_id, const std::string & routeInfo)
{
}

/*
bool MpiCore::isInitialized () { return false; }

void MpiCore::error (Core::federate_id_t federateId, int errorCode){}

void MpiCore::finalize (Core::federate_id_t federateId){}

void MpiCore::enterInitializingState(Core::federate_id_t federateID) { }

bool MpiCore::enterExecutingState (Core::federate_id_t federateID, bool iterationCompleted){ return true; }

Core::federate_id_t MpiCore::registerFederate (const char *name, const FederateInfo &info)
{
    return static_cast<Core::federate_id_t> (0);
}

const char *MpiCore::getFederateName (Core::federate_id_t federateId) { return ""; }

Core::federate_id_t MpiCore::getFederateId (const char *name) { return static_cast<Core::federate_id_t> (0); }

void MpiCore::setFederationSize (unsigned int size){}

unsigned int MpiCore::getFederationSize () { return 0; }

Time MpiCore::timeRequest (Core::federate_id_t federateId, Time next) { return 0; }

std::pair<Time, bool>
MpiCore::requestTimeIterative (Core::federate_id_t federateId, Time next, bool localConverged)
{
    return std::make_pair (0, true);
}

uint64_t MpiCore::getCurrentReiteration (Core::federate_id_t federateId) { return 0; }

void MpiCore::setMaximumIterations (federate_id_t federateId, uint64_t iterations){}

void MpiCore::setTimeDelta (Core::federate_id_t federateId, Time time){}

void MpiCore::setLookAhead (Core::federate_id_t federateId, Time time){}

void MpiCore::setImpactWindow(federate_id_t federateID, Time ImpactTime) {}

Core::Handle MpiCore::registerSubscription (Core::federate_id_t federateId,
                                            const char *key,
                                            const char *type,
                                            const char *units,
                                            bool required)
{
    return Core::Handle ();
}

Core::Handle MpiCore::getSubscription (Core::federate_id_t federateId, const char *key)
{
    return Core::Handle ();
}

Core::Handle
MpiCore::registerPublication (Core::federate_id_t federateId, const char *key, const char *type, const char *units)
{
    return Core::Handle ();
}

Core::Handle MpiCore::getPublication (Core::federate_id_t federateId, const char *key) { return Core::Handle (); }

const char *MpiCore::getUnits (Core::Handle handle) { return ""; }

const char *MpiCore::getType (Core::Handle handle) { return ""; }

void MpiCore::setValue (Core::Handle handle, const char *data, uint64_t len){}

data_t *MpiCore::getValue (Core::Handle handle) { return nullptr; }

void MpiCore::dereference (data_t *data){}
void MpiCore::dereference (message_t *msg){}

const Core::Handle *MpiCore::getValueUpdates (Core::federate_id_t federateId, uint64_t *size) { return nullptr; }

Core::Handle MpiCore::registerEndpoint (Core::federate_id_t federateId, const char *name, const char *type)
{
    return Core::Handle ();
}

Core::Handle MpiCore::registerSourceFilter (Core::federate_id_t federateId,
                                            const char *filterName,
                                            const char *source,
                                            const char *type_in)
{
    return Core::Handle ();
}
Core::Handle MpiCore::registerDestinationFilter (Core::federate_id_t federateId,
                                                 const char *filterName,
                                                 const char *dest,
                                                 const char *type_in)
{
    return Core::Handle ();
}
void MpiCore::addDependency(federate_id_t federateId, const char *federateName)
{

}
void MpiCore::registerFrequentCommunicationsPair (const char *source, const char *dest){}

void MpiCore::send (Core::Handle sourceHandle, const char *destination, const char *data, uint64_t len){}

void MpiCore::sendEvent (Time time, Handle source, const char *destination, const char *data, uint64_t len){}

void MpiCore::sendMessage (message_t *message){}

uint64_t MpiCore::receiveCount (Handle destination) { return 0; }

message_t *MpiCore::receive (Core::Handle destination) { return nullptr; }

std::pair<const Core::Handle, message_t*> MpiCore::receiveAny(Core::federate_id_t federateId) { return{ 0xFFFFFFFF,nullptr }; }

uint64_t MpiCore::receiveCountAny (Core::federate_id_t federateId) { return 0; }

void MpiCore::logMessage (Core::federate_id_t federateId, int logCode, const char *logMessage) {}

uint64_t MpiCore::receiveFilterCount(federate_id_t federateID) { return 0; }

std::pair<const Core::Handle, message_t*> MpiCore::receiveAnyFilter(federate_id_t federateID) { return{ 0xFFFFFFFF, nullptr }; }

void MpiCore::setFilterOperator(Handle filter, FilterOperator* callback) {}
*/
}  // namespace helics

#endif
