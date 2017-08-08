/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "core-broker.h"
#include "common/stringToCmdLine.h"
namespace helics
{

bool matchingTypes(const std::string &type1, const std::string &type2);

void CoreBroker::broker()
{
	while (true)
	{
		auto command = _queue.pop();
		//LOG (INFO) << "\"\"\"" << command << std::endl << "\"\"\"" << ENDL;

		switch (command.action())
		{
		case CMD_IGNORE:
			break;
		case CMD_STOP:
			return;
		default:
			processCommand(command);
			break;
		}
	}
}

CoreBroker::~CoreBroker()
{
	if (_initialized)
	{
		_queue.push(CMD_STOP);
		_broker_thread.join();
	}
}


void CoreBroker::setIdentifier(const std::string &name)
{
	if (!_initialized)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		local_broker_identifier = name;
	}
}
int32_t CoreBroker::getRoute(Core::federate_id_t fedid) const
{
	if (_initialized)
	{
		auto fnd = routing_table.find(fedid);
		return (fnd != routing_table.end()) ? fnd->second : 0;
	}
	else
	{
		std::lock_guard<std::mutex> lock(mutex_);
		auto fnd = routing_table.find(fedid);
		return (fnd != routing_table.end()) ? fnd->second : 0;
	}
	
}


int32_t CoreBroker::getFedByName(const std::string &fedName) const
{
	if (_initialized)
	{
		auto fnd = fedNames.find(fedName);
		return (fnd != fedNames.end()) ? fnd->second : -1;
	}
	else
	{
		std::lock_guard<std::mutex> lock(mutex_);
		auto fnd = fedNames.find(fedName);
		return (fnd != fedNames.end()) ? fnd->second : -1;
	}
}


int32_t CoreBroker::getBrokerByName(const std::string &brokerName) const
{
	if (_initialized)
	{
		auto fnd = brokerNames.find(brokerName);
		return (fnd != brokerNames.end()) ? fnd->second : -1;
	}
	else
	{
		std::lock_guard<std::mutex> lock(mutex_);
		auto fnd = brokerNames.find(brokerName);
		return (fnd != brokerNames.end()) ? fnd->second : -1;
	}
}

bool isPriorityCommand(const ActionMessage &command)
{
	switch (command.action())
	{
	case CMD_CONNECT:
	case CMD_REG_ROUTE:
	case CMD_REG_FED:
	case CMD_FED_ACK:
		//case CMD_ROUTE_ACK:
		return true;
	default:
		return false;
	}
}

void CoreBroker::addMessage(const ActionMessage &m)
{
	if (isPriorityCommand(m))
	{
		processPriorityCommand(m);
	}
	else
	{
		//just route to the general queue;
		_queue.push(m);
	}
	
}



