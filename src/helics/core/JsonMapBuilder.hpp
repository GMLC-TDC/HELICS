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
class JsonMapBuilder
{
  private:
    std::unique_ptr<Json::Value> jMap;
    std::map<int, std::string> missing_components;

  public:
    JsonMapBuilder () = default;
    Json::Value &getJValue ();
    bool isCompleted () const;
    // check whether a map is currently completed or under construction
    bool isActive () const { return static_cast<bool> (jMap); }
    bool addComponent (const std::string &info, int index);
    int generatePlaceHolder (const std::string &location);
    std::string generate ();
    void reset ();
};
}  // namespace helics
