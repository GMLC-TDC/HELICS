/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. 
SPDX-License-Identifier: BSD-3-Clause
*/
#include "testFixtures.h"
#include <boost/test/unit_test.hpp>

#include "helics/core/ActionMessage.hpp"
#include "helics/core/TimeCoordinator.hpp"

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (timeCoord_tests, *utf::label("ci"))
using namespace helics;

static constexpr global_federate_id fed2 (2);
static constexpr global_federate_id fed3 (3);
BOOST_AUTO_TEST_CASE (dependency_tests)
{
    TimeCoordinator ftc;
    ftc.addDependency (fed2);
    ftc.addDependency (fed3);
    
    auto deps = ftc.getDependencies ();
    BOOST_CHECK (deps.size () == 2);
    BOOST_CHECK (deps[0] == fed2);
    BOOST_CHECK (deps[1] == fed3);
    // test repeated inputs are dealt with correctly
    ftc.addDependency (fed3);
    deps = ftc.getDependencies ();
    BOOST_CHECK (deps.size () == 2);
    BOOST_CHECK (deps[0] == fed2);
    BOOST_CHECK (deps[1] == fed3);

    ftc.removeDependency (fed2);
    deps = ftc.getDependencies ();
    BOOST_CHECK (deps.size () == 1);
    BOOST_CHECK (deps[0] == fed3);
    // remove same one
    ftc.removeDependency (fed2);
    deps = ftc.getDependencies ();
    BOOST_CHECK (deps.size () == 1);
    BOOST_CHECK (deps[0] == fed3);
}

BOOST_AUTO_TEST_CASE (dependency_test_message)
{
    TimeCoordinator ftc;
    ActionMessage addDep (CMD_ADD_DEPENDENCY);
    addDep.source_id = fed2;

    ftc.processDependencyUpdateMessage (addDep);
    addDep.source_id = fed3;
    ftc.processDependencyUpdateMessage (addDep);
    auto deps = ftc.getDependencies ();
    BOOST_CHECK (deps.size () == 2);
    BOOST_CHECK (deps[0] ==fed2);
    BOOST_CHECK (deps[1] == fed3);
    // test redundancy checking
    ftc.processDependencyUpdateMessage (addDep);
    deps = ftc.getDependencies ();
    BOOST_CHECK (deps.size () == 2);
    BOOST_CHECK (deps[0] == fed2);
    BOOST_CHECK (deps[1] == fed3);

    ActionMessage remDep (CMD_REMOVE_DEPENDENCY);
    remDep.source_id = fed2;
    ftc.processDependencyUpdateMessage (remDep);
    deps = ftc.getDependencies ();
    BOOST_CHECK (deps.size () == 1);
    BOOST_CHECK (deps[0] == fed3);

    // remove unrecognized one
    remDep.source_id = global_federate_id(10);
    ftc.processDependencyUpdateMessage (remDep);
    deps = ftc.getDependencies ();
    BOOST_CHECK (deps.size () == 1);
    BOOST_CHECK (deps[0] == fed3);
}

BOOST_AUTO_TEST_CASE (dependent_tests)
{
    TimeCoordinator ftc;
    ftc.addDependent (fed2);
    ftc.addDependent (fed3);
    auto deps = ftc.getDependents ();
    BOOST_CHECK (deps.size () == 2);
    BOOST_CHECK (deps[0] == fed2);
    BOOST_CHECK (deps[1] == fed3);
    // test repeated inputs are dealt with correctly
    ftc.addDependent (fed3);
    // deps is a reference so it should change automatically
    BOOST_CHECK (deps.size () == 2);
    BOOST_CHECK (deps[0] == fed2);
    BOOST_CHECK (deps[1] == fed3);

    ftc.removeDependent (fed2);
    deps = ftc.getDependents();
    BOOST_CHECK (deps.size () == 1);
    BOOST_CHECK (deps[0] == fed3);
    // remove same one
    ftc.removeDependent (fed2);
    deps = ftc.getDependents();
    BOOST_CHECK (deps.size () == 1);
    BOOST_CHECK (deps[0] == fed3);
}

BOOST_AUTO_TEST_CASE (dependent_test_message)
{
    TimeCoordinator ftc;
    
    ActionMessage addDep (CMD_ADD_DEPENDENT);
    addDep.source_id = fed2;

    ftc.processDependencyUpdateMessage (addDep);
    addDep.source_id = fed3;
    ftc.processDependencyUpdateMessage (addDep);
    auto deps = ftc.getDependents ();
    BOOST_CHECK (deps.size () == 2);
    BOOST_CHECK (deps[0] == fed2);
    BOOST_CHECK (deps[1] == fed3);
    // test redundancy checking
    ftc.processDependencyUpdateMessage (addDep);
    deps = ftc.getDependents();
    BOOST_CHECK (deps.size () == 2);
    BOOST_CHECK (deps[0] == fed2);
    BOOST_CHECK (deps[1] == fed3);

    ActionMessage remDep (CMD_REMOVE_DEPENDENT);
    remDep.source_id = fed2;
    ftc.processDependencyUpdateMessage (remDep);
    deps = ftc.getDependents();
    BOOST_CHECK (deps.size () == 1);
    BOOST_CHECK (deps[0] == fed3);

    // remove unrecognized one
    remDep.source_id = global_federate_id(10);
    ftc.processDependencyUpdateMessage (remDep);
    deps = ftc.getDependents();
    BOOST_CHECK (deps.size () == 1);
    BOOST_CHECK (deps[0] == fed3);
}

BOOST_AUTO_TEST_SUITE_END ()
