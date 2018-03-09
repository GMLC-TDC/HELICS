/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "testFixtures.h"
#include <boost/test/unit_test.hpp>

#include "helics/core/ForwardingTimeCoordinator.hpp"
#include "helics/core/ActionMessage.hpp"

BOOST_AUTO_TEST_SUITE(ftc_tests)
using namespace helics;

BOOST_AUTO_TEST_CASE(dependency_tests)
{

    ForwardingTimeCoordinator ftc;
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

    ForwardingTimeCoordinator ftc;
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

    ForwardingTimeCoordinator ftc;
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

    ForwardingTimeCoordinator ftc;
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

BOOST_AUTO_TEST_CASE(execMode_entry)
{
    ForwardingTimeCoordinator ftc;
    ftc.addDependency(2);
    ftc.addDependency(3);
    ftc.enteringExecMode();
    //iteration_state
    auto ret=ftc.checkExecEntry();
    BOOST_CHECK(ret == iteration_state::continue_processing);

    ActionMessage execReady(CMD_EXEC_REQUEST);
    execReady.source_id = 2;
    auto modified=ftc.processTimeMessage(execReady);
    BOOST_CHECK(modified);
    ret = ftc.checkExecEntry();
    BOOST_CHECK(ret == iteration_state::continue_processing);
    execReady.source_id = 3;
    modified = ftc.processTimeMessage(execReady);
    BOOST_CHECK(modified);
    ret = ftc.checkExecEntry();
    BOOST_CHECK(ret == iteration_state::next_step);
}


void getFTCtoExecMode(ForwardingTimeCoordinator &ftc)
{
    ActionMessage execReady(CMD_EXEC_REQUEST);
    auto depList = ftc.getDependencies();
    for (auto dep : depList)
    {
        execReady.source_id = dep;
        ftc.processTimeMessage(execReady);
    }
    auto ret = ftc.checkExecEntry();
    BOOST_CHECK(ret == iteration_state::next_step);

    ActionMessage execGrant(CMD_EXEC_GRANT);
    for (auto dep : depList)
    {
        execReady.source_id = dep;
        ftc.processTimeMessage(execReady);
    }
    ftc.updateTimeFactors();
}

BOOST_AUTO_TEST_CASE(timing_test1)
{
    ForwardingTimeCoordinator ftc;
    ftc.addDependency(2);
    ftc.addDependency(3);
    getFTCtoExecMode(ftc);

    ftc.addDependent(5);
    ActionMessage lastMessage(CMD_INVALID);
    ftc.source_id = 1;
    ftc.setMessageSender([&lastMessage](const helics::ActionMessage &mess) {lastMessage = mess; });

    ActionMessage timeUpdate(CMD_TIME_REQUEST, 2,1);
    timeUpdate.actionTime = 1.0;
    timeUpdate.Te = 1.0;
    timeUpdate.Tdemin = 1.0;

    auto modified = ftc.processTimeMessage(timeUpdate);
    BOOST_CHECK(modified);
    ftc.updateTimeFactors();
    //there should be no update yet
    BOOST_CHECK(lastMessage.action()==CMD_INVALID);

    timeUpdate.source_id = 3;
    timeUpdate.actionTime = 0.5;
    modified = ftc.processTimeMessage(timeUpdate);
    BOOST_CHECK(modified);
    ftc.updateTimeFactors();
    BOOST_CHECK_EQUAL(lastMessage.actionTime, 0.5);
    BOOST_CHECK_EQUAL(lastMessage.Te, 1.0);
    BOOST_CHECK_EQUAL(lastMessage.Tdemin, 1.0);
    BOOST_CHECK(lastMessage.action() == CMD_TIME_REQUEST);

}

BOOST_AUTO_TEST_SUITE_END()

