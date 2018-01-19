#include "helicsVersion.hpp"

namespace helics
{
std::string helicsVersionString()
{
	std::string vstr = std::to_string(HELICS_VERSION_MAJOR);
	vstr.push_back('.');
	vstr.append(std::to_string(HELICS_VERSION_MINOR));
	vstr.push_back('.');
	vstr.append(std::to_string(HELICS_VERSION_PATCH));

	std::string buildStr(HELICS_VERSION_BUILD);
	if (!buildStr.empty())
	{
		vstr.push_back('.');
		vstr.append(buildStr);
	}
	vstr.push_back(' ');
	vstr.push_back('(');
	vstr += HELICS_DATE;
	vstr.push_back(')');
	return vstr;
}
}
