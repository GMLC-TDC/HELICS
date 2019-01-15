/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include <boost/version.hpp>

class AsioServiceManager;

#if (BOOST_VERSION / 100 >= 1066)
namespace boost
{
namespace asio
{
class io_context;
using io_service = io_context;
}  // namespace asio
}  // namespace boost
#else
namespace boost
{
namespace asio
{
class io_service;
}  // namespace asio
}  // namespace boost
#endif

namespace boost
{
namespace system
{
class error_code;
}  // namespace system
}  // namespace boost
