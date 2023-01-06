/*
Copyright (c) 2017-2023,
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
static const std::map<std::string_view, FilterTypes> filterTypes{
    {"clone", FilterTypes::CLONE},
    {"delay", FilterTypes::DELAY},
    {"randomdelay", FilterTypes::RANDOM_DELAY},
    {"random_delay", FilterTypes::RANDOM_DELAY},
    {"randomDelay", FilterTypes::RANDOM_DELAY},
    {"randomdrop", FilterTypes::RANDOM_DROP},
    {"random_drop", FilterTypes::RANDOM_DROP},
    {"randomDrop", FilterTypes::RANDOM_DROP},
    {"reroute", FilterTypes::REROUTE},
    {"firewall", FilterTypes::FIREWALL},
    {"custom", FilterTypes::CUSTOM}};

FilterTypes filterTypeFromString(std::string_view filterType) noexcept
{
    auto fnd = filterTypes.find(filterType);
    if (fnd != filterTypes.end()) {
        return fnd->second;
    }
    std::string nfilt{filterType};
    std::transform(nfilt.begin(), nfilt.end(), nfilt.begin(), ::tolower);
    fnd = filterTypes.find(nfilt);
    if (fnd != filterTypes.end()) {
        return fnd->second;
    }
    return FilterTypes::UNRECOGNIZED;
}

void addOperations(Filter* filt, FilterTypes type, Core* /*cptr*/)
{
    switch (type) {
        case FilterTypes::CUSTOM:
        default:
            break;
        case FilterTypes::RANDOM_DELAY: {
            auto op = std::make_shared<RandomDelayFilterOperation>();
            filt->setFilterOperations(std::move(op));
        } break;
        case FilterTypes::DELAY: {
            auto op = std::make_shared<DelayFilterOperation>();
            filt->setFilterOperations(std::move(op));
        } break;
        case FilterTypes::RANDOM_DROP: {
            auto op = std::make_shared<RandomDropFilterOperation>();
            filt->setFilterOperations(std::move(op));
        } break;
        case FilterTypes::REROUTE: {
            auto op = std::make_shared<RerouteFilterOperation>();
            filt->setFilterOperations(std::move(op));
        } break;
        case FilterTypes::CLONE: {
            auto op = std::make_shared<CloneFilterOperation>();
            filt->setFilterOperations(std::move(op));
        } break;
        case FilterTypes::FIREWALL: {
            auto op = std::make_shared<FirewallFilterOperation>();
            filt->setFilterOperations(std::move(op));
        } break;
    }
}

Filter::Filter(Federate* ffed, std::string_view filtName):
    Filter(InterfaceVisibility::LOCAL, ffed, filtName)
{
}

Filter::Filter(Federate* ffed, std::string_view filtName, InterfaceHandle ihandle):
    Interface(ffed, ihandle, filtName)
{
}

Filter::Filter(Core* core, std::string_view filtName, InterfaceHandle ihandle):
    Interface(core, ihandle, filtName)
{
}

Filter::Filter(InterfaceVisibility locality, Federate* ffed, std::string_view filtName):
    Interface(ffed, InterfaceHandle(), filtName)
{
    if (ffed != nullptr) {
        if (locality == InterfaceVisibility::GLOBAL) {
            handle = ffed->registerGlobalFilter(filtName);
        } else {
            handle = ffed->registerFilter(filtName);
        }
    }
}

Filter::Filter(Core* core, std::string_view filtName): Interface(core, InterfaceHandle(), filtName)
{
    handle = cr->registerFilter(filtName, std::string_view{}, std::string_view{});
}

void Filter::setOperator(std::shared_ptr<FilterOperator> mo)
{
    cr->setFilterOperator(handle, std::move(mo));
}

void Filter::setFilterOperations(std::shared_ptr<FilterOperations> filterOps)
{
    filtOp = std::move(filterOps);
    cr->setFilterOperator(handle, (filtOp) ? filtOp->getOperator() : nullptr);
}

void Filter::set(std::string_view property, double val)
{
    if (filtOp) {
        filtOp->set(property, val);
    }
}

void Filter::setString(std::string_view property, std::string_view val)
{
    if (filtOp) {
        filtOp->setString(property, val);
    }
}

CloningFilter::CloningFilter(Core* core, std::string_view filtName):
    Filter(core, filtName, InterfaceHandle())
{
    handle = cr->registerCloningFilter(filtName, std::string_view(), std::string_view());
    setFilterOperations(std::make_shared<CloneFilterOperation>());
}

CloningFilter::CloningFilter(Federate* ffed, std::string_view filtName):
    Filter(ffed, filtName, InterfaceHandle())
{
    if (ffed != nullptr) {
        handle = ffed->registerCloningFilter(filtName);
    }
    setFilterOperations(std::make_shared<CloneFilterOperation>());
}

