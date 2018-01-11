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
        void initializeFromArgs(int argc, const char *const *argv);
        static void displayHelp();
        void setInterfaceType(interface_type type) {
            allowedType = type;
        }
    private:
        interface_type allowedType = interface_type::both;
    };

    /** generate a full address from an interface and port number */
    std::string makePortAddress(const std::string &networkInterface, int portNumber);

    /** extract the interface and port number from a string
    @returns a pair containing the interface in the first element and the port number as the second (-1) if no port number specified*/
    std::pair<std::string, int> extractInterfaceandPort(const std::string &address);
    /** extract the interface and port number as a string from a string
    @returns a pair containing the interface in the first element and the port number as the second empty string if no port number specified*/
    std::pair<std::string, std::string> extractInterfaceandPortString(const std::string &address);

}
#endif /*NETWORK_BROKER_DATA_H_*/