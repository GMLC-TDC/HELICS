/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "Publications.hpp"

namespace helics
{
void Publication::publish(double val) const
{
	if (changeDetectionEnabled)
	{
		if (changeDetected(val))
		{
			fed->publish(id, val);
		}
	}
	else
	{
		fed->publish(id, val);
	}
}
void Publication::publish(int64_t val) const
{
	if (changeDetectionEnabled)
	{
		if (changeDetected(val))
		{
			fed->publish(id, val);
		}
	}
	else
	{
		fed->publish(id, val);
	}
}
void Publication::publish(const char *val) const
{
	if (changeDetectionEnabled)
	{
		if (changeDetected(val))
		{
			fed->publish(id, val);
		}
	}
	else
	{
		fed->publish(id, val);
	}
}
void Publication::publish(const std::string &val) const
{
	if (changeDetectionEnabled)
	{
		if (changeDetected(val))
		{
			fed->publish(id, val);
		}
	}
	else
	{
		fed->publish(id, val);
	}
}
void Publication::publish(const std::vector<double> &val) const
{
	if (changeDetectionEnabled)
	{
		if (changeDetected(val))
		{
			fed->publish(id, val);
		}
	}
	else
	{
		fed->publish(id, val);
	}
}
void Publication::publish(const double val[], size_t len) const
{
	if (changeDetectionEnabled)
	{
		//if (changeDetected(val))
		{
			fed->publish(id, val);
		}
	}
	else
	{
		fed->publish(id, val);
	}
}
void Publication::publish(std::complex<double> val) const
{
	if (changeDetectionEnabled)
	{
		if (changeDetected(val))
		{
			fed->publish(id, val);
		}
	}
	else
	{
		fed->publish(id, val);
	}
}

bool Publication::changeDetected(const std::string &val) const
{
	return true;
}
bool Publication::changeDetected(const std::vector<double> &val) const
{
	return true;
}
bool Publication::changeDetected(const std::complex<double> &val) const
{
	return true;
}
bool Publication::changeDetected(double val) const
{
	return true;
}
bool Publication::changeDetected(int64_t val) const
{
	return true;
}

} //namespace helics
