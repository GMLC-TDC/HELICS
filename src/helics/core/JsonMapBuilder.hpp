/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once
#include <map>
#include <memory>

namespace helics
{
namespace Json
{
class Value;
}  // namespace Json

/** class handling the construction in pieces of a JSON map*/
class JsonMapBuilder
{
  private:
    std::unique_ptr<Json::Value> jMap;
    std::map<int, std::string> missing_components;

  public:
    JsonMapBuilder () = default;
    /** get the underlying json object*/
    Json::Value &getJValue ();
    /** check if the map has completed*/
    bool isCompleted () const;
    // check whether a map is currently completed or under construction
    bool isActive () const { return static_cast<bool> (jMap); }
    /** add a component value for a previously generated location
    @param info the string to use for information
    @param index the index of the place holder
    @return true if successfully added
    */
    bool addComponent (const std::string &info, int index);
    /** generate a new location to fill in later
    @return the index value of the location for use in addComponent*/
    int generatePlaceHolder (const std::string &location);
    /** generate the JSON value*/
    std::string generate ();
    /** reset the builder*/
    void reset ();
};
}  // namespace helics
