/*
 * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#ifndef EXE_HELPER_HEADER_
#define EXE_HELPER_HEADER_
#pragma once
#include <future>
#include <string>

/** helper class to run executable files*/
class exeTestRunner
{
  private:
    std::string exeString;
    bool active=false;
    static int counter;
    std::string outFile;

  public:
    exeTestRunner ();
    exeTestRunner (const std::string &baseLocation, const std::string &target);
    exeTestRunner (const std::string &baseLocation, const std::string &baseLocation2, const std::string &target);
    bool findFileLocation (const std::string &baseLocation, const std::string &target);
    bool isActive () const { return active; };

    int run (const std::string &args) const;
    std::future<int> runAsync (const std::string &args) const;
    std::string runCaptureOutput (const std::string &args) const;
    std::future<std::string> runCaptureOutputAsync (const std::string &args) const;
};

#endif
