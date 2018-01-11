/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef NETWORK_BROKER_DATA_H_
#define NETWORK_BROKER_DATA_H_
#pragma once

#include <string>

namespace helics
{
    /** helper class designed to contain the common elements between networking brokers and cores
    */
    class NetworkBrokerData
    {
    public:
        enum class interface_type
        {
            tcp,
            udp,
            both,
        };
        std::string brokerAddress;	//!< the address or domain name of the broker
        std::string localInterface; //!< the interface to use for the local receive ports
        int portNumber = -1;	//!< the port number for the local interface
        int brokerPort = -1;  //!< the port number to use for the main broker interface
        int portStart = -1;  //!< the starting port for automatic port definitions
    public:
        NetworkBrokerData() = default;
        /** constructor from the allowed type*/
        explicit NetworkBrokerData(interface_type type) :allowedType(type) {};
        void initializeFromArgs(int argc, const char *const *argv, const std::string &localAddress);
        static void displayHelp();
        void setInterfaceType(interface_type type) {
            allowedType = type;
        }
    private:
        /** do some checking on the brokerAddress*/
        void checkAndUpdateBrokerAddress(const std::string &localAddress);
        interface_type allowedType = interface_type::both;
    };


    /** generate a string with a full address based on an interface string and port number
    @details,  how things get merged depend on what interface is used some use port number some do not

    @param[in] interface a string with an interface description i.e 127.0.0.1
    @param portNumber the number of the port to use
    @return a string with the merged address
    */
    std::string makePortAddress(const std::string &networkInterface, int portNumber);

    /** extract a port number and interface string from an address number
    @details,  if there is no port number it default to -1 this is true if none was listed
    or the interface doesn't use port numbers

    @param[in] address a string with an network location description i.e 127.0.0.1:34
    @return a pair with a string and int with the interface name and port number
    */
    std::pair<std::string, int> extractInterfaceandPort(const std::string &address);


    /** extract a port number string and interface string from an address number
    @details,  if there is no port number it default to empty string this is true if none was listed
    or the interface doesn't use port numbers

    @param[in] address a string with an network location description i.e 127.0.0.1:34
    @return a pair with 2 strings with the interface name and port number
    */
    std::pair<std::string, std::string> extractInterfaceandPortString(const std::string &address);

    /** get the external ipv4 address of the current computer
    */
    std::string getLocalExternalAddressV4();
    
    /** get the external ipv4 Ethernet address of the current computer that best matches the listed server*/
    std::string getLocalExternalAddressV4(const std::string &server);
}
#endif /*NETWORK_BROKER_DATA_H_*/