CloningFilter::CloningFilter(Federate* ffed, std::string_view filtName, InterfaceHandle ihandle):
    Filter(ffed, filtName, ihandle)
{
}

CloningFilter::CloningFilter(InterfaceVisibility locality,
                             Federate* ffed,
                             std::string_view filtName):
    Filter(ffed, filtName, InterfaceHandle())
{
    if (ffed != nullptr) {
        if (locality == InterfaceVisibility::GLOBAL) {
            operator=(ffed->registerGlobalCloningFilter(filtName));
        } else {
            operator=(ffed->registerCloningFilter(filtName));
        }

        setFilterOperations(std::make_shared<CloneFilterOperation>());
    }
}

void CloningFilter::addDeliveryEndpoint(std::string_view endpoint)
{
    Filter::setString("add delivery", endpoint);
}

void CloningFilter::removeDeliveryEndpoint(std::string_view endpoint)
{
    Filter::setString("remove delivery", endpoint);
}

void CloningFilter::setString(std::string_view property, std::string_view val)
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

Filter& make_filter(FilterTypes type, Federate* mFed, std::string_view name)
{
    if (type == FilterTypes::CLONE) {
        Filter& dfilt = mFed->registerCloningFilter(name);
        addOperations(&dfilt, type, mFed->getCorePointer().get());
        dfilt.setString("delivery", name);
        return dfilt;
    }
    auto& dfilt = mFed->registerFilter(name);
    addOperations(&dfilt, type, nullptr);
    return dfilt;
}

Filter& make_filter(InterfaceVisibility locality,
                    FilterTypes type,
                    Federate* mFed,
                    std::string_view name)
{
    if (type == FilterTypes::CLONE) {
        Filter& dfilt = (locality == InterfaceVisibility::GLOBAL) ?
            mFed->registerGlobalCloningFilter(name) :
            mFed->registerCloningFilter(name);
        addOperations(&dfilt, type, mFed->getCorePointer().get());
        dfilt.setString("delivery", name);
        return dfilt;
    }
    auto& dfilt = (locality == InterfaceVisibility::GLOBAL) ? mFed->registerGlobalFilter(name) :
                                                              mFed->registerFilter(name);
    addOperations(&dfilt, type, nullptr);
    return dfilt;
}

std::unique_ptr<Filter> make_filter(FilterTypes type, Core* cr, std::string_view name)
{
    if (type == FilterTypes::CLONE) {
        std::unique_ptr<Filter> dfilt = std::make_unique<CloningFilter>(cr, name);
        addOperations(dfilt.get(), type, cr);
        dfilt->setString("delivery", name);
        return dfilt;
    }
    auto dfilt = std::make_unique<Filter>(cr, name);
    addOperations(dfilt.get(), type, cr);
    return dfilt;
}

std::unique_ptr<Filter> make_filter(FilterTypes type, CoreApp& cr, std::string_view name)
{
    return make_filter(type, cr.getCopyofCorePointer().get(), name);
}

CloningFilter& make_cloning_filter(FilterTypes type,
                                   Federate* mFed,
                                   std::string_view delivery,
                                   std::string_view name)
{
    auto& dfilt = mFed->registerCloningFilter(name);
    addOperations(&dfilt, type, mFed->getCorePointer().get());
    if (!delivery.empty()) {
        dfilt.addDeliveryEndpoint(delivery);
    }
    return dfilt;
}

CloningFilter& make_cloning_filter(InterfaceVisibility locality,
                                   FilterTypes type,
                                   Federate* mFed,
                                   std::string_view delivery,
                                   std::string_view name)

{
    auto& dfilt = (locality == InterfaceVisibility::GLOBAL) ?
        mFed->registerGlobalCloningFilter(name) :
        mFed->registerCloningFilter(name);
    addOperations(&dfilt, type, mFed->getCorePointer().get());
    if (!delivery.empty()) {
        dfilt.addDeliveryEndpoint(delivery);
    }
    return dfilt;
}

std::unique_ptr<CloningFilter> make_cloning_filter(FilterTypes type,
                                                   Core* cr,
                                                   std::string_view delivery,
                                                   std::string_view name)

{
    auto dfilt = std::make_unique<CloningFilter>(cr, name);
    addOperations(dfilt.get(), type, cr);
    if (!delivery.empty()) {
        dfilt->addDeliveryEndpoint(delivery);
    }
    return dfilt;
}

std::unique_ptr<CloningFilter> make_cloning_filter(FilterTypes type,
                                                   CoreApp& cr,
                                                   std::string_view delivery,
                                                   std::string_view name)

{
    return make_cloning_filter(type, cr.getCopyofCorePointer().get(), delivery, name);
}

}  // namespace helics
