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

namespace helics
{
/** class defining a message operator that inherits from the core filterCallback
@details the intention is to provide an application interface to access various common filter operations
this class is virtual
*/
class MessageOperator :public helics::FilterOperator
{
public:
	MessageOperator() {};
private:
	virtual message_t operator() (message_t *) override = 0;
};

class MessageTimeOperator :public MessageOperator
{
public:
	MessageTimeOperator() {};
	MessageTimeOperator(std::function<Time(Time)> userTimeFunction);
	void setTimeFunction(std::function<Time(Time)> userTimeFunction);
private:
	std::function<Time(Time)> TimeFunction;
	virtual message_t operator() (message_t *) override;
};

class MessageDataOperator :public MessageOperator
{
public:
	MessageDataOperator() {};
	MessageDataOperator(std::function<data_view(data_view)> userDataFunction);
	void setDataFunction(std::function<data_view(data_view)> userDataFunction);
private:
	std::function<data_view(data_view)> dataFunction;
	virtual message_t operator() (message_t *) override;
};

}
#endif