void CoreBroker::processPriorityCommand(const ActionMessage &command)
{
	//deal with a few types of message immediately
	switch (command.action())
	{
	case CMD_REG_FED:
	{
		if (!_initialized)
		{
			if (allInitReady())
			{
				//send an init not ready
			}
		}
		else //we are initialized already
		{
			ActionMessage badInit(CMD_FED_ACK);
			badInit.error = true;
			badInit.source_id = global_broker_id;
			badInit.name = command.name;
			transmit(command.source_id, badInit);
			return;
		}
		std::unique_lock<std::mutex> lock(mutex_);
		_federates.emplace_back(command.name);
		_federates.back().route_id = command.dest_id;
		if (!_isRoot)
		{
			if (_gateway)
			{
				auto mcopy = command;
				mcopy.source_id = global_broker_id;
				transmit(0, mcopy);
			}
			else
			{
				transmit(0, command);
			}

		}
		else
		{
			_federates.back().global_id = static_cast<Core::federate_id_t>(_federates.size()) - 1;
			ActionMessage fedReply(CMD_FED_ACK);
			fedReply.source_id = 0;
			fedReply.dest_id = _federates.back().global_id;
			fedReply.name = command.name;
			transmit(command.source_id, fedReply);
		}
	}
	break;
	case CMD_CONNECT:
	{
		if (!_initialized)
		{
			if (allInitReady())
			{
				//send an init not ready
			}
		}
		else //we are initialized already
		{
			ActionMessage badInit(CMD_BROKER_ACK);
			badInit.error = true;
			badInit.source_id = global_broker_id;
			badInit.name = command.name;
			transmit(command.source_id, badInit);
			return;
		}
		std::unique_lock<std::mutex> lock(mutex_);
		_brokers.emplace_back(command.name);
		_brokers.back().route_id = command.dest_id;
		if (!_isRoot)
		{
			if (_gateway)
			{
				auto mcopy = command;
				mcopy.source_id = global_broker_id;
				transmit(0, mcopy);
			}
			else
			{
				transmit(0, command);
			}

		}
		else
		{
			_brokers.back().global_id = static_cast<Core::federate_id_t>(_brokers.size()) - 1;
			ActionMessage brokerReply(CMD_BROKER_ACK);
			brokerReply.source_id = 0;
			brokerReply.dest_id = _brokers.back().global_id;
			brokerReply.name = command.name;
			transmit(command.source_id, brokerReply);
		}
	}
	break;
	case CMD_FED_ACK:
	{  //we can't be root if we got one of these
		auto fed_num = getFedByName(command.name);
		int32_t route;
		if (fed_num >= 0)
		{
			std::unique_lock<std::mutex> lock(mutex_);
			_federates[fed_num].global_id = command.dest_id;
			route = _federates[fed_num].route_id;
			lock.unlock();
			transmit(_federates[fed_num].route_id, command);
		}
		else
		{
			std::unique_lock<std::mutex> lock(mutex_);
			_federates.emplace_back(command.name);
			_federates.back().route_id = getRoute(command.source_id);
			_federates.back().global_id = command.dest_id;
		}
		
		
	}
	break;
	case CMD_BROKER_ACK:
	{  //we can't be root if we got one of these
		if (command.name == local_broker_identifier)
		{
			global_broker_id = command.dest_id;
			return;
		}
		auto broker_num = getBrokerByName(command.name);
		int32_t route;
		if (broker_num >= 0)
		{
			std::unique_lock<std::mutex> lock(mutex_);
			_brokers[broker_num].global_id = command.dest_id;
			route = _brokers[broker_num].route_id;
			lock.unlock();
			transmit(_brokers[broker_num].route_id, command);
		}
		else
		{
			std::unique_lock<std::mutex> lock(mutex_);
			_brokers.emplace_back(command.name);
			_brokers.back().route_id = getRoute(command.source_id);
			_brokers.back().global_id = command.dest_id;
		}


	}
	break;
		break;
	}
	
}

void CoreBroker::processCommand(ActionMessage &command)
{
	if (isPriorityCommand(command))
	{
		processPriorityCommand(command);
		return;
	}
	switch (command.action())
	{
	case CMD_IGNORE:
		break;
	case CMD_INIT:

			if (allInitReady())
			{
				if (_isRoot)
				{
					checkPublications();
					checkEndpoints();
					checkFilters();
					//computeDependencies();
					ActionMessage m(CMD_INIT_GRANT);
					for (auto &brk : _brokers)
					{
						transmit(brk.route_id, m);
					}
				}
				else
				{
					command.source_id = global_broker_id;
					transmit(0, command);
				}
			}
		
		break;
	case CMD_INIT_GRANT:
		for (auto &brk : _brokers)
		{
			transmit(brk.route_id, command);
		}
		break;
	case CMD_EXEC_REQUEST:
		transmit(getRoute(command.dest_id), command);
		break;
	case CMD_EXEC_GRANT:
		transmit(getRoute(command.dest_id), command);
		break;
	case CMD_REG_ROUTE:
		break;
	case CMD_TIME_REQUEST:
		transmit(getRoute(command.dest_id), command);
		break;
	case CMD_TIME_GRANT:
		transmit(getRoute(command.dest_id), command);
		break;
	case CMD_SEND_MESSAGE:
		transmit(getRoute(command.dest_id), command);
		break;
	case CMD_SEND_FOR_FILTER:
		transmit(getRoute(command.dest_id), command);
		break;
	case CMD_PUB:
		transmit(getRoute(command.dest_id), command);
		break;
	case CMD_BYE:
		break;
	case CMD_LOG:
		if (_isRoot)
		{
			//do some logging
		}
		else
		{
			transmit(0, command);
		}
		break;
	case CMD_ERROR:
		if (_isRoot)
		{
			//do some logging
		}
		else
		{
			transmit(0, command);
		}
		break;
	case CMD_REG_PUB:
		break;
	case CMD_REG_SUB:
		break;
	case CMD_REG_END:
		break;
	case CMD_REG_DST:
		break;
	case CMD_REG_SRC:
		break;
	}
}

CoreBroker::CoreBroker(bool isRoot) noexcept : _isRoot(isRoot)
{

}

