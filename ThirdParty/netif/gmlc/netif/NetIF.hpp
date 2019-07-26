/*
Copyright © 2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance
for Sustainable Energy, LLC.  See the top-level NOTICE for additional details.
All rights reserved. SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

// other platforms might need different headers -- detecting with macros:
// https://sourceforge.net/p/predef/wiki/OperatingSystems/
// https://stackoverflow.com/questions/142508/how-do-i-check-os-with-a-preprocessor-directive
#if defined(_WIN32)
  #include <winsock2.h>
  #include <WS2tcpip.h>
  #include <iphlpapi.h>

  #pragma comment(lib, "IPHLPAPI.lib")
  #pragma comment(lib, "Ws2_32.lib")
#else
  #include <arpa/inet.h>
  #include <ifaddrs.h>
#endif

namespace gmlc
{
namespace netif
{
  inline std::string addressToString(struct sockaddr *addr)
  {
    int family = addr->sa_family;
    char addr_str[INET6_ADDRSTRLEN];
    void *src_addr = nullptr;
    switch (family) {
      case AF_INET:
        src_addr = &((struct sockaddr_in *)addr)->sin_addr;
        break;
      case AF_INET6:
        src_addr = &((struct sockaddr_in6 *)addr)->sin6_addr;
        break;
      default: // Invalid address type for conversion to text
        return std::string();
    }
    inet_ntop(family, src_addr, addr_str, INET6_ADDRSTRLEN);
    return std::string(addr_str);
  }
	
  template <class A>
  inline void freeAddresses(A addrs)
  {
    #if defined(_WIN32)
      HeapFree(GetProcessHeap(),0,addrs);
    #else
      freeifaddrs(addrs);
    #endif
  }

  template <class A>
  inline auto getAddresses(int family, A* addrs)
  {
    #if defined(_WIN32)
      // Windows API: https://docs.microsoft.com/en-us/windows/win32/api/iphlpapi/nf-iphlpapi-getadaptersaddresses
      DWORD rv = 0;
      ULONG bufLen = 15000; // recommended size from Windows API docs to avoid error
      ULONG iter = 0;
      do {
        *addrs = (IP_ADAPTER_ADDRESSES *)HeapAlloc(GetProcessHeap(),0,bufLen);
        if (*addrs == NULL) {
          return -1;
        }

        rv = GetAdaptersAddresses(family, GAA_FLAG_INCLUDE_PREFIX, NULL, *addrs, &bufLen);
        if (rv == ERROR_BUFFER_OVERFLOW) {
          freeAddresses<decltype(*addrs)>(*addrs);
          *addrs = NULL;
          bufLen = bufLen*2; // Double buffer length for the next attempt
        } else {
          break;
        }
        iter++;
      } while ((rv == ERROR_BUFFER_OVERFLOW) && (iter < 3));
      if (rv != NO_ERROR) {
        // May want to change this depending on the type of error
        return -1;
      }
      return 0;
    #else
      return getifaddrs(addrs);
    #endif
  }

  template <class A>
  inline auto getSockAddr(A addr)
  {
    #if defined(_WIN32)
      return addr->Address.lpSockaddr;
    #else
      return addr->ifa_addr;
    #endif
  }
	
  template <class A>
  inline A getNextAddress(int family, A addrs)
  {
    #if defined(_WIN32)
      return addrs->Next;
    #else
    auto next = addrs->ifa_next;
    while (next != NULL) {
      // Skip entries with a null sockaddr
      auto next_sockaddr = getSockAddr<decltype(next)>(next);
      if (next_sockaddr == NULL) {
        next = next->ifa_next;
	continue;
      }
	    
      int next_family = next_sockaddr->sa_family;
      // Skip if the family is not IPv4 or IPv6
      if (next_family != AF_INET && next_family != AF_INET6) {
        next = next->ifa_next;
        continue;
      }
      // Skip if a specific IP family was requested and the family doesn't match
      if ((family == AF_INET || family == AF_INET6) && family != next_family) {
        next = next->ifa_next;
        continue;
      }
      // An address entry meeting the requirements was found, return it
      return next;
    }
    return nullptr;
    #endif
  }
	  
  std::vector<std::string> getInterfaceAddresses(int family)
  {
    std::vector<std::string> result_list;
	  
#if defined(_WIN32)
    PIP_ADAPTER_ADDRESSES allAddrs = NULL;
#else
    struct ifaddrs *allAddrs;
#endif
	 
    getAddresses<decltype(allAddrs)>(family, &allAddrs);

#if defined(_WIN32)
    PIP_ADAPTER_ADDRESSES winAddrs = allAddrs;
    while (winAddrs) {
      auto addrs = winAddrs->FirstUnicastAddress;
#else
    auto addrs = allAddrs;
#endif
	    
    for (auto a = addrs; a != NULL; a = getNextAddress<decltype(a)>(family, a)) {
      std::string ipAddr = addressToString(getSockAddr<decltype(a)>(a));
      if (!ipAddr.empty()) {
        result_list.push_back(ipAddr);
      }
    }

#if defined(_WIN32)
      winAddrs = getNextAddress<decltype(winAddrs)>(family, winAddrs);
    }
#endif

    if (allAddrs) {
      freeAddresses<decltype(allAddrs)>(allAddrs);
    }
    return result_list;
  }

  std::vector<std::string> getInterfaceAddressesV4()
  {
    return getInterfaceAddresses(AF_INET);
  }
	
  std::vector<std::string> getInterfaceAddressesV6()
  {
    return getInterfaceAddresses(AF_INET6);
  }

  std::vector<std::string> getInterfaceAddressesAll()
  {
    return getInterfaceAddresses(AF_UNSPEC);
  }
} // namespace netif
} // namespace gmlc