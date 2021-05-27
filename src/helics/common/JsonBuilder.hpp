/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Json {
class Value;
}  // namespace Json

namespace helics {
/** class handling the construction in pieces of a JSON map*/
class JsonMapBuilder {
  private:
    std::unique_ptr<Json::Value> jMap;
    std::map<int, std::pair<std::string, int32_t>> missing_components;
    int counterCode{0};  // a code for the user to include for various purposes
  public:
    JsonMapBuilder() noexcept;
    ~JsonMapBuilder();
    JsonMapBuilder(JsonMapBuilder&& map) = default;
    JsonMapBuilder& operator=(JsonMapBuilder&& map) = default;
    /** get the underlying json object*/
    Json::Value& getJValue();
    /** check if the map has completed*/
    bool isCompleted() const;
    // check whether a map is currently completed or under construction
    bool isActive() const { return static_cast<bool>(jMap); }
    /** add a component value for a previously generated location
    @param info the string to use for information
    @param index the index of the place holder
    @return true if successfully added
    */
    bool addComponent(const std::string& info, int index) noexcept;
    /** generate a new location to fill in later
    @return the index value of the location for use in addComponent*/
    int generatePlaceHolder(const std::string& location, int32_t code);
    /** clear components with a specific code
    return true if the map is now complete*/
    bool clearComponents(int32_t code);
    /** clear all remaining components
    return true if the builder can be generated*/
    bool clearComponents();
    /** generate the JSON value*/
    std::string generate();
    /** reset the builder*/
    void reset();
    /** set the counter code value*/
    void setCounterCode(int code) { counterCode = code; }
    int getCounterCode() const { return counterCode; }
};

/** class to help with the generation of JSON*/
class JsonBuilder {
  private:
    std::unique_ptr<Json::Value> jMap;

  public:
    JsonBuilder() noexcept;
    ~JsonBuilder();
    /** get the underlying json object*/
    Json::Value& getJValue();
    /** add a string element on a specific path*/
    void addElement(const std::string& path, const std::string& value);
    /** add a double element on a specific path*/
    void addElement(const std::string& path, double value);
    /** add a vector element on a specific path*/
    void addElement(const std::string& path, const std::vector<double>& value);
    /** generate the JSON value*/
    std::string generate();
    /** reset the builder*/
    void reset();
};
}  // namespace helics
