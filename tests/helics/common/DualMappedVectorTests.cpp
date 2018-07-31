/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "helics/common/DualMappedVector.hpp"

BOOST_AUTO_TEST_SUITE (dual_mapped_vector_tests)

/** test basic operations */
BOOST_AUTO_TEST_CASE (definition_tests)
{
    DualMappedVector<double, std::string, int64_t> M;
    DualMappedVector<std::string, std::string, double> S2;
    BOOST_CHECK_EQUAL (M.size (), 0u);
    BOOST_CHECK_EQUAL (S2.size (), 0u);
    DualMappedVector<std::vector<std::string>, double, std::string> V2;

    // test move and assignment operators
    decltype (M) TV2;
    TV2 = std::move (M);

    decltype (TV2) TV3;
    TV3 = TV2;
}

BOOST_AUTO_TEST_CASE (insertion_tests)
{
    DualMappedVector<std::vector<double>, std::string, int64_t> Mvec;
    Mvec.insert ("el1", 41, 3, 1.7);
    BOOST_CHECK_EQUAL (Mvec.size (), 1u);
    Mvec.insert ("a2", 27, std::vector<double> (45));
    BOOST_CHECK_EQUAL (Mvec.size (), 2u);
    auto &V = Mvec[0];
    BOOST_CHECK_EQUAL (V.size (), 3u);
    BOOST_CHECK_EQUAL (V[0], 1.7);
    BOOST_CHECK_EQUAL (V[2], 1.7);

    auto &V2 = Mvec[1];
    BOOST_CHECK_EQUAL (V2.size (), 45u);

    auto V3 = Mvec.find ("el1");
    BOOST_CHECK_EQUAL (V3->size (), 3u);

    auto V4 = Mvec.find ("a2");
    BOOST_CHECK_EQUAL (V4->size (), 45u);

    auto V5 = Mvec.find (41);
    BOOST_CHECK_EQUAL (V5->size (), 3u);
}

BOOST_AUTO_TEST_CASE (additional_searchTerm_tests)
{
    DualMappedVector<double, std::string, int64_t> Mvec;

    Mvec.insert ("s1", 64, 3.2);
    Mvec.insert ("s2", 63, 4.3);
    Mvec.insert ("s3", 47, 9.7);
    Mvec.insert ("s4", 92, 11.4);

    BOOST_CHECK_EQUAL (Mvec.size (), 4u);

    Mvec.addSearchTerm ("s5", "s1");
    BOOST_CHECK_EQUAL (Mvec.size (), 4u);

    BOOST_CHECK_EQUAL (*(Mvec.find ("s5")), 3.2);

    Mvec.addSearchTerm (93, 47);
    BOOST_CHECK_EQUAL (*(Mvec.find (93)), 9.7);

    Mvec.addSearchTerm (143, "s3");
    BOOST_CHECK_EQUAL (*(Mvec.find (143)), 9.7);

    Mvec.addSearchTerm ("extra", 63);
    BOOST_CHECK_EQUAL (*(Mvec.find ("extra")), 4.3);

    Mvec.addSearchTermForIndex ("astring", 3);
    BOOST_CHECK_EQUAL (*(Mvec.find ("astring")), 11.4);

    auto res = Mvec.addSearchTermForIndex (99, 2);
    BOOST_CHECK_EQUAL (*(Mvec.find (99)), 9.7);
    BOOST_CHECK (res);

    // check for appropriate return values
    BOOST_CHECK (!Mvec.addSearchTerm ("missing", "none"));
    BOOST_CHECK (!Mvec.addSearchTerm (1241, 98));
    BOOST_CHECK (!Mvec.addSearchTerm ("missing", 98));
    BOOST_CHECK (!Mvec.addSearchTerm (1241, "none"));
}

BOOST_AUTO_TEST_CASE (iterator_tests)
{
    DualMappedVector<double, std::string, int64_t> Mvec;

    Mvec.insert ("s1", 64, 3.2);
    Mvec.insert ("s2", 63, 4.3);
    Mvec.insert ("s3", 47, 9.7);
    Mvec.insert ("s4", 92, 11.4);

    BOOST_CHECK_EQUAL (Mvec.size (), 4u);

    Mvec.transform ([](double val) { return val + 1; });

    BOOST_CHECK_EQUAL (Mvec[0], 3.2 + 1.0);
    BOOST_CHECK_EQUAL (Mvec[1], 4.3 + 1.0);
    BOOST_CHECK_EQUAL (Mvec[2], 9.7 + 1.0);

    double sum = 0.0;
    for (auto &el : Mvec)
    {
        sum += el;
    }
    BOOST_CHECK_EQUAL (sum, 4.2 + 5.3 + 10.7 + 12.4);
}

BOOST_AUTO_TEST_CASE (remove_tests)
{
    DualMappedVector<double, std::string, int64_t> Mvec;

    Mvec.insert ("s1", 64, 3.2);
    Mvec.insert ("s2", 63, 4.3);
    Mvec.insert ("s3", 47, 9.7);
    Mvec.insert ("s4", 92, 11.4);

    Mvec.addSearchTerm ("s5", 64);
    Mvec.addSearchTermForIndex (107, 2);
    BOOST_CHECK_EQUAL (Mvec.size (), 4u);

    Mvec.removeIndex (1);

    BOOST_CHECK_EQUAL (Mvec.size (), 3u);
    BOOST_CHECK (Mvec.find ("s2") == Mvec.end ());
    BOOST_CHECK_EQUAL (Mvec[1], 9.7);
    BOOST_CHECK_EQUAL (*Mvec.find ("s4"), 11.4);
    BOOST_CHECK_EQUAL (*Mvec.find ("s5"), 3.2);
    Mvec.remove ("s1");
    BOOST_CHECK_EQUAL (Mvec.size (), 2u);
    BOOST_CHECK_EQUAL (*Mvec.find ("s4"), 11.4);
    BOOST_CHECK_EQUAL (Mvec[0], 9.7);
    BOOST_CHECK (Mvec.find ("s5") == Mvec.end ());
    BOOST_CHECK_EQUAL (*Mvec.find (107), 9.7);

    auto MV2 = std::move (Mvec);
    BOOST_CHECK_EQUAL (MV2.size (), 2u);

    auto MV3 = MV2;
    BOOST_CHECK_EQUAL (MV2.size (), 2u);
    BOOST_CHECK_EQUAL (MV3.size (), 2u);

    MV3.remove (92);
    BOOST_CHECK_EQUAL (MV2.size (), 2u);
    BOOST_CHECK_EQUAL (MV3.size (), 1u);
    MV3.clear ();
    BOOST_CHECK_EQUAL (MV2.size (), 2u);
    BOOST_CHECK_EQUAL (MV3.size (), 0u);
}

BOOST_AUTO_TEST_SUITE_END ()
