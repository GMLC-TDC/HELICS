/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef MESSAGE_OPERATORS_H_
#define MESSAGE_OPERATORS_H_
#pragma once

#include <functional>

#include "Message.h"
#include "core/core-data.h"
/** @file
@brief define helper classes to scope filter operations
*/

namespace helics
{
/** class defining a message operator that inherits from the core filterCallback
@details the intention is to provide an application interface to access various common filter operations
this class is virtual
*/
class MessageOperator :public helics::FilterOperator
{
public:
	/** default constructor*/
	MessageOperator()=default;
private:
	virtual std::unique_ptr<Message> process(std::unique_ptr<Message> message) override = 0;
};

/** class defining an message operator that operates purely on the time aspect of a message*/
class MessageTimeOperator :public MessageOperator
{
public:
	/** default constructor*/
	MessageTimeOperator()=default;
	/** set the function to modify the time of the message in the constructor*/
	MessageTimeOperator(std::function<Time(Time)> userTimeFunction);
	/** set the function to modify the time of the message*/
	void setTimeFunction(std::function<Time(Time)> userTimeFunction);
private:
	std::function<Time(Time)> TimeFunction; //!< the function that actually does the processing
	virtual std::unique_ptr<Message> process(std::unique_ptr<Message> message) override;
};

/** class defining an message operator that operates purely on the data aspect of a message*/
class MessageDataOperator :public MessageOperator
{
public:
	/** default constructor*/
	MessageDataOperator()=default;
	/** set the function to modify the data of the message in the constructor*/
	MessageDataOperator(std::function<data_view(data_view)> userDataFunction);
	/** set the function to modify the data of the message*/
	void setDataFunction(std::function<data_view(data_view)> userDataFunction);
private:
	std::function<data_view(data_view)> dataFunction; //!< the function actually doing the processing
	virtual std::unique_ptr<Message> process(std::unique_ptr<Message> message) override;
};

/** class defining an message operator that either passes the message or not
@details  the evaluation function used should return true if the message should be allowed through 
false if it should be dropped
*/
class MessageConditionalOperator :public MessageOperator
{
public:
	/** default constructor*/
	MessageConditionalOperator() = default;
	/** set the function to modify the data of the message in the constructor*/
	MessageConditionalOperator(std::function<bool(const Message *)> userConditionalFunction);
	/** set the function to modify the data of the message*/
	void setConditionFunction(std::function<bool(const Message *)> userDataFunction);
private:
	std::function<bool(const Message *)> evalFunction; //!< the function actually doing the processing
	virtual std::unique_ptr<Message> process(std::unique_ptr<Message> message) override;
};

} //namespace helics
#endif