void CoreBroker::Initialize(const std::string &initializationString)
{
	bool exp = false;
	if (_initialized.compare_exchange_strong(exp,true))
	{
		stringToCmdLine strcmd(initializationString);
		_broker_thread = std::thread(&CoreBroker::broker, this);
	}
	
}


void CoreBroker::checkPublications()
{
	// pub/sub checks
	//LOG(INFO) << "performing pub/sub check" << ENDL;
	for (auto &hndl : _handles)
	{
		if (hndl.what == HANDLE_SUB)
		{
			auto pubHandle = publications.find(hndl.key);
			if (pubHandle != publications.end())
			{
				auto pubInfo = _handles[pubHandle->second];
				if (!matchingTypes(pubInfo.type, hndl.type))
				{
					//LOG(WARN) << "sub " << hndl->key << " does not match types" << hndl->type << " " << pubInfo->type << ENDL;
				}
				//TODO::send a message to the subscriber and publisher
				//auto pubInfoComplete = getFederate(pubInfo->fed_id)->getPublication(pubInfo->id);
				//pubInfoComplete->subscribers.emplace_back(hndl->fed_id, hndl->id);
			}
			else if (hndl.flag)
			{
				//LOG(WARN) << "sub " << hndl->key << " has no corresponding pub" << ENDL;
			}
		}
	}

}

void CoreBroker::checkEndpoints()
{

}

void CoreBroker::checkFilters()
{
	//LOG(INFO) << "performing filter check" << ENDL;
	for (auto &hndl : _handles)
	{
		if (hndl.what == HANDLE_FILTER)
		{
			auto target = endpoints.find(hndl.target);
			if (target != endpoints.end())
			{
				//flag that the endpoint has a filter
			//	auto filterI = getFederate(hndl->fed_id)->getFilter(hndl->id);
			//	auto epI = getHandleInfo(target->second);
			//	epI->flag = true;  //flag indicates that the endpoint has filters
								   //get the filter Functions
			//	auto ffunc = getFilterFunctions(epI->id);
			//	if (hndl->destFilter)
			//	{
					//get the detailed info
			//		if (filterI->filterOp == nullptr)
			//		{
				//		LOG(WARN) << "destination filter for" << hndl->target << " must use filter operators" << ENDL;
			//		}
			//		epI->destFilter;
			//		if (ffunc->hasDestOperator)
			//		{
			//			LOG(WARN) << "multiple destination Filters set for" << hndl->target << ENDL;
			//		}
			//		else
			//		{
			//			ffunc->destOperator = { hndl->fed_id,hndl->id };
			//			ffunc->hasDestOperator = true;
			//		}
			//	}
			//	else
			//	{
					//is the filter an operator or a different Federate
			//		if (filterI->filterOp != nullptr)
			//		{
			//			ffunc->hasSourceOperators = true;
			//			ffunc->sourceOperators.emplace_back(hndl->fed_id, hndl->id);
			//		}
			//		else
			//		{
			//			if (ffunc->hasSourceFilter)
			//			{
			//				LOG(WARN) << "multiple non-operator Source Filters set for" << hndl->target << ENDL;
			//			}
			//			else
			//			{
			//				ffunc->finalSourceFilter = { hndl->fed_id,hndl->id };
			//				ffunc->hasSourceFilter = true;
			//			}
			//		}
			//	}

			//}
			//else
			//{
			//	LOG(WARN) << "filter for" << hndl->target << " has no corresponding endpoint" << ENDL;
			}

		}
	}
}

bool CoreBroker::allInitReady() const
{
	//the federate count must be greater than the min size
	if (static_cast<decltype(_min_federates)>(_federates.size()) < _min_federates)
	{
		return false;
	}
	if (static_cast<decltype(_min_brokers)>(_brokers.size()) < _min_brokers)
	{
		return false;
	}
	//all subBrokers must be requesting init
	for (auto &brk : _brokers)
	{
		if (brk._initRequested == false)
		{
			return false;
		}
	}
	return true;
}




bool matchingTypes(const std::string &type1, const std::string &type2)
{
	if (type1 == type2)
	{
		return true;
	}
	if ((type1.empty()) || (type2.empty()))
	{
		return true;
	}
	if ((type1 == "block") || (type2 == "block"))
	{
		return true;
	}
	if ((type1 == "string") || (type2 == "string"))
	{
		return true;
	}
	if ((type1 == "data") || (type2 == "data"))
	{
		return true;
	}
	return false;
}
}
