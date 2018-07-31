/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "helics/common/DualMappedPointerVector.hpp"

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE (dual_mapped_pointer_vector_tests)

/** test basic operations */
BOOST_AUTO_TEST_CASE (definition_tests)
{
    DualMappedPointerVector<double, std::string, int64_t> M;
    DualMappedPointerVector<std::string, std::string, double> S2;
    BOOST_CHECK_EQUAL (M.size (), 0u);
    BOOST_CHECK_EQUAL (S2.size (), 0u);
    DualMappedPointerVector<std::vector<std::string>, double, std::string> V2;

    // test move operators
    decltype (M) TV2;
    TV2 = std::move (M);

    decltype (TV2) TV3 (std::move (TV2));
}

BOOST_AUTO_TEST_CASE (insertion_tests)
{
    DualMappedPointerVector<std::vector<double>, std::string, int64_t> Mvec;
    Mvec.insert ("el1", 41, 3, 1.7);
    BOOST_CHECK_EQUAL (Mvec.size (), 1);
    Mvec.insert ("a2", 27, std::vector<double> (45));
    BOOST_CHECK_EQUAL (Mvec.size (), 2);
    auto V = Mvec[0];
    BOOST_CHECK_EQUAL (V->size (), 3);
    BOOST_CHECK_EQUAL ((*V)[0], 1.7);
    BOOST_CHECK_EQUAL ((*V)[2], 1.7);

    auto V2 = Mvec[1];
    BOOST_CHECK_EQUAL (V2->size (), 45);

    auto V3 = Mvec.find ("el1");
    BOOST_CHECK_EQUAL (V3->size (), 3);

    auto V4 = Mvec.find ("a2");
    BOOST_CHECK_EQUAL (V4->size (), 45);

    auto V5 = Mvec.find (41);
    BOOST_CHECK_EQUAL (V5->size (), 3);
}

BOOST_AUTO_TEST_CASE (iterator_tests)
{
    DualMappedPointerVector<double, std::string, int64_t> Mvec;

    Mvec.insert ("s1", 64, 3.2);
    Mvec.insert ("s2", 63, 4.3);
    Mvec.insert ("s3", 47, 9.7);
    Mvec.insert ("s4", 92, 11.4);

    BOOST_CHECK_EQUAL (Mvec.size (), 4);

    Mvec.apply ([](double *val) { *val = *val + 1; });

    BOOST_CHECK_EQUAL (*Mvec[0], 3.2 + 1.0);
    BOOST_CHECK_EQUAL (*Mvec[1], 4.3 + 1.0);
    BOOST_CHECK_EQUAL (*Mvec[2], 9.7 + 1.0);
}

BOOST_AUTO_TEST_CASE (remove_tests)
{
    DualMappedPointerVector<double, std::string, int64_t> Mvec;

    Mvec.insert ("s1", 64, 3.2);
    Mvec.insert ("s2", 63, 4.3);
    Mvec.insert ("s3", 47, 9.7);
    Mvec.insert ("s4", 92, 11.4);

    BOOST_CHECK_EQUAL (Mvec.size (), 4);

    Mvec.removeIndex (1);

    BOOST_CHECK_EQUAL (Mvec.size (), 3);
    BOOST_CHECK (Mvec.find ("s2") == nullptr);
    BOOST_CHECK (Mvec.find (63) == nullptr);
    BOOST_CHECK_EQUAL (*Mvec[1], 9.7);
    BOOST_CHECK_EQUAL (*Mvec.find ("s4"), 11.4);

    Mvec.remove ("s1");
    BOOST_CHECK_EQUAL (Mvec.size (), 2);
    BOOST_CHECK_EQUAL (*Mvec.find ("s4"), 11.4);
    BOOST_CHECK_EQUAL (*Mvec[0], 9.7);

    auto MV2 = std::move (Mvec);
    BOOST_CHECK_EQUAL (MV2.size (), 2);

    auto MV3 = std::move (MV2);
    BOOST_CHECK_EQUAL (MV3.size (), 2);

    MV3.remove (92);
    BOOST_CHECK_EQUAL (MV3.size (), 1);
    MV3.clear ();
    BOOST_CHECK_EQUAL (MV3.size (), 0);
}

BOOST_AUTO_TEST_SUITE_END ()
