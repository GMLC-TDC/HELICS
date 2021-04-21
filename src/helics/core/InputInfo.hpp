/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "basic_core_types.hpp"

#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace helics {
/** data class for managing information about a subscription*/
class InputInfo {
  public:
    /** data structure containing a helics data value recorded from a publication*/
    struct dataRecord {
        Time time{Time::minVal()};  //!< the time of the data value
        unsigned int iteration{0};  //!< the iteration number of the data value
        std::shared_ptr<const data_block> data;  //!< the data value
        /** default constructor*/
        dataRecord() = default;
        dataRecord(Time recordTime, std::shared_ptr<const data_block> recordData):
            time(recordTime), data(std::move(recordData))
        {
        }
        dataRecord(Time recordTime,
                   unsigned int recordIteration,
                   std::shared_ptr<const data_block> recordData):
            time(recordTime),
            iteration(recordIteration), data(std::move(recordData))
        {
        }
    };

    struct sourceInformation {
        std::string key;
        std::string type;
        std::string units;
        sourceInformation() = default;
        sourceInformation(const std::string& key_,
                          const std::string& type_,
                          const std::string& units_):
            key(key_),
            type(type_), units(units_)
        {
        }
    };
    /** constructor with all the information*/
    InputInfo(global_handle handle,
              const std::string& key_,
              const std::string& type_,
              const std::string& units_):
        id(handle),
        key(key_), type(type_), units(units_)
    {
    }

    const global_handle id;  //!< identifier for the handle
    const std::string key;  //!< the identifier for the input
    const std::string type;  //! the nominal type of data for the input
    const std::string units;  //!< the units of the controlInput
    bool required{
        false};  //!< flag indicating that the subscription requires a matching publication
    bool optional{false};  //!< flag indicating that any targets are optional
    bool has_target{false};  //!< flag indicating that the input has a source
    bool only_update_on_change{
        false};  //!< flag indicating that the data should only be updated on change
    bool not_interruptible{
        false};  //!< indicator that this handle should not be used for interrupting
    bool strict_type_matching{
        false};  //!< indicator that the handle need to have strict type matching
    bool ignore_unit_mismatch{false};  //!< ignore unit mismatches
    int32_t required_connnections{0};  //!< an exact number of connections required
    std::vector<std::pair<helics::Time, unsigned int>>
        current_data_time;  //!< the most recent published data times
    std::vector<std::shared_ptr<const data_block>>
        current_data;  //!< the most recent published data
    std::vector<global_handle> input_sources;  //!< the sources of the input signals
    std::vector<Time> deactivated;  //!< indicator that the source has been deactivated
    std::vector<sourceInformation> source_info;  //!< the name,type,units of the sources
    std::vector<int32_t> priority_sources;  //!< the list of priority inputs;
  private:
    std::vector<std::vector<dataRecord>> data_queues;  //!< queue of the data

  public:
    /** get all the current data*/
    const std::vector<std::shared_ptr<const data_block>>& getAllData() const;
    /** get a particular data input*/
    const std::shared_ptr<const data_block>& getData(int index) const;
    /** get a the most recent data point*/
    const std::shared_ptr<const data_block>& getData(uint32_t* inputIndex) const;
    /** add a data block into the queue*/
    void addData(global_handle source_id,
                 Time valueTime,
                 unsigned int iteration,
                 std::shared_ptr<const data_block> data);

    /** update current data not including data at the specified time
    @param newTime the time to move the subscription to
    @return true if the value has changed
    */
    bool updateTimeUpTo(Time newTime);
    /** update current data to all new data at newTime
    @param newTime the time to move the subscription to
    @return true if the value has changed
    */
    bool updateTimeInclusive(Time newTime);

    /** update current data to get all data through the first iteration at newTime
    @param newTime the time to move the subscription to
    @return true if the value has changed
    */
    bool updateTimeNextIteration(Time newTime);
    /** get the event based on the event queue*/
    Time nextValueTime() const;
    /** add a new source target to the input
    @return true if the source was added false if duplicate
    */
    bool addSource(global_handle newSource,
                   const std::string& sourceName,
                   const std::string& stype,
                   const std::string& sunits);
    /** remove a source */
    void removeSource(global_handle sourceToRemove, Time minTime);
    /** remove a source */
    void removeSource(const std::string& sourceName, Time minTime);
    /** clear all non-current data*/
    void clearFutureData();

    const std::string& getInjectionType() const;
    const std::string& getInjectionUnits() const;
    /** get the name of the source given a global id*/
    const std::string& getSourceName(global_handle source) const;

  private:
    bool updateData(dataRecord&& update, int index);
    mutable std::string inputUnits;
    mutable std::string inputType;
};

bool checkTypeMatch(const std::string& type1, const std::string& type2, bool strict_match);

bool checkUnitMatch(const std::string& unit1, const std::string& unit2, bool strict_match);
}  // namespace helics
