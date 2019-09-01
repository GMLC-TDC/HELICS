/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/ValueFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
//#include "helics/core/CoreFactory.hpp"
#include "helics/common/logger.h"
#include "helics/core/Core.hpp"
#include "helics/core/core-exceptions.hpp"
#include "helics/core/helics_definitions.hpp"
#include "helics/external/filesystem.hpp"
#include <future>
#include <gmlc/libguarded/guarded.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/tools/floating_point_comparison.hpp>

/** these test cases test out the value converters
 */

#define CORE_TYPE_TO_TEST helics::core_type::TEST

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (logging_tests, *utf::label ("ci"))

BOOST_AUTO_TEST_CASE (basic_logging)
{
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker";
    fi.setProperty (helics::defs::log_level, 5);

    auto Fed = std::make_shared<helics::Federate> ("test1", fi);

    gmlc::libguarded::guarded<std::vector<std::pair<int, std::string>>> mlog;
    Fed->setLoggingCallback ([&mlog](int level, const std::string &, const std::string &message) {
        mlog.lock ()->emplace_back (level, message);
    });

    Fed->enterExecutingMode ();
    Fed->finalize ();

    BOOST_CHECK (!mlog.lock ()->empty ());
}

BOOST_AUTO_TEST_CASE (file_logging)
{
    helics::FederateInfo fi (CORE_TYPE_TO_TEST);
    fi.coreInitString = "--autobroker --logfile logfile.txt --fileloglevel=5";

    auto Fed = std::make_shared<helics::Federate> ("test1", fi);

    Fed->enterExecutingMode ();
    Fed->finalize ();
    auto cr = Fed->getCorePointer ();
    Fed.reset ();
    BOOST_CHECK (ghc::filesystem::exists ("logfile.txt"));
    cr->waitForDisconnect ();
    cr.reset ();
    std::error_code ec;
    bool res = ghc::filesystem::remove ("logfile.txt", ec);
    int ii = 0;
    while (!res)
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (100));
        res = ghc::filesystem::remove ("logfile.txt", ec);
        ++ii;
        if (ii > 15)
        {
            break;
        }
    }
    BOOST_CHECK (!res);
}

BOOST_AUTO_TEST_SUITE_END ()
