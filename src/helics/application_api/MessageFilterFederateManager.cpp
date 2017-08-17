/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "MessageFilterFederateManager.h"
#include "MessageOperators.h"

namespace helics
{

	MessageFilterFederateManager::MessageFilterFederateManager(std::shared_ptr<Core> coreOb, Core::federate_id_t id) :coreObject(std::move(coreOb)), fedID(id)
	{

	}

	MessageFilterFederateManager::~MessageFilterFederateManager()
	{

	}

	filter_id_t MessageFilterFederateManager::registerSourceFilter(const std::string &filterName, const std::string &sourceEndpoint, const std::string &inputType)
	{
		std::lock_guard<std::mutex> fLock(filterLock);
		filter_id_t id = static_cast<identifier_type>(filters.size());
		filters.emplace_back(filterName, sourceEndpoint, inputType);
		filters.back().id = id;
		filters.back().ftype = filter_type::source_filter;
		filters.back().handle = coreObject->registerSourceFilter(fedID, filterName, sourceEndpoint, inputType);
		SourceFilterNames.emplace(filterName, id);
		if (filterName != sourceEndpoint)
		{
			SourceFilterNames.emplace(sourceEndpoint, id);
		}
		filterQueues.resize(id.value());
		handleLookup.emplace(filters.back().handle, id);
		return id;
	}
		
	filter_id_t MessageFilterFederateManager::registerDestinationFilter(const std::string &filterName, const std::string &destEndpoint, const std::string &inputType)
	{
		std::lock_guard<std::mutex> fLock(filterLock);
		filter_id_t id = static_cast<identifier_type>(filters.size());
		filters.emplace_back(filterName, destEndpoint, inputType);
		filters.back().id = id;
		filters.back().ftype = filter_type::dest_filter;
		DestFilterNames.emplace(filterName, id);
		if (filterName != destEndpoint)
		{
			DestFilterNames.emplace(destEndpoint, id);
		}
		filters.back().handle = coreObject->registerDestinationFilter(fedID, filterName, destEndpoint, inputType);
		handleLookup.emplace(filters.back().handle, id);
		return id;
	}

	bool MessageFilterFederateManager::hasMessageToFilter() const
	{
		for (auto &mq : filterQueues)
		{
			if (!mq.empty())
			{
				return true;
			}
		}
		return false;
	}
		
	bool MessageFilterFederateManager::hasMessageToFilter(filter_id_t filter) const
	{
		return (filter.value() < filters.size()) ? (!filterQueues[filter.value()].empty()) : false;
	}
		
	std::unique_ptr<Message> MessageFilterFederateManager::getMessageToFilter(filter_id_t filter)
	{
		if (filter.value() < filters.size())
		{
			auto ms = filterQueues[filter.value()].pop();
			return (ms) ? (std::move(*ms)) : nullptr;
			
		}
		return nullptr;
	}

	static const std::string nullStr;

	void MessageFilterFederateManager::updateTime(Time newTime, Time /*oldTime*/)
	{
		CurrentTime = newTime;
		auto epCount = coreObject->receiveFilterCount(fedID);
		// lock the data updates
		std::unique_lock<std::mutex> filtlock(filterLock);
		Core::Handle filterID;
		for (size_t ii = 0; ii < epCount; ++ii)
		{
			auto message = coreObject->receiveAnyFilter(fedID, filterID);
			if (!message)
			{
				break;
			}

			/** find the id*/
			auto fid = handleLookup.find(filterID);
			if (fid != handleLookup.end())
			{  // assign the data

				auto localfilterIndex = fid->second.value();
				filterQueues[localfilterIndex].emplace(std::move(message));
				if (filters[localfilterIndex].callbackIndex >= 0)
				{
					auto cb = callbacks[filters[localfilterIndex].callbackIndex];
					filtlock.unlock();
					cb(fid->second, CurrentTime);
					filtlock.lock();
				}
				else if (allCallbackIndex >= 0)
				{
					auto ac = callbacks[allCallbackIndex];
					filtlock.unlock();
					ac(fid->second, CurrentTime);
					filtlock.lock();
				}
			}
		}
	}

	void MessageFilterFederateManager::StartupToInitializeStateTransition()
	{
		//this is only called via a single thread
		filterQueues.resize(filters.size());
	}

	void MessageFilterFederateManager::InitializeToExecuteStateTransition()
	{
		
	}

