/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "MessageFilterFederate.h"
#include "MessageFilterFederateManager.h"
#include "core/core.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#include "json/json.h"
#pragma warning(pop)
#else
#include "json/json.h"
#endif

#include <fstream>

namespace helics
{
MessageFilterFederate::MessageFilterFederate() = default;

MessageFilterFederate::MessageFilterFederate (const FederateInfo &fi) : Federate (fi), MessageFederate (true)
{
    filterManager = std::make_unique<MessageFilterFederateManager> (coreObject, getID ());
}

MessageFilterFederate::MessageFilterFederate (const std::string &file) : Federate (file), MessageFederate (true)
{
    filterManager = std::make_unique<MessageFilterFederateManager> (coreObject, getID ());
}


MessageFilterFederate::MessageFilterFederate (MessageFilterFederate &&mFed) noexcept
    : Federate (std::move (mFed)), MessageFederate (std::move (mFed)),
      filterManager (std::move (mFed.filterManager))
{
    filterCount.store (mFed.filterCount.load ());
}

MessageFilterFederate &MessageFilterFederate::operator= (MessageFilterFederate &&mFed) noexcept
{
    MessageFederate::operator= (std::move (mFed));
    filterManager = std::move (mFed.filterManager);
    filterCount.store (mFed.filterCount.load ());
    return *this;
}

MessageFilterFederate::~MessageFilterFederate () = default;

void MessageFilterFederate::updateTime (Time newTime, Time oldTime)
{
    filterManager->updateTime (newTime, oldTime);
}

void MessageFilterFederate::StartupToInitializeStateTransition ()
{
    filterManager->StartupToInitializeStateTransition ();
}
void MessageFilterFederate::InitializeToExecuteStateTransition ()
{
    filterManager->InitializeToExecuteStateTransition ();
}


void MessageFilterFederate::registerInterfaces (const std::string &jsonString)
{
    MessageFederate::registerInterfaces (jsonString);
    std::ifstream file (jsonString);
    Json_helics::Value doc;

    if (file.is_open ())
    {
        Json_helics::CharReaderBuilder rbuilder;
        std::string errs;
        bool ok = Json_helics::parseFromStream (rbuilder, file, &doc, &errs);
        if (!ok)
        {
            // should I throw an error here?
            return;
        }
    }
    else
    {
        Json_helics::Reader stringReader;
        bool ok = stringReader.parse (jsonString, doc, false);
        if (!ok)
        {
            // should I throw an error here?
            return;
        }
    }
    if (doc.isMember ("filters"))
    {
        auto filts = doc["filters"];
        for (auto filtIt = filts.begin (); filtIt != filts.end (); ++filtIt)
        {
            auto filt = (*filtIt);
            auto name = (filt.isMember ("name")) ? filt["name"].asString () : "";
            auto target = filt["target"].asString ();
            auto mode = filt["mode"].asString ();

            auto type = (filt.isMember ("type")) ? filt["type"].asString () : "";
            if (mode == "source")
            {
                if ((name.empty ()) && (type.empty ()))
                {
                    registerSourceFilter (target);
                }
                else
                {
                    registerSourceFilter (name, target, type);
                }
            }
            else if (mode == "dest")
            {
                if ((name.empty ()) && (type.empty ()))
                {
                    registerDestinationFilter (target);
                }
                else
                {
                    registerDestinationFilter (name, target, type);
                }
            }
        }
    }
}


filter_id_t MessageFilterFederate::registerSourceFilter (const std::string &filterName,
                                                         const std::string &sourceEndpoint,
                                                         const std::string &inputType,
														 const std::string &outputType)
{
    if (state == op_states::startup)
    {
        ++filterCount;
        if (!filterName.empty ())
        {
            return filterManager->registerSourceFilter (filterName, sourceEndpoint, inputType, outputType);
        }
        auto cnt = filterCount.load ();
        return filterManager->registerSourceFilter ("filter" + std::to_string (cnt), sourceEndpoint, inputType,outputType);
    }
    throw (InvalidFunctionCall ("filters can only be created in startup mode"));
}

filter_id_t MessageFilterFederate::registerDestinationFilter (const std::string &filterName,
                                                              const std::string &destEndpoint,
                                                              const std::string &inputType,
																const std::string &outputType)
{
    if (state == op_states::startup)
    {
        ++filterCount;
        if (!filterName.empty ())
        {
            return filterManager->registerDestinationFilter (filterName, destEndpoint, inputType, outputType);
        }
        auto cnt = filterCount.load ();
        return filterManager->registerDestinationFilter ("filter" + std::to_string (cnt), destEndpoint, inputType, outputType);
    }
    throw (InvalidFunctionCall ("filters can only be created in startup mode"));
}


static const std::string nullStr;

filter_id_t MessageFilterFederate::registerSourceFilter (const std::string &sourceEndpoint)
{
    if (state == op_states::startup)
    {
        auto cnt = filterCount++;
        return filterManager->registerSourceFilter ("filter" + std::to_string (cnt), sourceEndpoint, nullStr, nullStr);
    }
    throw (InvalidFunctionCall ("filters can only be created in startup mode"));
}

filter_id_t MessageFilterFederate::registerDestinationFilter (const std::string &destEndpoint)
{
    if (state == op_states::startup)
    {
        auto cnt = filterCount++;
        return filterManager->registerDestinationFilter ("filter" + std::to_string (cnt), destEndpoint, nullStr, nullStr);
    }
    throw (InvalidFunctionCall ("filters can only be created in startup mode"));
}

bool MessageFilterFederate::hasMessageToFilter () const { return filterManager->hasMessageToFilter (); }

bool MessageFilterFederate::hasMessageToFilter (filter_id_t filter) const
{
    return filterManager->hasMessageToFilter (filter);
}

std::unique_ptr<Message> MessageFilterFederate::getMessageToFilter (filter_id_t filter)
{
    return filterManager->getMessageToFilter (filter);
}

std::string MessageFilterFederate::getFilterName (filter_id_t id) const
{
    return filterManager->getFilterName (id);
}

std::string MessageFilterFederate::getFilterEndpoint (filter_id_t id) const
{
    return filterManager->getFilterEndpoint (id);
}

std::string MessageFilterFederate::getFilterInputType (filter_id_t id) const
{
    return filterManager->getFilterInputType (id);
}

std::string MessageFilterFederate::getFilterOutputType(filter_id_t id) const
{
	return filterManager->getFilterOutputType(id);
}

filter_id_t MessageFilterFederate::getFilterId (const std::string &filterName) const
{
    return filterManager->getFilterId (filterName);
}


filter_id_t MessageFilterFederate::getSourceFilterId (const std::string &name) const
{
    return filterManager->getSourceFilterId (name);
}
filter_id_t MessageFilterFederate::getDestFilterId (const std::string &name) const
{
    return filterManager->getDestFilterId (name);
}

void MessageFilterFederate::registerFilterCallback (std::function<void(filter_id_t, Time)> func)
{
    filterManager->registerFilterCallback (func);
}
void MessageFilterFederate::registerFilterCallback (filter_id_t filter, std::function<void(filter_id_t, Time)> func)
{
    filterManager->registerFilterCallback (filter, func);
}
void MessageFilterFederate::registerFilterCallback (const std::vector<filter_id_t> &filters,
                                                    std::function<void(filter_id_t, Time)> func)
{
    filterManager->registerFilterCallback (filters, func);
}

void MessageFilterFederate::registerMessageOperator (std::shared_ptr<MessageOperator> mo)
{
            filterManager->registerMessageOperator (mo);
}

void MessageFilterFederate::registerMessageOperator (filter_id_t filter, std::shared_ptr<MessageOperator> mo)
{
   
   filterManager->registerMessageOperator (filter, mo);
     
}

void MessageFilterFederate::registerMessageOperator (const std::vector<filter_id_t> &filters,
                                                     std::shared_ptr<MessageOperator> mo)
{
         filterManager->registerMessageOperator (filters, mo);
}
}