/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "gmlc/libguarded/guarded.hpp"
#include <complex>
#include <boost/test/unit_test.hpp>

/** these test cases test out the value converters
 */
#include "ctestFixtures.hpp"

#include "helics/shared_api_library/helicsCallbacks.h"

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (logging_tests, *utf::label ("ci"))

using logblocktype = gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>>;
BOOST_AUTO_TEST_CASE (check_log_message)
{
    auto fi = helicsCreateFederateInfo ();
    auto err = helicsErrorInitialize ();
    helicsFederateInfoSetCoreType (fi, helics_core_type_test, &err);
    helicsFederateInfoSetCoreInitString (fi, "--autobroker", &err);
    helicsFederateInfoSetIntegerProperty (fi, helics_property_int_log_level, 5, &err);

    auto fed = helicsCreateValueFederate ("test1", fi, &err);

    logblocktype mlog;

    auto logg = [] (int level, const char *, const char *message, void *udata) {
        auto *mp = reinterpret_cast<logblocktype *> (udata);
        mp->lock ()->emplace_back (level, message);
    };

    helicsFederateAddLoggingCallback (fed, logg, &mlog, &err);

    BOOST_CHECK_EQUAL (err.error_code, 0);

    helicsFederateEnterExecutingMode (fed, &err);
    helicsFederateLogInfoMessage (fed, "test MEXAGE", &err);
    helicsFederateRequestNextStep (fed, &err);
    helicsFederateFinalize (fed, &err);
    BOOST_CHECK_EQUAL (err.error_code, 0);
    auto llock = mlog.lock ();
    bool found = false;
    for (auto &m : llock)
    {
        if (m.second.find ("MEXAGE") != std::string::npos)
        {
            found = true;
        }
    }
    BOOST_CHECK (found);
}

BOOST_AUTO_TEST_CASE (check_log_message_levels)
{
    auto fi = helicsCreateFederateInfo ();
    auto err = helicsErrorInitialize ();
    helicsFederateInfoSetCoreType (fi, helics_core_type_test, &err);
    helicsFederateInfoSetCoreInitString (fi, "--autobroker", &err);
    helicsFederateInfoSetIntegerProperty (fi, helics_property_int_log_level, 5, &err);

    auto fed = helicsCreateValueFederate ("test1", fi, &err);

    logblocktype mlog;

    auto logg = [] (int level, const char *, const char *message, void *udata) {
        auto *mp = reinterpret_cast<logblocktype *> (udata);
        mp->lock ()->emplace_back (level, message);
    };

    helicsFederateAddLoggingCallback (fed, logg, &mlog, &err);

    BOOST_CHECK_EQUAL (err.error_code, 0);

    helicsFederateEnterExecutingMode (fed, &err);
    helicsFederateLogLevelMessage (fed, 3, "test MEXAGE1", &err);
    helicsFederateLogLevelMessage (fed, 8, "test MEXAGE2", &err);
    helicsFederateRequestNextStep (fed, &err);
    helicsFederateFinalize (fed, &err);
    BOOST_CHECK_EQUAL (err.error_code, 0);

    auto llock = mlog.lock ();
    bool found_low = false;
    bool found_high = false;
    for (auto &m : llock)
    {
        if (m.second.find ("MEXAGE1") != std::string::npos)
        {
            found_low = true;
        }
        if (m.second.find ("MEXAGE2") != std::string::npos)
        {
            found_high = true;
        }
    }
    BOOST_CHECK (found_low && !found_high);
}

BOOST_AUTO_TEST_CASE (check_log_message_levels_high)
{
    auto fi = helicsCreateFederateInfo ();
    auto err = helicsErrorInitialize ();
    helicsFederateInfoSetCoreType (fi, helics_core_type_test, &err);
    helicsFederateInfoSetCoreInitString (fi, "--autobroker", &err);
    helicsFederateInfoSetIntegerProperty (fi, helics_property_int_log_level, 9, &err);

    auto fed = helicsCreateValueFederate ("test1", fi, &err);

    logblocktype mlog;

    auto logg = [] (int level, const char *, const char *message, void *udata) {
        auto *mp = reinterpret_cast<logblocktype *> (udata);
        mp->lock ()->emplace_back (level, message);
    };

    helicsFederateAddLoggingCallback (fed, logg, &mlog, &err);

    BOOST_CHECK_EQUAL (err.error_code, 0);

    helicsFederateEnterExecutingMode (fed, &err);
    helicsFederateLogLevelMessage (fed, 3, "test MEXAGE1", &err);
    helicsFederateLogLevelMessage (fed, 8, "test MEXAGE2", &err);
    helicsFederateRequestNextStep (fed, &err);
    helicsFederateFinalize (fed, &err);
    BOOST_CHECK_EQUAL (err.error_code, 0);

    auto llock = mlog.lock ();
    bool found_low = false;
    bool found_high = false;
    for (auto &m : llock)
    {
        if (m.second.find ("MEXAGE1") != std::string::npos)
        {
            found_low = true;
        }
        if (m.second.find ("MEXAGE2") != std::string::npos)
        {
            found_high = true;
        }
    }
    BOOST_CHECK (found_low && found_high);
}

BOOST_AUTO_TEST_SUITE_END ()
