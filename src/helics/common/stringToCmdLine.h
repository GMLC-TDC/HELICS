/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
/*
 * LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */
#pragma once

#include <string>
#include <vector>

/** class used to convert a string into command line arguments*/
class StringToCmdLine
{
  public:
    /** construct from a string*/
    explicit StringToCmdLine (const std::string &cmdString);
    /** load a string
    @param cmdString a single string containing command line arguments
    */
    void load (const std::string &cmdString);
    /** get the number of separate arguments corresponding to argc*/
    int getArgCount () const { return argCount; }
    /** get the argument values corresponding to char *argv[]
     */
    auto getArgV () { return stringPtrs.data (); }

  private:
    std::vector<std::string> stringCap;  //!< the locations for the captured strings
    std::vector<char *> stringPtrs;  //!< vector of char * pointers matching stringCap
    int argCount = 0;  //!< the number of arguments
};
