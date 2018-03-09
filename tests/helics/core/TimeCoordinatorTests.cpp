/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "testFixtures.h"
#include <boost/test/unit_test.hpp>

#include "helics/core/TimeCoordinator.hpp"
#include "helics/core/ActionMessage.hpp"

BOOST_AUTO_TEST_SUITE(timeCoord_tests)
using namespace helics;

BOOST_AUTO_TEST_CASE(dependency_tests)
{

    TimeCoordinator ftc;
    ftc.addDependency(2);
    ftc.addDependency(3);
    auto deps = ftc.getDependencies();
    BOOST_CHECK(deps.size() == 2);
    BOOST_CHECK(deps[0] == 2);
    BOOST_CHECK(deps[1] == 3);
    // test repeated inputs are dealt with correctly
    ftc.addDependency(3);
    deps = ftc.getDependencies();
    BOOST_CHECK(deps.size() == 2);
    BOOST_CHECK(deps[0] == 2);
    BOOST_CHECK(deps[1] == 3);

    ftc.removeDependency(2);
    deps = ftc.getDependencies();
    BOOST_CHECK(deps.size() == 1);
    BOOST_CHECK(deps[0] == 3);
    //remove same one
    ftc.removeDependency(2);
    deps = ftc.getDependencies();
    BOOST_CHECK(deps.size() == 1);
    BOOST_CHECK(deps[0] == 3);

}

BOOST_AUTO_TEST_CASE(dependency_test_message)
{

    TimeCoordinator ftc;
    ActionMessage addDep(CMD_ADD_DEPENDENCY);
    addDep.source_id = 2;

    ftc.processDependencyUpdateMessage(addDep);
    addDep.source_id = 3;
    ftc.processDependencyUpdateMessage(addDep);
    auto deps = ftc.getDependencies();
    BOOST_CHECK(deps.size() == 2);
    BOOST_CHECK(deps[0] == 2);
    BOOST_CHECK(deps[1] == 3);
    //test redundency checking
    ftc.processDependencyUpdateMessage(addDep);
    deps = ftc.getDependencies();
    BOOST_CHECK(deps.size() == 2);
    BOOST_CHECK(deps[0] == 2);
    BOOST_CHECK(deps[1] == 3);

    ActionMessage remDep(CMD_REMOVE_DEPENDENCY);
    remDep.source_id = 2;
    ftc.processDependencyUpdateMessage(remDep);
    deps = ftc.getDependencies();
    BOOST_CHECK(deps.size() == 1);
    BOOST_CHECK(deps[0] == 3);

    //remove unrecognized one
    remDep.source_id = 10;
    ftc.processDependencyUpdateMessage(remDep);
    deps = ftc.getDependencies();
    BOOST_CHECK(deps.size() == 1);
    BOOST_CHECK(deps[0] == 3);

}

BOOST_AUTO_TEST_CASE(dependent_tests)
{

    TimeCoordinator ftc;
    ftc.addDependent(2);
    ftc.addDependent(3);
    auto &deps = ftc.getDependents();
    BOOST_CHECK(deps.size() == 2);
    BOOST_CHECK(deps[0] == 2);
    BOOST_CHECK(deps[1] == 3);
    // test repeated inputs are dealt with correctly
    ftc.addDependent(3);
    //deps is a reference so it should change automatically
    BOOST_CHECK(deps.size() == 2);
    BOOST_CHECK(deps[0] == 2);
    BOOST_CHECK(deps[1] == 3);

    ftc.removeDependent(2);
    BOOST_CHECK(deps.size() == 1);
    BOOST_CHECK(deps[0] == 3);
    //remove same one
    ftc.removeDependent(2);

    BOOST_CHECK(deps.size() == 1);
    BOOST_CHECK(deps[0] == 3);

}

BOOST_AUTO_TEST_CASE(dependent_test_message)
{

    TimeCoordinator ftc;
    ActionMessage addDep(CMD_ADD_DEPENDENT);
    addDep.source_id = 2;

    ftc.processDependencyUpdateMessage(addDep);
    addDep.source_id = 3;
    ftc.processDependencyUpdateMessage(addDep);
    auto &deps = ftc.getDependents();
    BOOST_CHECK(deps.size() == 2);
    BOOST_CHECK(deps[0] == 2);
    BOOST_CHECK(deps[1] == 3);
    //test redundency checking
    ftc.processDependencyUpdateMessage(addDep);
    BOOST_CHECK(deps.size() == 2);
    BOOST_CHECK(deps[0] == 2);
    BOOST_CHECK(deps[1] == 3);

    ActionMessage remDep(CMD_REMOVE_DEPENDENT);
    remDep.source_id = 2;
    ftc.processDependencyUpdateMessage(remDep);

    BOOST_CHECK(deps.size() == 1);
    BOOST_CHECK(deps[0] == 3);

    //remove unrecognized one
    remDep.source_id = 10;
    ftc.processDependencyUpdateMessage(remDep);
    BOOST_CHECK(deps.size() == 1);
    BOOST_CHECK(deps[0] == 3);

}




BOOST_AUTO_TEST_SUITE_END()

