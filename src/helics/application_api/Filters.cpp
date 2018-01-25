/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

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
        auto op = std::make_shared<RandomDropFilterOperation> ();
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

Filter::Filter (Federate *fed)
{
    if (fed != nullptr)
    {
        corePtr = fed->getCorePointer ().get ();
    }
}

Filter::Filter (Core *cr) : corePtr (cr) {}

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

const std::string &Filter::getTarget () const
{
    if (corePtr != nullptr)
    {
        return corePtr->getTarget (id);
    }
    return nullStr;
}

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

SourceFilter::SourceFilter (Federate *fed,
                            const std::string &target,
                            const std::string &name,
                            const std::string &input_type,
                            const std::string &output_type)
    : Filter (fed)
{
    if (fed != nullptr)
    {
        fid = fed->registerSourceFilter (name, target, input_type, output_type);
        id = fid.value ();
    }
}

SourceFilter::SourceFilter (Core *cr,
                            const std::string &target,
                            const std::string &name,
                            const std::string &input_type,
                            const std::string &output_type)
    : Filter (cr)
{
    if (corePtr != nullptr)
    {
        id = corePtr->registerSourceFilter (name, target, input_type, output_type);
        fid = id;
    }
}

DestinationFilter::DestinationFilter (Federate *fed,
                                      const std::string &target,
                                      const std::string &name,
                                      const std::string &input_type,
                                      const std::string &output_type)
    : Filter (fed)
{
    if (fed != nullptr)
    {
        fid = fed->registerDestinationFilter (name, target, input_type, output_type);
        id = fid.value ();
    }
}

DestinationFilter::DestinationFilter (Core *cr,
                                      const std::string &target,
                                      const std::string &name,
                                      const std::string &input_type,
                                      const std::string &output_type)
    : Filter (cr)
{
    if (corePtr != nullptr)
    {
        id = corePtr->registerDestinationFilter (name, target, input_type, output_type);
        fid = id;
    }
}

CloningFilter::CloningFilter (Core *cr) : Filter (cr) { filtOp = std::make_shared<CloneFilterOperation> (cr); }

CloningFilter::CloningFilter (Federate *fed) : Filter (fed)
{
    filtOp = std::make_shared<CloneFilterOperation> (fed->getCorePointer ().get ());
}

void CloningFilter::addSourceTarget (const std::string &sourceName)
{
    auto filtid = corePtr->registerSourceFilter (getName (), sourceName, std::string (), std::string ());
    sourceFilters.push_back (filtid);
    sourceEndpoints.push_back (sourceName);
    corePtr->setFilterOperator (filtid, filtOp->getOperator ());
}

void CloningFilter::addDestinationTarget (const std::string &destinationName)
{
    auto filtid = corePtr->registerDestinationFilter (getName (), destinationName, std::string (), std::string ());
    destFilters.push_back (filtid);
    destEndpoints.push_back (destinationName);
    corePtr->setFilterOperator (filtid, filtOp->getOperator ());
}

void CloningFilter::addDeliveryEndpoint (const std::string &endpoint)
{
    Filter::setString ("add delivery", endpoint);
}

void CloningFilter::removeSourceTarget (const std::string &sourceName)
{
    for (size_t ii = 0; ii < sourceEndpoints.size (); ++ii)
    {
        if (sourceEndpoints[ii] == sourceName)
        {
            corePtr->setFilterOperator (sourceFilters[ii].value (), nullptr);
        }
    }
}

void CloningFilter::removeDestinationTarget (const std::string &destinationName)
{
    for (size_t ii = 0; ii < destEndpoints.size (); ++ii)
    {
        if (destEndpoints[ii] == destinationName)
        {
            corePtr->setFilterOperator (destFilters[ii].value (), nullptr);
        }
    }
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
    else if ((property == "dest") || (property == "destination") || (property == "add destination") || (property == "add dest"))
    {
        addDestinationTarget (val);
    }
    else if ((property == "endpoint") || (property == "add endpoint"))
    {
        addSourceTarget(val);
        addDestinationTarget(val);
    }
    else if ((property == "remove destination") || (property == "remove dest"))
    {
        removeDestinationTarget (val);
    }
    else if (property == "remove source")
    {
        removeSourceTarget (val);
    }
    else if (property == "remove endpoint")
    {
        removeDestinationTarget(val);
        removeSourceTarget(val);
    }
    else
    {
        Filter::setString (property, val);
    }
}

std::unique_ptr<DestinationFilter> make_destination_filter (defined_filter_types type,
                                                            Federate *mFed,
                                                            const std::string &target,
                                                            const std::string &name)

{
    if (type == defined_filter_types::clone)
    {
        auto dfilt = std::make_unique<DestinationFilter> (mFed, target, name);
        addOperations (dfilt.get (), type, mFed->getCorePointer ().get ());
        dfilt->setString ("delivery", name);
        return dfilt;
    }
    else
    {
        auto dfilt = std::make_unique<DestinationFilter> (mFed, target, name);
        addOperations (dfilt.get (), type, nullptr);
        return dfilt;
    }
}

std::unique_ptr<SourceFilter>
make_source_filter (defined_filter_types type, Core *cr, const std::string &target, const std::string &name)
{
    if (type == defined_filter_types::clone)
    {
        auto dfilt = std::make_unique<SourceFilter> (cr, target, name);
        addOperations (dfilt.get (), type, cr);
        dfilt->setString ("delivery", name);
        return dfilt;
    }
    else
    {
        auto sfilt = std::make_unique<SourceFilter> (cr, target, name);
        addOperations (sfilt.get (), type, cr);
        return sfilt;
    }
}

std::unique_ptr<DestinationFilter>
make_destination_filter (defined_filter_types type, Core *cr, const std::string &target, const std::string &name)

{
    if (type == defined_filter_types::clone)
    {
        auto dfilt = std::make_unique<DestinationFilter> (cr, target, name);
        addOperations (dfilt.get (), type, cr);
        dfilt->setString ("delivery", name);
        return dfilt;
    }
    else
    {
        auto dfilt = std::make_unique<DestinationFilter> (cr, target, name);
        addOperations (dfilt.get (), type, cr);
        return dfilt;
    }
}

std::unique_ptr<SourceFilter>
make_source_filter (defined_filter_types type, Federate *fed, const std::string &target, const std::string &name)
{
    if (type == defined_filter_types::clone)
    {
        auto dfilt = std::make_unique<SourceFilter> (fed, target, name);
        addOperations (dfilt.get (), type, fed->getCorePointer ().get ());
        dfilt->setString ("delivery", name);
        return dfilt;
    }
    else
    {
        auto sfilt = std::make_unique<SourceFilter> (fed, target, name);
        addOperations (sfilt.get (), type, nullptr);
        return sfilt;
    }
}

}  // namespace helics