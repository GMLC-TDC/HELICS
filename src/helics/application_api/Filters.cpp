/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "Filters.hpp"
#include "FilterOperations.hpp"
#include "MessageOperators.hpp"

#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <regex>
#include <thread>

namespace helics
{
static const std::map<std::string, defined_filter_types> filterTypes{{"clone", defined_filter_types::clone},
                                                                     {"cloning", defined_filter_types::clone},
                                                                     {"delay", defined_filter_types::delay},
                                                                     {"timedelay", defined_filter_types::delay},
                                                                     {"randomdelay",
                                                                      defined_filter_types::randomDelay},
                                                                     {"randomdrop",
                                                                      defined_filter_types::randomDrop},
                                                                     {"reroute", defined_filter_types::reroute},
                                                                     {"redirect", defined_filter_types::reroute},
                                                                     {"custom", defined_filter_types::custom}};

defined_filter_types filterTypeFromString (const std::string &filterType) noexcept
{
    auto fnd = filterTypes.find (filterType);
    if (fnd != filterTypes.end ())
    {
        return fnd->second;
    }
    auto nfilt = filterType;
    std::transform (nfilt.begin (), nfilt.end (), nfilt.begin (), ::tolower);
    fnd = filterTypes.find (nfilt);
    if (fnd != filterTypes.end ())
    {
        return fnd->second;
    }
    return defined_filter_types::unrecognized;
}

void addOperations (Filter *filt, defined_filter_types type, Core *cptr)
{
    switch (type)
    {
    case defined_filter_types::custom:
    default:
        break;
    case defined_filter_types::randomDelay:
    {
        auto op = std::make_shared<RandomDelayFilterOperation> ();
        filt->setFilterOperations (std::move (op));
    }
    break;
    case defined_filter_types::delay:
    {
        auto op = std::make_shared<DelayFilterOperation> ();
        filt->setFilterOperations (std::move (op));
    }
    break;
    case defined_filter_types::randomDrop:
    {
        auto op = std::make_shared<RandomDropFilterOperation> ();
        filt->setFilterOperations (std::move (op));
    }
    break;
    case defined_filter_types::reroute:
    {
        auto op = std::make_shared<RerouteFilterOperation> ();
        filt->setFilterOperations (std::move (op));
    }
    break;
    case defined_filter_types::clone:
    {
        auto op = std::make_shared<CloneFilterOperation> (cptr);
        filt->setFilterOperations (std::move (op));
    }
    break;
    }
}

Filter::Filter (Federate *fed, const std::string &name)
{
    if (fed != nullptr)
    {
        corePtr = fed->getCorePointer ().get ();
        fid = fed->registerFilter (name);
        id = interface_handle (fid.value ());
    }
   
}

Filter::Filter (Core *cr,const std::string &name) : corePtr (cr) 
{
	if (corePtr != nullptr)
	{
        id = corePtr->registerFilter (name, std::string (), std::string ());
        fid = id;
	}
    
}

void Filter::setOperator (std::shared_ptr<FilterOperator> mo)
{
    if (corePtr != nullptr)
    {
        corePtr->setFilterOperator (id, std::move (mo));
    }
}

void Filter::setFilterOperations (std::shared_ptr<FilterOperations> filterOps)
{
    filtOp = std::move (filterOps);
    if (corePtr != nullptr)
    {
        if (filtOp)
        {
            corePtr->setFilterOperator (id, filtOp->getOperator ());
        }
        else
        {
            corePtr->setFilterOperator (id, nullptr);
        }
    }
}

static const std::string nullStr;


const std::string &Filter::getName () const
{
    if (corePtr != nullptr)
    {
        return corePtr->getHandleName (id);
    }
    return nullStr;
}

const std::string &Filter::getInputType () const
{
    if (corePtr != nullptr)
    {
        return corePtr->getType (id);
    }
    return nullStr;
}

const std::string &Filter::getOutputType () const
{
    if (corePtr != nullptr)
    {
        return corePtr->getOutputType (id);
    }
    return nullStr;
}

void Filter::set (const std::string &property, double val)
{
    if (filtOp)
    {
        filtOp->set (property, val);
    }
}

void Filter::setString (const std::string &property, const std::string &val)
{
    if (filtOp)
    {
        filtOp->setString (property, val);
    }
}



CloningFilter::CloningFilter (Core *cr, const std::string &name)
{
    corePtr = cr;
    if (corePtr != nullptr)
    {
        fid =corePtr->registerCloningFilter (name,std::string(),std::string());
        id = interface_handle (fid.value ());
    }
    setFilterOperations(std::make_shared<CloneFilterOperation> (cr));
}

CloningFilter::CloningFilter (Federate *fed, const std::string &name)
{
    if (fed != nullptr)
    {
        corePtr = fed->getCorePointer ().get ();
        fid = fed->registerCloningFilter (name);
        id = interface_handle (fid.value ());
		if (corePtr != nullptr)
		{
            setFilterOperations(std::make_shared<CloneFilterOperation> (corePtr));
		}
    }
    
}

void Filter::addSourceTarget (const std::string &sourceName)
{
   // sourceEndpoints.push_back (sourceName);
    corePtr->addSourceTarget (id, sourceName);
}

void Filter::addDestinationTarget (const std::string &destinationName)
{
   // destEndpoints.push_back (destinationName);
    corePtr->addDestinationTarget (id, destinationName);
}

void CloningFilter::addDeliveryEndpoint (const std::string &endpoint)
{
    Filter::setString ("add delivery", endpoint);
}

void Filter::removeTarget (const std::string &sourceName)
{
    corePtr->removeTarget (id, sourceName);
}

void CloningFilter::removeDeliveryEndpoint (const std::string &endpoint)
{
    Filter::setString ("remove delivery", endpoint);
}

void CloningFilter::setString (const std::string &property, const std::string &val)
{
    if ((property == "source") || (property == "add source"))
    {
        addSourceTarget (val);
    }
    else if ((property == "dest") || (property == "destination") || (property == "add destination") ||
             (property == "add dest"))
    {
        addDestinationTarget (val);
    }
    else if ((property == "endpoint") || (property == "add endpoint"))
    {
        addSourceTarget (val);
        addDestinationTarget (val);
    }
    else if ((property == "remove destination") || (property == "remove dest"))
    {
        removeTarget (val);
    }
    else if (property == "remove source")
    {
        removeTarget (val);
    }
    else if (property == "remove endpoint")
    {
        removeTarget (val);
    }
    else
    {
        Filter::setString (property, val);
    }
}

std::unique_ptr<Filter> make_filter (defined_filter_types type,
                                                            Federate *mFed,
                                                            const std::string &name)

{
    if (type == defined_filter_types::clone)
    {
        std::unique_ptr<Filter> dfilt = std::make_unique<CloningFilter> (mFed, name);
        addOperations (dfilt.get (), type, mFed->getCorePointer ().get ());
        dfilt->setString ("delivery", name);
        return dfilt;
    }
    else
    {
        auto dfilt = std::make_unique<Filter> (mFed, name);
        addOperations (dfilt.get (), type, nullptr);
        return dfilt;
    }
}


std::unique_ptr<Filter>
make_filter (defined_filter_types type, Core *cr, const std::string &name)

{
    if (type == defined_filter_types::clone)
    {
        std::unique_ptr<Filter> dfilt = std::make_unique<CloningFilter> (cr, name);
        addOperations (dfilt.get (), type, cr);
        dfilt->setString ("delivery", name);
        return dfilt;
    }
    else
    {
        auto dfilt = std::make_unique<Filter> (cr, name);
        addOperations (dfilt.get (), type, cr);
        return dfilt;
    }
}


std::unique_ptr<CloningFilter> make_cloning_filter (defined_filter_types type, Federate *mFed, const std::string &delivery, const std::string &name)

{
        auto dfilt = std::make_unique<CloningFilter> (mFed, name);
        addOperations (dfilt.get (), type, mFed->getCorePointer ().get ());
        dfilt->addDeliveryEndpoint (delivery);
        return dfilt;
}

std::unique_ptr<CloningFilter>
make_cloning_filter (defined_filter_types type, Core *cr, const std::string &delivery, const std::string &name)

{
        auto dfilt = std::make_unique<CloningFilter> (cr, name);
        addOperations (dfilt.get (), type, cr);
        dfilt->addDeliveryEndpoint(delivery);
        return dfilt;
}

}  // namespace helics
