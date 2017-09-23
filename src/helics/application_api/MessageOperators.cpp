/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "MessageOperators.h"

namespace helics
{



	MessageTimeOperator::MessageTimeOperator(std::function<Time(Time)> userTimeFunction) :TimeFunction(std::move(userTimeFunction))
{

}

std::unique_ptr<Message> MessageTimeOperator::process(std::unique_ptr<Message> message)
{
	if (TimeFunction)
	{
		message->time = TimeFunction(message->time);
	}
	return message;
}

void MessageTimeOperator::setTimeFunction(std::function<Time(Time)> userTimeFunction)
{
	TimeFunction = userTimeFunction;
}

MessageDataOperator::MessageDataOperator(std::function<data_view(data_view)> userDataFunction)
{

}

void MessageDataOperator::setDataFunction(std::function<data_view(data_view)> userDataFunction)
{
	dataFunction = userDataFunction;
}

std::unique_ptr<Message> MessageDataOperator::process (std::unique_ptr<Message> message)
{
	if (dataFunction)
	{
		auto dv = dataFunction(data_view(message->data));
		message->data = dv.to_data_block();
	}
	return message;
}


}