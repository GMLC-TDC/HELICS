/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "application_api/filters.hpp"
#include "application_api/application_api.h"
#include "core/helics-time.h"
#include "helics.h"
#include "shared_api_library/internal/api_objects.h"
#include <memory>
#include <mutex>
#include <vector>

helics_source_filter helicsRegisterSourceFilter (helics_message_filter_federate fed,
                                                 const char *name,
                                                 const char *inputType,
                                                 const char *outputType)
{
	// now generate a generic subscription
	auto fedObj = getFilterFedSharedPtr(fed);
	if (!fedObj)
	{
		return nullptr;
	}
	helics::SourceFilterObject *filt = nullptr;
	try
	{
		filt = new helics::SourceFilterObject();
		filt->filtptr = std::make_unique<helics::SourceFilter>(fedObj.get(), name, inputType, outputType);
		filt->fedptr = std::move(fedObj);
		return reinterpret_cast<helics_source_filter> (filt);
	}
	catch (const helics::InvalidFunctionCall &)
	{
		if (filt != nullptr)
		{
			delete filt;
		}
	}
	return nullptr;
}

helics_destination_filter helicsRegisterDestinationFilter (helics_message_filter_federate fed,
                                                           const char *name,
                                                           const char *inputType,
                                                           const char *outputType)
{
	// now generate a generic subscription
	auto fedObj = getFilterFedSharedPtr(fed);
	if (!fedObj)
	{
		return nullptr;
	}
	helics::DestFilterObject *filt = nullptr;
	try
	{
		filt = new helics::DestFilterObject();
		filt->filtptr = std::make_unique<helics::DestinationFilter>(fedObj.get(), name, inputType, outputType);
		filt->fedptr = std::move(fedObj);
		return reinterpret_cast<helics_destination_filter> (filt);
	}
	catch (const helics::InvalidFunctionCall &)
	{
		if (filt != nullptr)
		{
			delete filt;
		}
	}
	return nullptr;
}

int helicsFederateHasMessageToFilter (helics_message_filter_federate fed) 
{ 
	if (fed == nullptr)
	{
		return false;
	}
	auto mFed = getFilterFed(fed);
	return (mFed->hasMessageToFilter()) ? 1 : 0;
}

int helicsFilterHasMessage (helics_source_filter filt) 
{ 
	if (filt== nullptr)
	{
		return false;
	}

	auto filtObj = reinterpret_cast<helics::SourceFilterObject *> (filt);
	return (filtObj->filtptr->hasMessage()) ? 1 : 0;
}

int helicsFederateFilterReceiveCount (helics_message_filter_federate fedID) { return 0; }

int helicsFilterReceiveCount (helics_source_filter filter) { return 0; }


static message_t emptyMessage()
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

message_t helicsFilterGetMessage(helics_source_filter filter)
{
	if (filter == nullptr)
	{
		return emptyMessage();
	}

	auto filtObj = reinterpret_cast<helics::SourceFilterObject *> (filter);
	filtObj->lastMessage = filtObj->filtptr->getMessage();
	message_t mess;
	mess.data = filtObj->lastMessage->data.data();
	mess.dst = filtObj->lastMessage->dest.c_str();
	mess.length = filtObj->lastMessage->data.size();
	mess.origsrc = filtObj->lastMessage->origsrc.c_str();
	mess.src = filtObj->lastMessage->src.c_str();
	mess.time = filtObj->lastMessage->time.getBaseTimeCode();
	return mess;
}
