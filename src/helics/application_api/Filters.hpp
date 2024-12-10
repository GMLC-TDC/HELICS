/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../core/Core.hpp"
#include "Federate.hpp"
#include "helics/helics_enums.h"

#include <memory>
#include <string>

namespace helics {
class FilterOperations;
class FilterOperator;
class CoreApp;
/** a set of common defined filters*/
enum class FilterTypes {
    CUSTOM = HELICS_FILTER_TYPE_CUSTOM,
    DELAY = HELICS_FILTER_TYPE_DELAY,
    RANDOM_DELAY = HELICS_FILTER_TYPE_RANDOM_DELAY,
    RANDOM_DROP = HELICS_FILTER_TYPE_RANDOM_DROP,
    REROUTE = HELICS_FILTER_TYPE_REROUTE,
    CLONE = HELICS_FILTER_TYPE_CLONE,
    FIREWALL = HELICS_FILTER_TYPE_FIREWALL,
    UNRECOGNIZED = 7

};

#define EMPTY_STRING std::string_view()

/** get the filter type from a string*/
HELICS_CXX_EXPORT FilterTypes filterTypeFromString(std::string_view filterType) noexcept;

/** class for managing a particular filter*/
class HELICS_CXX_EXPORT Filter: public Interface {
  protected:
    bool cloning = false;
    bool disableAssign = false;  //!< disable assignment for the object
  private:
    std::shared_ptr<FilterOperations>
        filtOp;  //!< a class running any specific operation of the Filter
  public:
    /** default constructor*/
    Filter() = default;
    /** construct through a federate*/
    explicit Filter(Federate* ffed, std::string_view filtName = EMPTY_STRING);
    /** construct from handle and federate*/
    Filter(Federate* ffed, std::string_view filtName, InterfaceHandle ihandle);
    /** construct from handle and core*/
    Filter(Core* core, std::string_view filtName, InterfaceHandle ihandle);
    /** construct through a federate*/
    Filter(InterfaceVisibility locality, Federate* ffed, std::string_view filtName = EMPTY_STRING);
    /** construct through a core object*/
    explicit Filter(Core* core, std::string_view filtName = EMPTY_STRING);
    /** virtual destructor*/
    virtual ~Filter() = default;

    Filter(Filter&& filt) = default;
    /** copy the filter, a copied filter will point to the same object*/
    Filter(const Filter& filt) = default;
    Filter& operator=(Filter&& filt) = default;
    /** copy the filter, a copied filter will point to the same object as the original*/
    Filter& operator=(const Filter& filt) = default;
    /** check if the filter is a cloning filter*/
    bool isCloningFilter() const { return cloning; }
    /** set a message operator to process the message*/
    void setOperator(std::shared_ptr<FilterOperator> filterOp);

    virtual const std::string& getDisplayName() const override { return getName(); }

    /** set a property on a filter
    @param property the name of the property of the filter to change
    @param val the numerical value of the property
    */
    virtual void set(std::string_view property, double val);

    /** set a string property on a filter
    @param property the name of the property of the filter to change
    @param val the numerical value of the property
    */
    virtual void setString(std::string_view property, std::string_view val);

    /** get a string property on a filter
    @param property the name of the property of the filter to get
    */
    virtual std::string getString(std::string_view property);

    /** get a double valued property of a filter
    @param property the name of the property of the filter to retrieve
    */
    virtual double getProperty(std::string_view property);

    /** add a target to the filter*/
    void addTarget(std::string_view target) { addSourceTarget(target); }

    /** set the type of operations specifying how the filter should operate*/
    void setFilterType(std::int32_t type);

  protected:
    /** set a filter operations object */
    void setFilterOperations(std::shared_ptr<FilterOperations> filterOps);
    /** add a defined operation to a filter*/
    friend void addOperations(Filter* filt, FilterTypes type, Core* cptr);
};

/** class used to clone message for delivery to other endpoints*/
class HELICS_CXX_EXPORT CloningFilter: public Filter {
  public:
    /** default constructor*/
    CloningFilter() = default;
    /** construct from a core object
     */
    explicit CloningFilter(Core* core, std::string_view filtName = EMPTY_STRING);
    /** construct from a Federate
     */
    explicit CloningFilter(Federate* ffed, std::string_view filtName = EMPTY_STRING);
    /** construct from a Federate
     */
    CloningFilter(InterfaceVisibility locality,
                  Federate* ffed,
                  std::string_view filtName = EMPTY_STRING);

