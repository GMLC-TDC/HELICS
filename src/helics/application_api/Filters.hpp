/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_FILTER_H_
#define _HELICS_FILTER_H_
#pragma once

#include "../core/core.h"
#include "Federate.h"
#include "helics/helics-config.h"


namespace helics
{

class FilterOperations;

/** a set of common defined filters*/
enum defined_filter_types
{
    custom = 0,
    delay = 1,
    randomDelay = 2,
    randomDrop = 3,
    reroute = 4,

};

/** class for managing a particular filter*/
class Filter
{
  protected:
    Core *corePtr = nullptr;  //!< the Core to use
    Core::Handle id = invalid_Handle;  //!< the id as generated by the Federate
    filter_id_t fid = invalid_id_value;  //!< id for interacting with a federate
    std::shared_ptr<FilterOperations> filtOp;  //!< a class running any specific operation of the Filter
  public:
      /** default constructor*/
    Filter () = default;
    /** construct through a federate*/
    explicit Filter (Federate *fed);
    /** construct through a core object*/
    explicit Filter (Core *cr);
    /** virtual destructor*/
    virtual ~Filter () = default;

    /** set a message operator to process the message*/
    void setOperator (std::shared_ptr<FilterOperator> mo);

    
    filter_id_t getID () const { return fid; }
    Core::Handle getCoreHandle () const { return id; }

    /** get the target of the filter*/
    const std::string &getTarget () const;
    /** get the name for the filter*/
    const std::string &getName () const;
    /** get the specified input type of the filter*/
    const std::string &getInputType () const;
    /** get the specified output type of the filter*/
    const std::string &getOutputType () const;
protected:
    /** set a filter operations object */
    void setFilterOperations(std::shared_ptr<FilterOperations> filterOps);
    friend void addOperations(Filter *filt, defined_filter_types type);
};

/** class wrapping a source filter*/
class SourceFilter : public Filter
{
  public:
    /**constructor to build an source filter object
    @param[in] fed  the Federate to use
    @param[in] target the endpoint the filter is targeting
    @param[in] name the name of the filter
    @param[in] input_type the type of data the filter is expecting
    @param[in] output_type the type of data the filter is generating
    */
    SourceFilter (Federate *fed,
                  const std::string &target,
                  const std::string &name = "",
                  const std::string &input_type = "",
                  const std::string &output_type = "");
    /**constructor to build an source filter object
    @param[in] fed  the Federate to use
    @param[in] target the endpoint the filter is targeting
    @param[in] name the name of the filter
    @param[in] input_type the type of data the filter is expecting
    @param[in] output_type the type of data the filter is generating
    */
    SourceFilter (Core *cr,
                  const std::string &target,
                  const std::string &name = "",
                  const std::string &input_type = "",
                  const std::string &output_type = "");
    virtual ~SourceFilter () = default;
};

/** class wrapping a destination filter*/
class DestinationFilter : public Filter
{
  public:
    /**constructor to build an destination filter object
    @param[in] fed  the MessageFederate to use
    @param[in] target the endpoint the filter is targeting
    @param[in] name the name of the filter
    @param[in] input_type the type of data the filter is expecting
    @param[in] output_type the type of data the filter is generating
    */
    DestinationFilter (Federate *fed,
                       const std::string &target,
                       const std::string &name = "",
                       const std::string &input_type = "",
                       const std::string &output_type = "");
    /**constructor to build an destination filter object
    @param[in] cr  the Core to register the filter with
    @param[in] target the endpoint the filter is targeting
    @param[in] name the name of the filter
    @param[in] input_type the type of data the filter is expecting
    @param[in] output_type the type of data the filter is generating
    */
    DestinationFilter (Core *cr,
                       const std::string &target,
                       const std::string &name = "",
                       const std::string &input_type = "",
                       const std::string &output_type = "");
    virtual ~DestinationFilter () = default;
};

/** class used to clone message for delivery to other endpoints*/
class cloningFilter : public Filter
{
  public:
    explicit cloningFilter (Core *cr);
    explicit cloningFilter (Federate *fed);

    void addSourceEndpoint (const std::string &sourceName);
    void addDestinationEndpoint (const std::string &destinationName);
    void addDeliveryEndpoint (const std::string &endpoint);
private:

};


/** create a destination filter
@param type the type of filter to create
@param fed the federate to create the filter through
@param target the target endpoint all message with the specified target as a destination will route through the filter
@param name the name of the filter (optional)
@return a unique pointer to a destination Filter object,  note destroying the object does not deactivate the filter
*/
std::unique_ptr<DestinationFilter> make_destination_filter (defined_filter_types type,
                                                            Federate *fed,
                                                            const std::string &target,
                                                            const std::string &name = "");
/** create a source filter
@param type the type of filter to create
@param fed the federate to create the filter through
@param target the target endpoint all message coming from the specified source will route through the filter
@param name the name of the filter (optional)
@return a unique pointer to a source Filter object,  note destroying the object does not deactivate the filter
*/
std::unique_ptr<SourceFilter> make_source_filter (defined_filter_types type,
                                                  Federate *fed,
                                                  const std::string &target,
                                                  const std::string &name = "");

/** create a destination filter
@param type the type of filter to create
@param cr the core to create the federate through
@param target the target endpoint all message with the specified target as a destination will route through the filter
@param name the name of the filter (optional)
@return a unique pointer to a destination Filter object,  note destroying the object does not deactivate the filter
*/
std::unique_ptr<DestinationFilter> make_destination_filter (defined_filter_types type,
                                                            Core *cr,
                                                            const std::string &target,
                                                            const std::string &name = "");

/** create a source filter
@param type the type of filter to create
@param cr the core to create the filter through
@param target the target endpoint all message coming from the specified source will route through the filter
@param name the name of the filter (optional)
@return a unique pointer to a source Filter object,  note destroying the object does not deactivate the filter
*/
std::unique_ptr<SourceFilter>
make_source_filter (defined_filter_types type, Core *cr, const std::string &target, const std::string &name = "");

}  // namespace helics
#endif
