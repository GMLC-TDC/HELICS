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
                                                                     {"firewall", defined_filter_types::firewall},
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
    case defined_filter_types::firewall:
    {
        auto op = std::make_shared<FirewallFilterOperation> ();
        filt->setFilterOperations (std::move (op));
    }
    break;
    }
}


Filter::Filter (Federate *ffed, const std::string &filtName) : Filter (ffed->registerFilter (filtName))
{
   
}


Filter::Filter (Federate *ffed, const std::string &filtName, interface_handle handle) : fed(ffed), id (handle), name (filtName)
{
    if (ffed != nullptr)
    {
        corePtr = ffed->getCorePointer ().get ();
    }
}

Filter::Filter(interface_visibility locality, Federate *ffed, const std::string &filtName)
{
    if (ffed != nullptr)
    {
        corePtr = ffed->getCorePointer().get();
        if (locality == interface_visibility::global)
        {
            operator=(ffed->registerGlobalFilter(filtName));
        }
        else
        {
            operator=(ffed->registerFilter(filtName));
        }
    }

}

Filter::Filter (Core *cr, const std::string &filtName) : corePtr (cr), name (filtName)
{
	if (corePtr != nullptr)
	{
        id = corePtr->registerFilter (filtName, std::string (), std::string ());
        fed = nullptr;
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



CloningFilter::CloningFilter (Core *cr, const std::string &filtName)
{
    corePtr = cr;
    if (corePtr != nullptr)
    {
        id =corePtr->registerCloningFilter (filtName,std::string(),std::string());
        name = filtName;
    }
    setFilterOperations(std::make_shared<CloneFilterOperation> (cr));
}

CloningFilter::CloningFilter (Federate *ffed, const std::string &name) : Filter (ffed->registerCloningFilter (name))
{
		if (corePtr != nullptr)
		{
            setFilterOperations(std::make_shared<CloneFilterOperation> (corePtr));
		}
}

CloningFilter::CloningFilter(Federate *ffed, const std::string &name, interface_handle handle)
	: Filter(ffed, name, handle)
{

}

CloningFilter::CloningFilter(interface_visibility locality, Federate *ffed, const std::string &name)
{
    if (ffed != nullptr)
    {
        corePtr = ffed->getCorePointer().get();
        if (locality == interface_visibility::global)
        {
            operator=(ffed->registerGlobalCloningFilter(name));
        }
        else
        {
            operator=(ffed->registerCloningFilter(name));
        }
       
        setFilterOperations(std::make_shared<CloneFilterOperation>(corePtr));
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

Filter & make_filter (defined_filter_types type,
                                                            Federate *mFed,
                                                            const std::string &name)

{
    if (type == defined_filter_types::clone)
    {
        Filter &dfilt = mFed->registerCloningFilter (name);
        addOperations (&dfilt, type, mFed->getCorePointer ().get ());
        dfilt.setString ("delivery", name);
        return dfilt;
    }
    else
    {
        auto &dfilt = mFed->registerFilter (name);
        addOperations (&dfilt, type, nullptr);
        return dfilt;
    }
}

Filter &make_filter(interface_visibility locality, defined_filter_types type,
    Federate *mFed,
    const std::string &name)

{
    if (type == defined_filter_types::clone)
    {
        Filter &dfilt = (locality == interface_visibility::global) ? mFed->registerGlobalCloningFilter (name) :
                                                                     mFed->registerCloningFilter (name); 
        addOperations(&dfilt, type, mFed->getCorePointer().get());
        dfilt.setString("delivery", name);
        return dfilt;
    }
    else
    {
        auto &dfilt = (locality == interface_visibility::global) ? mFed->registerGlobalFilter (name) :
                                                                   mFed->registerFilter (name);
        addOperations(&dfilt, type, nullptr);
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


CloningFilter &make_cloning_filter (defined_filter_types type, Federate *mFed, const std::string &delivery, const std::string &name)

{
    auto &dfilt = mFed->registerCloningFilter (name);
        addOperations (&dfilt, type, mFed->getCorePointer ().get ());
        dfilt.addDeliveryEndpoint (delivery);
        return dfilt;
}

CloningFilter & make_cloning_filter (interface_visibility locality,
                                    defined_filter_types type,
                                    Federate *mFed,
                                    const std::string &delivery,
                                    const std::string &name)

{
    auto &dfilt = (locality == interface_visibility::global) ? mFed->registerGlobalCloningFilter (name) :
                                                               mFed->registerCloningFilter (name); 
    addOperations(&dfilt, type, mFed->getCorePointer().get());
    dfilt.addDeliveryEndpoint(delivery);
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