    /** constructor used by FilterFederateManager*/
    CloningFilter(Federate* ffed, std::string_view filtName, InterfaceHandle handle);
    /** move the filter to a new cloning filter*/
    CloningFilter(CloningFilter&& filt) = default;
    /** copy the filter, a copied filter will point to the same object*/
    CloningFilter(const CloningFilter& filt) = default;
    /** move assign the cloning filter*/
    CloningFilter& operator=(CloningFilter&& filt) = default;
    /** copy the filter, a copied filter will point to the same object as the original*/
    CloningFilter& operator=(const CloningFilter& filt) = default;
    /** destructor */
    ~CloningFilter() = default;
    /** add a delivery address this is the name of an endpoint to deliver the message to*/
    void addDeliveryEndpoint(std::string_view endpoint);

    /** remove a delivery address this is the name of an endpoint to deliver the message to*/
    void removeDeliveryEndpoint(std::string_view endpoint);

    virtual void setString(std::string_view property, std::string_view val) override;

  private:
    friend class FilterFederateManager;
};

/** create a  filter
@param type the type of filter to create
@param fed the federate to create the filter through
@param name the name of the filter (optional)
@return a unique pointer to a destination Filter object,  note destroying the object does not
deactivate the filter
*/
HELICS_CXX_EXPORT Filter&
    make_filter(FilterTypes type, Federate* fed, std::string_view name = EMPTY_STRING);

/** create a  filter
@param locality the visibility of the filter global or local
@param type the type of filter to create
@param fed the federate to create the filter through
@param name the name of the filter (optional)
@return a unique pointer to a destination Filter object,  note destroying the object does not
deactivate the filter
*/
HELICS_CXX_EXPORT Filter& make_filter(InterfaceVisibility locality,
                                      FilterTypes type,
                                      Federate* fed,
                                      std::string_view name = EMPTY_STRING);

/** create a filter
@param type the type of filter to create
@param core the core to create the filter through
@param name the name of the filter (optional)
@return a unique pointer to a source Filter object,  note destroying the object does not deactivate
the filter
*/
HELICS_CXX_EXPORT std::unique_ptr<Filter>
    make_filter(FilterTypes type, Core* core, std::string_view name = EMPTY_STRING);

/** create a filter
@param type the type of filter to create
@param core the core to create the filter through
@param name the name of the filter (optional)
@return a unique pointer to a source Filter object,  note destroying the object does not deactivate
the filter
*/
HELICS_CXX_EXPORT std::unique_ptr<Filter>
    make_filter(FilterTypes type, CoreApp& core, std::string_view name = EMPTY_STRING);

/** create a  filter
@param type the type of filter to create
@param fed the federate to create the filter through
@param delivery the endpoint to deliver the cloned message to
@param name the name of the filter (optional)
@return a unique pointer to a destination Filter object,  note destroying the object does not
deactivate the filter
*/
HELICS_CXX_EXPORT CloningFilter& make_cloning_filter(FilterTypes type,
                                                     Federate* fed,
                                                     std::string_view delivery,
                                                     std::string_view name = EMPTY_STRING);

/** create a cloning filter with a specified visibility
@param locality can be global or local
@param type the type of filter to create
@param fed the federate to create the filter through
@param delivery the endpoint to deliver the cloned message to
@param name the name of the filter (optional)
@return a unique pointer to a destination Filter object,  note destroying the object does not
deactivate the filter
*/
HELICS_CXX_EXPORT CloningFilter& make_cloning_filter(InterfaceVisibility locality,
                                                     FilterTypes type,
                                                     Federate* fed,
                                                     std::string_view delivery,
                                                     std::string_view name = EMPTY_STRING);

/** create a cloning filter with a delivery location
@param type the type of filter to create
@param core the core to create the filter through
@param delivery the endpoint to deliver the cloned message to
@param name the name of the filter (optional)
@return a unique pointer to a source Filter object,  note destroying the object does not deactivate
the filter
*/
HELICS_CXX_EXPORT std::unique_ptr<CloningFilter>
    make_cloning_filter(FilterTypes type,
                        Core* core,
                        std::string_view delivery,
                        std::string_view name = EMPTY_STRING);

/** create a cloning filter with a delivery location
@param type the type of filter to create
@param core the core to create the filter through
@param delivery the endpoint to deliver the cloned message to
@param name the name of the filter (optional)
@return a unique pointer to a source Filter object,  note destroying the object does not deactivate
the filter
*/
HELICS_CXX_EXPORT std::unique_ptr<CloningFilter>
    make_cloning_filter(FilterTypes type,
                        CoreApp& core,
                        std::string_view delivery,
                        std::string_view name = EMPTY_STRING);

}  // namespace helics
