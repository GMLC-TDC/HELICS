/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include <string>

namespace helics
{
/** define the network access*/
enum class interface_networks :char
{
    local,  //!< just open local ports
    ipv4,  //!< use external ipv4 ports
    ipv6,  //!< use external ipv6 ports
    all, //!< use all external ports
};

/** define keys for particular interfaces*/
enum class interface_type :char
{
    tcp = 0,  //!< using tcp ports for communication
    udp = 1,  //!< using udp ports for communication
    ip = 2,  //!< using both types of ports (tcp/or udp) for communication
    ipc = 3, //!< using ipc locations
    inproc=4, //!< using inproc sockets for communications
};

/** helper class designed to contain the common elements between networking brokers and cores
 */
class NetworkBrokerData
{
  public:
   
	enum class server_mode_options :char
	{
		unspecified = 0,
		server_default_active=1,
		server_default_deactivated=2,
		server_active = 3,
		server_deactivated=4,
	};

    std::string brokerName;  //!< the identifier for the broker
    std::string brokerAddress;  //!< the address or domain name of the broker
    std::string localInterface;  //!< the interface to use for the local connection
    std::string brokerInitString;  //!< a string containing arguments for the broker initialization
    int portNumber = -1;  //!< the port number for the local interface
    int brokerPort = -1;  //!< the port number to use for the main broker interface
    int portStart = -1;  //!< the starting port for automatic port definitions
	int maxMessageSize = 16 * 256; //!< maximum message size
	int maxMessageCount = 256; //!< maximum message count
    interface_networks interfaceNetwork = interface_networks::local;
    bool reuse_address = false; //!< allow reuse of binding address
    bool use_os_port = false;  //!< specify that any automatic port allocation should use operating system allocation
    bool autobroker = false;  //!< flag for specifying an automatic broker generation
	server_mode_options server_mode = server_mode_options::unspecified; //!< setup a server mode
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
    interface_type allowedType = interface_type::ip;
    
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

/** strip any protocol strings from the interface and return a new string
@example tcp://127.0.0.1 -> 127.0.0.1*/
std::string stripProtocol(const std::string &networkAddress);
/** strip any protocol strings from the interface and return a new string*/
void removeProtocol (std::string &networkAddress);

/** add a protocol url to the interface and return a new string*/
std::string addProtocol (const std::string &networkAddress, interface_type interfaceT);

/** add a protocol url to the interface modifying the string in place*/
void insertProtocol (std::string &networkAddress, interface_type interfaceT);

/** check if a specified address is v6 or v4
@return true if the address is a v6 address
*/
bool isipv6(const std::string &address);

/** get the external ipv4 address of the current computer
 */
std::string getLocalExternalAddressV4 ();

/** get the external ipv4 Ethernet address of the current computer that best matches the listed server*/
std::string getLocalExternalAddress(const std::string &server);

/** get the external ipv4 Ethernet address of the current computer that best matches the listed server*/
std::string getLocalExternalAddressV4 (const std::string &server);

/** get the external ipv4 address of the current computer
*/
std::string getLocalExternalAddressV6();

/** get the external ipv4 Ethernet address of the current computer that best matches the listed server*/
std::string getLocalExternalAddressV6(const std::string &server);

/** generate an interface that matches a defined server or network specification
*/
std::string generateMatchingInterfaceAddress(const std::string &server, interface_networks network=interface_networks::local);
}  // namespace helics
