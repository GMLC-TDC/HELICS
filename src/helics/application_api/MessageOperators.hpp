/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../core/core-data.hpp"
#include "data_view.hpp"
#include "helics_cxx_export.h"

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <vector>
/** @file
@brief define helper classes to scope filter operations
*/

namespace helics {
/** class defining an message operator that operates purely on the time aspect of a message*/
class HELICS_CXX_EXPORT MessageTimeOperator: public FilterOperator {
  public:
    /** default constructor*/
    MessageTimeOperator() = default;
    /** set the function to modify the time of the message in the constructor*/
    explicit MessageTimeOperator(std::function<Time(Time)> userTimeFunction);
    /** set the function to modify the time of the message*/
    void setTimeFunction(std::function<Time(Time)> userTimeFunction);

  private:
    std::function<Time(Time)> TimeFunction;  //!< the function that actually does the processing
    virtual std::unique_ptr<Message> process(std::unique_ptr<Message> message) override;
};

/** class defining an message operator that operates purely on the destination aspect of a message*/
class HELICS_CXX_EXPORT MessageDestOperator: public FilterOperator {
  public:
    /** default constructor*/
    MessageDestOperator() = default;
    /** set the function to modify the time of the message in the constructor*/
    explicit MessageDestOperator(
        std::function<std::string(const std::string&, const std::string&)> userDestFunction);
    /** set the function to modify the time of the message*/
    void setDestFunction(
        std::function<std::string(const std::string&, const std::string&)> userDestFunction);
    virtual bool isMessageGenerating() const override { return true; }

  private:
    std::function<std::string(const std::string&, const std::string&)>
        DestUpdateFunction;  //!< the function that actually does the processing
    virtual std::unique_ptr<Message> process(std::unique_ptr<Message> message) override;
};

/** class defining an message operator that operates purely on the data aspect of a message*/
class HELICS_CXX_EXPORT MessageDataOperator: public FilterOperator {
  public:
    /** default constructor*/
    MessageDataOperator() = default;
    /** set the function to modify the data of the message in the constructor*/
    explicit MessageDataOperator(std::function<void(data_block&)> userDataFunction);
    /** set the function to modify the data of the message*/
    void setDataFunction(std::function<void(data_block&)> userDataFunction);

  private:
    std::function<void(data_block&)> dataFunction;  //!< the function actually doing the processing
    virtual std::unique_ptr<Message> process(std::unique_ptr<Message> message) override;
};

/** class defining an message operator that either passes the message or not
@details  the evaluation function used should return true if the message should be allowed through
false if it should be dropped
*/
class HELICS_CXX_EXPORT MessageConditionalOperator: public FilterOperator {
  public:
    /** default constructor*/
    MessageConditionalOperator() = default;
    /** set the function to modify the data of the message in the constructor*/
    explicit MessageConditionalOperator(
        std::function<bool(const Message*)> userConditionalFunction);
    /** set the function to modify the data of the message*/
    void setConditionFunction(std::function<bool(const Message*)> userConditionFunction);

  private:
    std::function<bool(const Message*)>
        evalFunction;  //!< the function actually doing the processing
    virtual std::unique_ptr<Message> process(std::unique_ptr<Message> message) override;
};

/** class defining an message operator that either passes the message or not
@details  the evaluation function used should return true if the message should be allowed through
false if it should be dropped
*/
class HELICS_CXX_EXPORT CloneOperator: public FilterOperator {
  public:
    /** default constructor*/
    CloneOperator() = default;
    /** set the function to modify the data of the message in the constructor*/
    explicit CloneOperator(
        std::function<std::vector<std::unique_ptr<Message>>(const Message*)> userCloneFunction);
    /** set the function to modify the data of the message*/
    void setCloneFunction(
        std::function<std::vector<std::unique_ptr<Message>>(const Message*)> userCloneFunction);
    virtual bool isMessageGenerating() const override { return true; }

  private:
    std::function<std::vector<std::unique_ptr<Message>>(const Message*)>
        evalFunction;  //!< the function actually doing the processing
    virtual std::unique_ptr<Message> process(std::unique_ptr<Message> message) override;
    virtual std::vector<std::unique_ptr<Message>>
        processVector(std::unique_ptr<Message> message) override;
};

/** class defining an message operator that either passes the message or not
@details  the evaluation function used should return true if the message should be allowed through
false if it should be dropped
*/
class HELICS_CXX_EXPORT FirewallOperator: public FilterOperator {
  public:
    /** enumeration of the possible operations of the firewall*/
    enum class operations : int {
        none = -1,
        drop = 0,
        pass = 1,
        set_flag1 = 2,
        set_flag2 = 3,
        set_flag3 = 4
    };
    /** default constructor*/
    FirewallOperator() = default;
    /** set the function to modify the data of the message in the constructor*/
    explicit FirewallOperator(std::function<bool(const Message*)> userCheckFunction);
    /** set the function to modify the data of the message*/
    void setCheckFunction(std::function<bool(const Message*)> userCheckFunction);
    /** set the operation to perform on positive checkFunction*/
    void setOperation(operations newop) { operation.store(newop); }

  private:
    std::function<bool(const Message*)>
        checkFunction;  //!< the function actually doing the processing
    std::atomic<operations> operation{
        operations::drop};  //!< the operation to perform if the firewall triggers
    virtual std::unique_ptr<Message> process(std::unique_ptr<Message> message) override;
};

/** class defining a message operator that can operate on any part of a message*/
class HELICS_CXX_EXPORT CustomMessageOperator: public FilterOperator {
  public:
    /** default constructor*/
    CustomMessageOperator() = default;
    /** set the function to modify the data of the message in the constructor*/
    explicit CustomMessageOperator(
        std::function<std::unique_ptr<Message>(std::unique_ptr<Message>)> userMessageFunction);
    /** set the function to modify the data of the message*/
    void setMessageFunction(
        std::function<std::unique_ptr<Message>(std::unique_ptr<Message>)> userMessageFunction);

  private:
    std::function<std::unique_ptr<Message>(std::unique_ptr<Message>)>
        messageFunction;  //!< the function actually doing the processing
    virtual std::unique_ptr<Message> process(std::unique_ptr<Message> message) override;
};
}  // namespace helics
