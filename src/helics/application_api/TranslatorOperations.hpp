/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

/** @file
file defines some common translator operations
*/

#include "../common/GuardedTypes.hpp"
#include "../core/core-data.hpp"
#include "../core/helicsTime.hpp"

#include <atomic>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace helics {
class Core;

/** class defining translator operations that converts values to a json string and vice versa*/
class JsonTranslatorOperator: public TranslatorOperator {
  public:
    /** default constructor*/
    JsonTranslatorOperator() = default;

  private:
    virtual SmallBuffer convertToValue(std::unique_ptr<Message> message) override;
    virtual std::unique_ptr<Message> convertToMessage(const SmallBuffer& value) override;
};

/** class defining translator operations that simply move the binary value data into a message and
 * vice versa*/
class BinaryTranslatorOperator: public TranslatorOperator {
  public:
    /** default constructor*/
    BinaryTranslatorOperator() = default;

  private:
    virtual SmallBuffer convertToValue(std::unique_ptr<Message> message) override;
    virtual std::unique_ptr<Message> convertToMessage(const SmallBuffer& value) override;
};

/** class defining a custom Translator operator*/
class CustomTranslatorOperator: public TranslatorOperator {
  public:
    /** default constructor*/
    CustomTranslatorOperator() = default;

    /** set the function to modify the data of the message*/
    void setToValueFunction(
        std::function<SmallBuffer(std::unique_ptr<Message> message)> userToValueFunction)
    {
        toValueFunction = std::move(userToValueFunction);
    }

    /** set the function to modify the data of the message*/
    void setToMessageFunction(
        std::function<std::unique_ptr<Message>(const SmallBuffer& value)> userToMessageFunction)
    {
        toMessageFunction = std::move(userToMessageFunction);
    }

  private:
    /** the custom operation to convert a message to a value*/
    std::function<SmallBuffer(std::unique_ptr<Message> message)> toValueFunction;
    /** the custom operation to convert a value into a message*/
    std::function<std::unique_ptr<Message>(const SmallBuffer& value)> toMessageFunction;

    virtual SmallBuffer convertToValue(std::unique_ptr<Message> message) override;
    virtual std::unique_ptr<Message> convertToMessage(const SmallBuffer& value) override;
};

/** class for managing translator operations*/
class TranslatorOperations {
  public:
    TranslatorOperations() = default;
    virtual ~TranslatorOperations() = default;
    // still figuring out if these functions have a use or not
    TranslatorOperations(const TranslatorOperations& fo) = delete;
    TranslatorOperations(TranslatorOperations&& fo) = delete;
    TranslatorOperations& operator=(const TranslatorOperations& fo) = delete;
    TranslatorOperations& operator=(TranslatorOperations&& fo) = delete;

    /** set a property on a translator
    @param property the name of the property of the translator to change
    @param val the numerical value of the property
    */
    virtual void set(std::string_view property, double val);
    /** set a string property on a translator
    @param property the name of the property of the translator to change
    @param val the numerical value of the property
    */
    virtual void setString(std::string_view property, std::string_view val);
    virtual std::shared_ptr<TranslatorOperator> getOperator() = 0;
};

/**custom operation object*/
class CustomTranslatorOperation: public TranslatorOperations {
  private:
    std::shared_ptr<TranslatorOperator> to;

  public:
    explicit CustomTranslatorOperation(std::shared_ptr<TranslatorOperator> op): to(std::move(op)) {}
    virtual std::shared_ptr<TranslatorOperator> getOperator() override { return to; }
};

/**translator for converting a message to JSON and vice versa*/
class JsonTranslatorOperation: public TranslatorOperations {
  private:
    std::shared_ptr<JsonTranslatorOperator> to;

  public:
    JsonTranslatorOperation();
    virtual std::shared_ptr<TranslatorOperator> getOperator() override
    {
        return std::static_pointer_cast<TranslatorOperator>(to);
    }
};

/** binary translator*/
class BinaryTranslatorOperation: public TranslatorOperations {
  private:
    std::shared_ptr<BinaryTranslatorOperator> to;

  public:
    /** default constructor*/
    BinaryTranslatorOperation();

    virtual std::shared_ptr<TranslatorOperator> getOperator() override
    {
        return std::static_pointer_cast<TranslatorOperator>(to);
    }
};

}  // namespace helics
