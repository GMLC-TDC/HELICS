/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "helics/common/MappedPointerVector.hpp"

BOOST_AUTO_TEST_SUITE (mapped_pointer_vector_tests)

/** test basic operations */
BOOST_AUTO_TEST_CASE (definition_tests)
{
    MappedPointerVector<double> M;
    MappedPointerVector<std::string> S2;
    BOOST_CHECK_EQUAL (M.size (), 0u);
    BOOST_CHECK_EQUAL (S2.size (), 0u);
    MappedPointerVector<std::vector<std::string>, double> V2;

    // test move operators
    decltype (M) TV2;
    TV2 = std::move (M);

    decltype (TV2) TV3 (std::move (TV2));
}

BOOST_AUTO_TEST_CASE (insertion_tests)
{
    MappedPointerVector<std::vector<double>> Mvec;
    Mvec.insert ("el1", 3, 1.7);
    BOOST_CHECK_EQUAL (Mvec.size (), 1);
    Mvec.insert ("a2", std::vector<double> (45));
    BOOST_CHECK_EQUAL (Mvec.size (), 2);
    auto V = Mvec[0];
    BOOST_CHECK_EQUAL (V->size (), 3);
    BOOST_CHECK_EQUAL ((*V)[0], 1.7);
    BOOST_CHECK_EQUAL ((*V)[2], 1.7);

    auto V2 = Mvec[1];
    BOOST_REQUIRE(V2 != nullptr);
    BOOST_CHECK_EQUAL (V2->size (), 45);

    auto V3 = Mvec.find ("el1");
    BOOST_REQUIRE(V3 != nullptr);
    BOOST_CHECK_EQUAL (V3->size (), 3);

    auto V4 = Mvec.find ("a2");
    BOOST_REQUIRE(V4 != nullptr);
    BOOST_CHECK_EQUAL (V4->size (), 45);
}

BOOST_AUTO_TEST_CASE (iterator_tests)
{
    MappedPointerVector<double> Mvec;

    Mvec.insert ("s1", 3.2);
    Mvec.insert ("s2", 4.3);
    Mvec.insert ("s3", 9.7);
    Mvec.insert ("s4", 11.4);

    BOOST_CHECK_EQUAL (Mvec.size (), 4);

    Mvec.apply ([](double *val) { *val = *val + 1; });
    BOOST_REQUIRE(Mvec[0] != nullptr);
    BOOST_CHECK_EQUAL (*Mvec[0], 3.2 + 1.0);
    BOOST_REQUIRE(Mvec[1] != nullptr);
    BOOST_CHECK_EQUAL (*Mvec[1], 4.3 + 1.0);
    BOOST_REQUIRE(Mvec[2] != nullptr);
    BOOST_CHECK_EQUAL (*Mvec[2], 9.7 + 1.0);
}

BOOST_AUTO_TEST_CASE (remove_tests)
{
    MappedPointerVector<double> Mvec;

    Mvec.insert ("s1", 3.2);
    Mvec.insert ("s2", 4.3);
    Mvec.insert ("s3", 9.7);
    Mvec.insert ("s4", 11.4);

    BOOST_CHECK_EQUAL (Mvec.size (), 4);

    Mvec.removeIndex (1);

    BOOST_CHECK_EQUAL (Mvec.size (), 3);
    BOOST_CHECK (Mvec.find ("s2") == nullptr);
    BOOST_REQUIRE(Mvec[1] != nullptr);
    BOOST_CHECK_EQUAL (*Mvec[1], 9.7);
    auto s4 = Mvec.find("s4");
    BOOST_REQUIRE(s4 != nullptr);
    BOOST_CHECK_EQUAL (*s4, 11.4);

    Mvec.remove ("s1");
    BOOST_CHECK_EQUAL (Mvec.size (), 2);
    s4 = Mvec.find("s4");
    BOOST_REQUIRE(s4 != nullptr);
    BOOST_CHECK_EQUAL (*Mvec.find ("s4"), 11.4);
    BOOST_REQUIRE(Mvec[0] != nullptr);
    BOOST_CHECK_EQUAL (*Mvec[0], 9.7);

    auto MV2 = std::move (Mvec);
    BOOST_CHECK_EQUAL (MV2.size (), 2);

    MV2.clear ();
    BOOST_CHECK_EQUAL (MV2.size (), 0);
}

BOOST_AUTO_TEST_SUITE_END ()
