/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "TranslatorOperations.hpp"

#include "../core/Core.hpp"
#include "../core/core-exceptions.hpp"
#include "../utilities/timeStringOps.hpp"
#include "HelicsPrimaryTypes.hpp"

namespace helics {


    SmallBuffer JsonTranslatorOperator::convertToValue(std::unique_ptr<Message> message)
    {
        
        defV val;
        val= readJsonValue(message->data.to_string());
        return typeConvertDefV(val);
    }

/** convert a value to a message*/
    std::unique_ptr<Message> JsonTranslatorOperator::convertToMessage(const SmallBuffer& value) {
        defV val;
        valueExtract(value, DataType::HELICS_ANY, val);
       valueConvert(val,DataType::HELICS_JSON);
        auto m = std::make_unique<Message>();
       m->data = std::get<std::string>(val);
        return m;
    }


    
    SmallBuffer BinaryTranslatorOperator::convertToValue(std::unique_ptr<Message> message)
    {
        SmallBuffer res;
        res = message->data;
        return res;
    }

    /** convert a value to a message*/
    std::unique_ptr<Message> BinaryTranslatorOperator::convertToMessage(const SmallBuffer& value)
    {
        auto m = std::make_unique<Message>();
        m->data = value;
        return m;
    }

    void TranslatorOperations::set(const std::string& property, double /*val*/)
{
    if (property == "delay") {
    } else if (property == "inputdelay") {

    } else if (property == "outputdelay") {

    }
}

    void TranslatorOperations::setString(const std::string& property, const std::string& val)
    {
        if (property == "delay") {
        } else if (property == "inputdelay") {
        } else if (property == "outputdelay") {
        }
    }

JsonTranslatorOperation::JsonTranslatorOperation(): to(std::make_shared<JsonTranslatorOperator>())
{
    
}

BinaryTranslatorOperation::BinaryTranslatorOperation(): to(std::make_shared<BinaryTranslatorOperator>())
{
}



}  // namespace helics