	std::string MessageFilterFederateManager::getFilterName(filter_id_t id) const
	{
		std::lock_guard<std::mutex> fLock(filterLock);
		return (id.value() < filters.size()) ? (filters[id.value()].name) : nullStr;

	}
	std::string  MessageFilterFederateManager::getFilterEndpoint(filter_id_t id) const
	{
		std::lock_guard<std::mutex> fLock(filterLock);
		return (id.value() < filters.size()) ? (filters[id.value()].endpoint) : nullStr;
	}

	std::string  MessageFilterFederateManager::getFilterType(filter_id_t id) const
	{
		std::lock_guard<std::mutex> fLock(filterLock);
		return (id.value() < filters.size()) ? (filters[id.value()].type) : nullStr;
	}


	filter_id_t MessageFilterFederateManager::getFilterId(const std::string &filterName) const
	{
		std::lock_guard<std::mutex> fLock(filterLock);
		auto sub = SourceFilterNames.find(filterName);
		if (sub != SourceFilterNames.end())
		{
			return sub->second;
		}
		auto subd = DestFilterNames.find(filterName);
		if (subd != DestFilterNames.end())
		{
			return subd->second;
		}
		return invalid_id_value;
	}

	filter_id_t MessageFilterFederateManager::getSourceFilterId(const std::string &endpointName) const
	{
		std::lock_guard<std::mutex> fLock(filterLock);
		auto sub = SourceFilterNames.find(endpointName);
		if (sub != SourceFilterNames.end())
		{
			return sub->second;
		}
		return invalid_id_value;
	}

	filter_id_t MessageFilterFederateManager::getDestFilterId(const std::string &endpointName) const
	{
		std::lock_guard<std::mutex> fLock(filterLock);
		auto sub = DestFilterNames.find(endpointName);
		if (sub != DestFilterNames.end())
		{
			return sub->second;
		}
		return invalid_id_value;
	}
		
	void MessageFilterFederateManager::registerFilterCallback(std::function<void(filter_id_t,Time)> callback)
	{
		std::lock_guard<std::mutex> fLock(filterLock);
		if (allCallbackIndex < 0)
		{
			allCallbackIndex = static_cast<int> (callbacks.size());
			callbacks.emplace_back(std::move(callback));
		}
		else
		{
			callbacks[allCallbackIndex] = std::move(callback);
		}
	}
		
	void MessageFilterFederateManager::registerFilterCallback(filter_id_t id, std::function<void(filter_id_t, Time)> callback)
	{
		if (id.value() < filters.size())
		{
			std::lock_guard<std::mutex> fLock(filterLock);
			filters[id.value()].callbackIndex = static_cast<int> (callbacks.size());
			callbacks.emplace_back(std::move(callback));
		}
		else
		{
			throw(std::invalid_argument("filter id is invalid"));
		}
	}
		
	void MessageFilterFederateManager::registerFilterCallback(const std::vector<filter_id_t> &ids, std::function<void(filter_id_t, Time)> callback)
	{
		std::lock_guard<std::mutex> fLock(filterLock);
		int ind = static_cast<int> (callbacks.size());
		callbacks.emplace_back(std::move(callback));
		for (auto id : ids)
		{
			if (id.value() < filters.size())
			{
				filters[id.value()].callbackIndex = ind;
			}
		}
	}

	void MessageFilterFederateManager::registerMessageOperator(std::shared_ptr<MessageOperator> mo)
	{
		std::lock_guard<std::mutex> fLock(filterLock);
		operators.push_back(mo);
		for (auto &flt : filters)
		{
			coreObject->setFilterOperator(flt.handle, mo);
		}

	}

	void MessageFilterFederateManager::registerMessageOperator(filter_id_t id, std::shared_ptr<MessageOperator> mo)
	{
		if (id.value() < filters.size())
		{
			std::lock_guard<std::mutex> fLock(filterLock);
			operators.push_back(mo);
			coreObject->setFilterOperator(filters[id.value()].handle, mo);
		}
		else
		{
			throw(std::invalid_argument("filter id is invalid"));
		}
	}

	void MessageFilterFederateManager::registerMessageOperator(const std::vector<filter_id_t> &filter_ids, std::shared_ptr<MessageOperator> mo)
	{
		std::lock_guard<std::mutex> fLock(filterLock);
		operators.push_back(mo);
		for (auto id : filter_ids)
		{
			if (id.value() < filters.size())
			{
				coreObject->setFilterOperator(filters[id.value()].handle, mo);
			}
		}
	}


}