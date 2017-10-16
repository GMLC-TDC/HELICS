/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "application_api/Filters.hpp"
#include "application_api/application_api.h"
#include "core/helics-time.h"
#include "helics.h"
#include "shared_api_library/internal/api_objects.h"
#include <memory>
#include <mutex>
#include <vector>

helics_source_filter
helicsRegisterSourceFilter (helics_message_filter_federate fed, const char *name, const char *inputType, const char *outputType)
{
    // now generate a generic subscription
    auto fedObj = getFilterFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }
    helics::SourceFilterObject *filt = nullptr;
    try
    {
        filt = new helics::SourceFilterObject ();
        filt->filtptr = std::make_unique<helics::SourceFilter> (fedObj.get (), name, inputType, outputType);
        filt->fedptr = std::move (fedObj);
        return reinterpret_cast<helics_source_filter> (filt);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete filt;
    }
    return nullptr;
}

helics_destination_filter
helicsRegisterDestinationFilter (helics_message_filter_federate fed, const char *name, const char *inputType, const char *outputType)
{
    // now generate a generic subscription
    auto fedObj = getFilterFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }
    helics::DestFilterObject *filt = nullptr;
    try
    {
        filt = new helics::DestFilterObject ();
        filt->filtptr = std::make_unique<helics::DestinationFilter> (fedObj.get (), name, inputType, outputType);
        filt->fedptr = std::move (fedObj);
        return reinterpret_cast<helics_destination_filter> (filt);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete filt;
    }
    return nullptr;
}

int helicsFederateHasMessageToFilter (helics_message_filter_federate fed)
{
    if (fed == nullptr)
    {
        return -1;
    }
    auto mFed = getFilterFed (fed);
    return (mFed->hasMessageToFilter ()) ? 1 : 0;
}

int helicsFilterHasMessage (helics_source_filter filter)
{
    if (filter == nullptr)
    {
        return 0;
    }

    auto filtObj = reinterpret_cast<helics::SourceFilterObject *> (filter);
    return (filtObj->filtptr->hasMessage ()) ? 1 : 0;
}

int helicsFilterReceiveCount (helics_source_filter filter)
{
    if (filter == nullptr)
    {
        return -1;
    }

    auto filtObj = reinterpret_cast<helics::SourceFilterObject *> (filter);
    // this isn't right yet but I might need to enable that functionality yet
    return (filtObj->filtptr->hasMessage ()) ? 1 : 0;
}

static message_t emptyMessage ()
{
    message_t empty;
    empty.time = 0;
    empty.data = nullptr;
    empty.length = 0;
    empty.dst = nullptr;
    empty.origsrc = nullptr;
    empty.src = nullptr;
    return empty;
}

message_t helicsFilterGetMessage (helics_source_filter filter)
{
    if (filter == nullptr)
    {
        return emptyMessage ();
    }

    auto filtObj = reinterpret_cast<helics::SourceFilterObject *> (filter);
    filtObj->lastMessage = filtObj->filtptr->getMessage ();
    message_t mess{};
    mess.data = filtObj->lastMessage->data.data ();
    mess.dst = filtObj->lastMessage->dest.c_str ();
    mess.length = filtObj->lastMessage->data.size ();
    mess.origsrc = filtObj->lastMessage->origsrc.c_str ();
    mess.src = filtObj->lastMessage->src.c_str ();
    mess.time = filtObj->lastMessage->time.getBaseTimeCode ();
    return mess;
}

void helicsFreeSourceFilter (helics_source_filter filter) { delete reinterpret_cast<helics::SourceFilterObject *> (filter); }

void helicsFreeDestinationFilter (helics_destination_filter filter) { delete reinterpret_cast<helics::DestFilterObject *> (filter); }
