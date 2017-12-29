/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "Filters.hpp"
#include "FilterOperations.h"
#include "MessageOperators.h"

#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <regex>
#include <thread>

namespace helics
{

void addOperations (Filter *filt, defined_filter_types type)
{
    switch (type)
    {
    case custom:
        break;
    case randomDelay:
    {
        auto op = std::make_shared<randomDelayFilterOperation> ();
        filt->setFilterOperations (std::move (op));
    }
    break;
    case delay:
    {
        auto op = std::make_shared<delayFilterOperation> ();
        filt->setFilterOperations (std::move (op));
    }
    break;
    case randomDrop:
    {
        auto op = std::make_shared<randomDropFilterOperation> ();
        filt->setFilterOperations (std::move (op));
    }
    break;
    case reroute:
    {
        auto op = std::make_shared<randomDropFilterOperation> ();
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
        corePtr->setFilterOperator (id, filtOp->getOperator ());
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

std::unique_ptr<DestinationFilter> make_destination_filter (defined_filter_types type,
                                                            Federate *mFed,
                                                            const std::string &target,
                                                            const std::string &name)

{
    auto dfilt = std::make_unique<DestinationFilter> (mFed, target, name);
    addOperations (dfilt.get (), type);
    return dfilt;
}

std::unique_ptr<SourceFilter>
make_source_filter (defined_filter_types type, Core *cr, const std::string &target, const std::string &name)
{
    auto sfilt = std::make_unique<SourceFilter> (cr, target, name);
    addOperations (sfilt.get (), type);
    return sfilt;
}

std::unique_ptr<DestinationFilter>
make_destination_filter (defined_filter_types type, Core *cr, const std::string &target, const std::string &name)

{
    auto dfilt = std::make_unique<DestinationFilter> (cr, target, name);
    addOperations (dfilt.get (), type);
    return dfilt;
}

std::unique_ptr<SourceFilter>
make_source_filter (defined_filter_types type, Federate *fed, const std::string &target, const std::string &name)
{
    auto sfilt = std::make_unique<SourceFilter> (fed, target, name);
    addOperations (sfilt.get (), type);
    return sfilt;
}

}  // namespace helics