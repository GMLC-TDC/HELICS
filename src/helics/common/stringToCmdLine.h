/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
*/
/*
* LLNS Copyright Start
* Copyright (c) 2017, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/
#ifndef STRING_TO_CMD_LINE
#define STRING_TO_CMD_LINE
#pragma once

#include <string>
#include <vector>

/** class used to convert a string into command line arguments*/
class StringToCmdLine
{
public:
	/** construct from a string*/
	StringToCmdLine(const std::string &cmdString);
	/** load a string
	@param cmdString a single string containing command line arguments
	*/
	void load(const std::string &cmdString);
	/** get the number of separate arguments corresponding to argc*/
	int getArgCount() const { return argCount; }
	/** get the argument values corresponding to char *argv[]
	*/
	auto getArgV() { return stringPtrs.data(); }

private:
	std::vector<std::string> stringCap; //!< the locations for the captured strings
	std::vector<char *> stringPtrs; //!< vector of char * pointers matching stringCap
	int argCount;	//!< the number of arguments

};

#endif

