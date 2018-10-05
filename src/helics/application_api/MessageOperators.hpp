/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include <atomic>
#include <functional>

#include "../core/core-data.hpp"
#include "data_view.hpp"
/** @file
@brief define helper classes to scope filter operations
*/

namespace helics
{
/** class defining an message operator that operates purely on the time aspect of a message*/
class MessageTimeOperator : public FilterOperator
{
  public:
    /** default constructor*/
    MessageTimeOperator () = default;
    /** set the function to modify the time of the message in the constructor*/
    explicit MessageTimeOperator (std::function<Time (Time)> userTimeFunction);
    /** set the function to modify the time of the message*/
    void setTimeFunction (std::function<Time (Time)> userTimeFunction);

  private:
    std::function<Time (Time)> TimeFunction;  //!< the function that actually does the processing
    virtual std::unique_ptr<Message> process (std::unique_ptr<Message> message) override;
};

/** class defining an message operator that operates purely on the destination aspect of a message*/
class MessageDestOperator : public FilterOperator
{
  public:
    /** default constructor*/
    MessageDestOperator () = default;
    /** set the function to modify the time of the message in the constructor*/
    explicit MessageDestOperator (
      std::function<std::string (const std::string &, const std::string &)> userDestFunction);
    /** set the function to modify the time of the message*/
    void setDestFunction (std::function<std::string (const std::string &, const std::string &)> userDestFunction);

  private:
    std::function<std::string (const std::string &, const std::string &)>
      DestUpdateFunction;  //!< the function that actually does the processing
    virtual std::unique_ptr<Message> process (std::unique_ptr<Message> message) override;
};

/** class defining an message operator that operates purely on the data aspect of a message*/
class MessageDataOperator : public FilterOperator
{
  public:
    /** default constructor*/
    MessageDataOperator () = default;
    /** set the function to modify the data of the message in the constructor*/
    explicit MessageDataOperator (std::function<data_view (data_view)> userDataFunction);
    /** set the function to modify the data of the message*/
    void setDataFunction (std::function<data_view (data_view)> userDataFunction);

  private:
    std::function<data_view (data_view)> dataFunction;  //!< the function actually doing the processing
    virtual std::unique_ptr<Message> process (std::unique_ptr<Message> message) override;
};

/** class defining an message operator that either passes the message or not
@details  the evaluation function used should return true if the message should be allowed through
false if it should be dropped
*/
class MessageConditionalOperator : public FilterOperator
{
  public:
    /** default constructor*/
    MessageConditionalOperator () = default;
    /** set the function to modify the data of the message in the constructor*/
    explicit MessageConditionalOperator (std::function<bool(const Message *)> userConditionalFunction);
    /** set the function to modify the data of the message*/
    void setConditionFunction (std::function<bool(const Message *)> userConditionalFunction);

  private:
    std::function<bool(const Message *)> evalFunction;  //!< the function actually doing the processing
    virtual std::unique_ptr<Message> process (std::unique_ptr<Message> message) override;
};

/** class defining an message operator that either passes the message or not
@details  the evaluation function used should return true if the message should be allowed through
false if it should be dropped
*/
class CloneOperator : public FilterOperator
{
  public:
    /** default constructor*/
    CloneOperator () = default;
    /** set the function to modify the data of the message in the constructor*/
    explicit CloneOperator (std::function<void(const Message *)> userCloneFunction);
    /** set the function to modify the data of the message*/
    void setCloneFunction (std::function<void(const Message *)> userCloneFunction);

  private:
    std::function<void(const Message *)> evalFunction;  //!< the function actually doing the processing
    virtual std::unique_ptr<Message> process (std::unique_ptr<Message> message) override;
};

/** class defining an message operator that either passes the message or not
@details  the evaluation function used should return true if the message should be allowed through
false if it should be dropped
*/
class FirewallOperator : public FilterOperator
{
  public:
    enum class operations
    {
        drop = 0,
        pass = 1,
        setFlag1 = 2,
        setFlag2 = 3,
        setFlag3 = 4,
        setFlag4 = 5
    };
    /** default constructor*/
    FirewallOperator () = default;
    /** set the function to modify the data of the message in the constructor*/
    explicit FirewallOperator (std::function<bool(const Message *)> userCheckFunction);
    /** set the function to modify the data of the message*/
    void setCheckFunction (std::function<bool(const Message *)> userCheckFunction);
    /** set the operation to perform on positive checkFunction*/
    void setOperation (operations newop) { operation.store (newop); }

  private:
    std::function<bool(const Message *)> checkFunction;  //!< the function actually doing the processing
    std::atomic<operations> operation;
    virtual std::unique_ptr<Message> process (std::unique_ptr<Message> message) override;
};

}  // namespace helics
