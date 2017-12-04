/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

/*
* LLNS Copyright Start
* Copyright (c) 2017, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#ifndef ASIO_SERVICE_MANAGER_HEADER_
#define ASIO_SERVICE_MANAGER_HEADER_

#include <vector>
#include <memory>
#include <atomic>
#include <string>
#include <map>
#include <thread>

#include <boost/asio/io_service.hpp>

/** class defining a (potential) singleton Asio Io_service manager for all boost::asio usage*/
class AsioServiceManager
{
private:
	static std::map<std::string, std::shared_ptr<AsioServiceManager>> services; //!< container for pointers to all the available contexts
	std::string name;  //!< context name
	std::unique_ptr<boost::asio::io_service> iserv; //!< pointer to the actual context
    std::unique_ptr<boost::asio::io_service::work> nullwork; //!< pointer to an object used to keep a service running
    bool leakOnDelete = false; //!< this is done to prevent some warning messages for use in DLL's  
    std::atomic<bool> running{ false };
    int runCounter = 0;  //!< counter for the number of times the runServiceLoop has been called
    std::thread serviceThread;
	AsioServiceManager(const std::string &contextName);
    
public:
    /** return a pointer to a service manager
    @details the function will search for an existing service manager for the name
    if it doesn't find one it will create a new one
    @param serviceName the name of the service to find or create*/
	static std::shared_ptr<AsioServiceManager> getServicePointer(const std::string &serviceName="");
    /** return a pointer to a service manager
    @details the function will search for an existing service manager for the name
    if it doesn't find one it will return nullptr
    @param serviceName the name of the service to find
    */
    static std::shared_ptr<AsioServiceManager> getExistingServicePointer(const std::string &serviceName = "");
    /** get the boost io_service associated with the service manager
    */
	static boost::asio::io_service &getService(const std::string &serviceName="");
    /** get the boost io_service associated with the service manager but only if the service exists
    if it doesn't this will throw and invalid_argument exception
    */
    static boost::asio::io_service &getExistingService(const std::string &serviceName = "");

	static void closeService(const std::string &serviceName="");
	/** tell the service to free the pointer and leak the memory on delete
	@details You may ask why, well in windows systems when operating in a DLL if this context is closed after certain other operations
	that happen when the DLL is unlinked bad things can happen, and since in nearly all cases this happens at Shutdown leaking really doesn't matter that much
    and if you don't the service could terminate before some other parts of the program which cause all sorts of odd errors and issues
	*/
	static void setServiceToLeakOnDelete(const std::string &serviceName = "");
	virtual ~AsioServiceManager();

    /** get the name  of the current service manager*/
	const std::string &getName() const
	{
		return name;
	}

    /** get the underlying boost::io_service reference*/
	boost::asio::io_service &getBaseService() const
	{
		return *iserv;
	}

    /** run a single thread for the service manager to execute asynchronous services in
    @details will run a single thread for the io_service,  it will not stop the thread until either the service
    manager is closed or the haltServiceLoop function is called and there is no more work
    @param in the name of the service
    */
    static void runServiceLoop( const std::string &serviceName = "");
    /** halt the service loop thread if the counter==0
    @details decrements the loop request counter and if it is 0 then will halt the 
    service loop
    @param in the name of the service
    */
    static void haltServiceLoop(const std::string &serviceName = "");


};

#endif
