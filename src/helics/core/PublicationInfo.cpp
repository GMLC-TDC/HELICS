/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
*/
#include "PublicationInfo.hpp"
#include "helics_includes/string_view.h"
namespace helics
{
bool PublicationInfo::CheckSetValue (const char *dataToCheck, uint64_t len)
{
    if ((len != data.length ()) || (stx::string_view (data) != stx::string_view (dataToCheck, len)))
    {
        data = std::string (dataToCheck, len);
        return true;
    }
    return false;
}
}  // namespace helics

