/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once

#include <string>

namespace helics
{
/** helper class designed to contain the common elements between networking brokers and cores
 */
class NetworkBrokerData
{
  public:
    /** define keys for particular interfaces*/
    enum class interface_type
    {
        tcp,  //!< using tcp ports for communication
        udp,  //!< using udp ports for communication
        both,  //!< using both types of ports for communication
    };
    std::string brokerName;  //!< the identifier for the broker
    std::string brokerAddress;  //!< the address or domain name of the broker
    std::string localInterface;  //!< the interface to use for the local receive ports
    int portNumber = -1;  //!< the port number for the local interface
    int brokerPort = -1;  //!< the port number to use for the main broker interface
    int portStart = -1;  //!< the starting port for automatic port definitions
  public:
    NetworkBrokerData () = default;
    /** constructor from the allowed type*/
    explicit NetworkBrokerData (interface_type type) : allowedType (type){};
    /** initialize the properties from input arguments
    @param argc the number of arguments
    @param argv the strings as they may have come from the command line
    @param localAddress a predefined string containing the desired local only address
    */
    void initializeFromArgs (int argc, const char *const *argv, const std::string &localAddress);
    /** display the help line for the network information
     */
    static void displayHelp ();
    /** set the desired interface type
     */
    void setInterfaceType (interface_type type) { allowedType = type; }

  private:
    /** do some checking on the brokerAddress*/
    void checkAndUpdateBrokerAddress (const std::string &localAddress);
    interface_type allowedType = interface_type::both;
};

/** generate a string with a full address based on an interface string and port number
@details,  how things get merged depend on what interface is used some use port number some do not

@param[in] interface a string with an interface description i.e 127.0.0.1
@param portNumber the number of the port to use
@return a string with the merged address
*/
std::string makePortAddress (const std::string &networkInterface, int portNumber);

/** extract a port number and interface string from an address number
@details,  if there is no port number it default to -1 this is true if none was listed
or the interface doesn't use port numbers

@param[in] address a string with an network location description i.e 127.0.0.1:34
@return a pair with a string and int with the interface name and port number
*/
std::pair<std::string, int> extractInterfaceandPort (const std::string &address);

/** extract a port number string and interface string from an address number
@details,  if there is no port number it default to empty string this is true if none was listed
or the interface doesn't use port numbers

@param[in] address a string with an network location description i.e 127.0.0.1:34
@return a pair with 2 strings with the interface name and port number
*/
std::pair<std::string, std::string> extractInterfaceandPortString (const std::string &address);

/** get the external ipv4 address of the current computer
 */
std::string getLocalExternalAddressV4 ();

/** get the external ipv4 Ethernet address of the current computer that best matches the listed server*/
std::string getLocalExternalAddressV4 (const std::string &server);
}  // namespace helics
