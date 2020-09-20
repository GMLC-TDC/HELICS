/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "Filters.hpp"

#include "CoreApp.hpp"
#include "FilterOperations.hpp"

#include <algorithm>
#include <map>
#include <memory>
#include <utility>

namespace helics {
static const std::map<std::string, filter_types> filterTypes{
    {"clone", filter_types::clone},
    {"cloning", filter_types::clone},
    {"delay", filter_types::delay},
    {"timedelay", filter_types::delay},
    {"randomdelay", filter_types::random_delay},
    {"randomdrop", filter_types::random_drop},
    {"time_delay", filter_types::delay},
    {"random_delay", filter_types::random_delay},
    {"random_drop", filter_types::random_drop},
    {"time delay", filter_types::delay},
    {"random delay", filter_types::random_delay},
    {"random drop", filter_types::random_drop},
    {"reroute", filter_types::reroute},
    {"redirect", filter_types::reroute},
    {"firewall", filter_types::firewall},
    {"custom", filter_types::custom}};

filter_types filterTypeFromString(const std::string& filterType) noexcept
{
    auto fnd = filterTypes.find(filterType);
    if (fnd != filterTypes.end()) {
        return fnd->second;
    }
    auto nfilt = filterType;
    std::transform(nfilt.begin(), nfilt.end(), nfilt.begin(), ::tolower);
    fnd = filterTypes.find(nfilt);
    if (fnd != filterTypes.end()) {
        return fnd->second;
    }
    return filter_types::unrecognized;
}

void addOperations(Filter* filt, filter_types type, Core* /*cptr*/)
{
    switch (type) {
        case filter_types::custom:
        default:
            break;
        case filter_types::random_delay: {
            auto op = std::make_shared<RandomDelayFilterOperation>();
            filt->setFilterOperations(std::move(op));
        } break;
        case filter_types::delay: {
            auto op = std::make_shared<DelayFilterOperation>();
            filt->setFilterOperations(std::move(op));
        } break;
        case filter_types::random_drop: {
            auto op = std::make_shared<RandomDropFilterOperation>();
            filt->setFilterOperations(std::move(op));
        } break;
        case filter_types::reroute: {
            auto op = std::make_shared<RerouteFilterOperation>();
            filt->setFilterOperations(std::move(op));
        } break;
        case filter_types::clone: {
            auto op = std::make_shared<CloneFilterOperation>();
            filt->setFilterOperations(std::move(op));
        } break;
        case filter_types::firewall: {
            auto op = std::make_shared<FirewallFilterOperation>();
            filt->setFilterOperations(std::move(op));
        } break;
    }
}

Filter::Filter(Federate* ffed, const std::string& filtName):
    Filter(interface_visibility::local, ffed, filtName)
{
}

Filter::Filter(Federate* ffed, const std::string& filtName, InterfaceHandle ihandle):
    Interface(ffed, ihandle, filtName)
{
}

Filter::Filter(Core* core, const std::string& filtName, InterfaceHandle ihandle):
    Interface(core, ihandle, filtName)
{
}

Filter::Filter(interface_visibility locality, Federate* ffed, const std::string& filtName):
    Interface(ffed, InterfaceHandle(), filtName)
{
    if (ffed != nullptr) {
        if (locality == interface_visibility::global) {
            handle = ffed->registerGlobalFilter(filtName);
        } else {
            handle = ffed->registerFilter(filtName);
        }
    }
}

Filter::Filter(Core* core, const std::string& filtName):
    Interface(core, InterfaceHandle(), filtName)
{
    if (cr != nullptr) {
        handle = cr->registerFilter(filtName, std::string(), std::string());
    }
}

void Filter::setOperator(std::shared_ptr<FilterOperator> mo)
{
    if (cr != nullptr) {
        cr->setFilterOperator(handle, std::move(mo));
    }
}

void Filter::setFilterOperations(std::shared_ptr<FilterOperations> filterOps)
{
    filtOp = std::move(filterOps);
    if (cr != nullptr) {
        cr->setFilterOperator(handle, (filtOp) ? filtOp->getOperator() : nullptr);
    }
}

static const std::string emptyStr;

void Filter::set(const std::string& property, double val)
{
    if (filtOp) {
        filtOp->set(property, val);
    }
}

void Filter::setString(const std::string& property, const std::string& val)
{
    if (filtOp) {
        filtOp->setString(property, val);
    }
}

CloningFilter::CloningFilter(Core* core, const std::string& filtName):
    Filter(core, filtName, InterfaceHandle())
{
    if (cr != nullptr) {
        handle = cr->registerCloningFilter(filtName, std::string(), std::string());
    }
    setFilterOperations(std::make_shared<CloneFilterOperation>());
}

CloningFilter::CloningFilter(Federate* ffed, const std::string& filtName):
    Filter(ffed, filtName, InterfaceHandle())
{
    if (ffed != nullptr) {
        handle = ffed->registerCloningFilter(filtName);
    }
    if (cr != nullptr) {
        setFilterOperations(std::make_shared<CloneFilterOperation>());
    }
}

CloningFilter::CloningFilter(Federate* ffed, const std::string& filtName, InterfaceHandle ihandle):
    Filter(ffed, filtName, ihandle)
{
}

CloningFilter::CloningFilter(interface_visibility locality,
                             Federate* ffed,
                             const std::string& filtName):
    Filter(ffed, filtName, InterfaceHandle())
{
    if (ffed != nullptr) {
        if (locality == interface_visibility::global) {
            operator=(ffed->registerGlobalCloningFilter(filtName));
        } else {
            operator=(ffed->registerCloningFilter(filtName));
        }

        setFilterOperations(std::make_shared<CloneFilterOperation>());
    }
}

void CloningFilter::addDeliveryEndpoint(const std::string& endpoint)
{
    Filter::setString("add delivery", endpoint);
}

void CloningFilter::removeDeliveryEndpoint(const std::string& endpoint)
{
    Filter::setString("remove delivery", endpoint);
}

void CloningFilter::setString(const std::string& property, const std::string& val)
{
    if ((property == "source") || (property == "add source")) {
        addSourceTarget(val);
    } else if (property == "dest" || property == "destination" || property == "add destination" ||
               property == "add dest") {
        addDestinationTarget(val);
    } else if (property == "endpoint" || property == "add endpoint") {
        addSourceTarget(val);
        addDestinationTarget(val);
    } else if (property == "remove destination" || property == "remove dest" ||
               property == "remove source" || property == "remove endpoint") {
        removeTarget(val);
    } else {
        Filter::setString(property, val);
    }
}

Filter& make_filter(filter_types type, Federate* mFed, const std::string& name)
{
    if (type == filter_types::clone) {
        Filter& dfilt = mFed->registerCloningFilter(name);
        addOperations(&dfilt, type, mFed->getCorePointer().get());
        dfilt.setString("delivery", name);
        return dfilt;
    }
    auto& dfilt = mFed->registerFilter(name);
    addOperations(&dfilt, type, nullptr);
    return dfilt;
}

Filter& make_filter(interface_visibility locality,
                    filter_types type,
                    Federate* mFed,
                    const std::string& name)
{
    if (type == filter_types::clone) {
        Filter& dfilt = (locality == interface_visibility::global) ?
            mFed->registerGlobalCloningFilter(name) :
            mFed->registerCloningFilter(name);
        addOperations(&dfilt, type, mFed->getCorePointer().get());
        dfilt.setString("delivery", name);
        return dfilt;
    }
    auto& dfilt = (locality == interface_visibility::global) ? mFed->registerGlobalFilter(name) :
                                                               mFed->registerFilter(name);
    addOperations(&dfilt, type, nullptr);
    return dfilt;
}

std::unique_ptr<Filter> make_filter(filter_types type, Core* cr, const std::string& name)
{
    if (type == filter_types::clone) {
        std::unique_ptr<Filter> dfilt = std::make_unique<CloningFilter>(cr, name);
        addOperations(dfilt.get(), type, cr);
        dfilt->setString("delivery", name);
        return dfilt;
    }
    auto dfilt = std::make_unique<Filter>(cr, name);
    addOperations(dfilt.get(), type, cr);
    return dfilt;
}

std::unique_ptr<Filter> make_filter(filter_types type, CoreApp& cr, const std::string& name)
{
    return make_filter(type, cr.getCopyofCorePointer().get(), name);
}

CloningFilter& make_cloning_filter(filter_types type,
                                   Federate* mFed,
                                   const std::string& delivery,
                                   const std::string& name)
{
    auto& dfilt = mFed->registerCloningFilter(name);
    addOperations(&dfilt, type, mFed->getCorePointer().get());
    if (!delivery.empty()) {
        dfilt.addDeliveryEndpoint(delivery);
    }
    return dfilt;
}

CloningFilter& make_cloning_filter(interface_visibility locality,
                                   filter_types type,
                                   Federate* mFed,
                                   const std::string& delivery,
                                   const std::string& name)

{
    auto& dfilt = (locality == interface_visibility::global) ?
        mFed->registerGlobalCloningFilter(name) :
        mFed->registerCloningFilter(name);
    addOperations(&dfilt, type, mFed->getCorePointer().get());
    if (!delivery.empty()) {
        dfilt.addDeliveryEndpoint(delivery);
    }
    return dfilt;
}

std::unique_ptr<CloningFilter> make_cloning_filter(filter_types type,
                                                   Core* cr,
                                                   const std::string& delivery,
                                                   const std::string& name)

{
    auto dfilt = std::make_unique<CloningFilter>(cr, name);
    addOperations(dfilt.get(), type, cr);
    if (!delivery.empty()) {
        dfilt->addDeliveryEndpoint(delivery);
    }
    return dfilt;
}

std::unique_ptr<CloningFilter> make_cloning_filter(filter_types type,
                                                   CoreApp& cr,
                                                   const std::string& delivery,
                                                   const std::string& name)

{
    return make_cloning_filter(type, cr.getCopyofCorePointer().get(), delivery, name);
}

}  // namespace helics
