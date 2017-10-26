/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/unit_test.hpp>

#include <cstdio>

#include "exeTestHelper.h"

BOOST_AUTO_TEST_SUITE (player_tests)

BOOST_AUTO_TEST_CASE (simple_player_test)
{
    static exeTestRunner playerExe ("", "helics_player");

    static exeTestRunner brokerExe ("", "helics_broker");

    auto res = brokerExe.runAsync ("1 --type=ipc --name=ipc_broker");
    std::string exampleFile = std::string (TEST_DIR) + "/test_files/example1.player";
    auto res2 = playerExe.runCaptureOutputAsync ("--name=player --broker=ipc_broker --core=ipc " + exampleFile);

    auto val = res2.get ();
    auto val2 = res.get ();
    BOOST_CHECK_EQUAL (val2, 0);
    std::string compareString = "read file 24 points for 1 tags";
    BOOST_CHECK (val.compare (0, compareString.size (), compareString) == 0);
}

BOOST_AUTO_TEST_SUITE_END ()