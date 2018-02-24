/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

#pragma once

#include <functional>

#include "../core/core-data.hpp"
#include "Message.hpp"
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
    explicit MessageDestOperator (std::function<std::string (const std::string &)> userDestFunction);
    /** set the function to modify the time of the message*/
    void setDestFunction (std::function<std::string (const std::string &)> userDestFunction);

  private:
    std::function<std::string (const std::string &)>
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
    CloneOperator (std::function<void(const Message *)> userCloneFunction);
    /** set the function to modify the data of the message*/
    void setCloneFunction (std::function<void(const Message *)> userCloneFunction);

  private:
    std::function<void(const Message *)> evalFunction;  //!< the function actually doing the processing
    virtual std::unique_ptr<Message> process (std::unique_ptr<Message> message) override;
};

}  // namespace helics
