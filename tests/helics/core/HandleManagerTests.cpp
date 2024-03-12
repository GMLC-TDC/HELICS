/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/core/HandleManager.hpp"

#include "gtest/gtest.h"

using namespace helics;

static constexpr GlobalFederateId fed2(2);
static constexpr GlobalFederateId fed3(3);

static constexpr InterfaceHandle i1(1);
static constexpr InterfaceHandle i2(2);
static constexpr InterfaceHandle i3(3);
static constexpr InterfaceHandle i4(4);
static constexpr InterfaceHandle i5(5);

TEST(handleManager, creationEndpoint)
{
    HandleManager h1;
    auto bi1 = h1.addHandle(fed2, i1, InterfaceType::ENDPOINT, "key1", "type1", "");
    EXPECT_EQ(bi1.key, "key1");
    EXPECT_EQ(bi1.getFederateId(), fed2);
    EXPECT_EQ(bi1.getInterfaceHandle(), i1);
    EXPECT_EQ(bi1.type, "type1");
    EXPECT_EQ(bi1.handleType, InterfaceType::ENDPOINT);
}

TEST(handleManager, creationPublication)
{
    HandleManager h1;
    auto bi1 = h1.addHandle(fed2, InterfaceType::PUBLICATION, "key1", "type1", "V");
    EXPECT_EQ(bi1.key, "key1");
    EXPECT_EQ(bi1.getFederateId(), fed2);
    EXPECT_EQ(bi1.type, "type1");
    EXPECT_EQ(bi1.handleType, InterfaceType::PUBLICATION);
    EXPECT_EQ(bi1.units, "V");
}

TEST(handleManager, creationInput)
{
    HandleManager h1;
    auto bi1 = h1.addHandle(fed2, InterfaceType::INPUT, "key1", "type1", "V");
    EXPECT_EQ(bi1.key, "key1");
    EXPECT_EQ(bi1.getFederateId(), fed2);
    EXPECT_EQ(bi1.type, "type1");
    EXPECT_EQ(bi1.handleType, InterfaceType::INPUT);
    EXPECT_EQ(bi1.units, "V");
}

TEST(handleManager, creationFilter)
{
    HandleManager h1;
    auto bi1 = h1.addHandle(fed2, InterfaceType::FILTER, "key1", "type1", "type2");
    EXPECT_EQ(bi1.key, "key1");
    EXPECT_EQ(bi1.getFederateId(), fed2);
    EXPECT_EQ(bi1.type_in, "type1");
    EXPECT_EQ(bi1.handleType, InterfaceType::FILTER);
    EXPECT_EQ(bi1.type_out, "type2");
}

TEST(handleManager, creationTranslator)
{
    HandleManager h1;
    auto bi1 = h1.addHandle(fed2, i2, InterfaceType::TRANSLATOR, "key1", "type1", "type2");
    EXPECT_EQ(bi1.key, "key1");
    EXPECT_EQ(bi1.getFederateId(), fed2);
    EXPECT_EQ(bi1.getInterfaceHandle(), i2);
    EXPECT_EQ(bi1.type_in, "type1");
    EXPECT_EQ(bi1.handleType, InterfaceType::TRANSLATOR);
    EXPECT_EQ(bi1.type_out, "type2");
    EXPECT_EQ(bi1.units, "type2");
}

static HandleManager generateExampleHandleManager()
{
    HandleManager h1;
    h1.addHandle(fed2, i1, InterfaceType::PUBLICATION, "p1", "double", "V");
    h1.addHandle(fed2, i2, InterfaceType::PUBLICATION, "p2", "double", "V");
    h1.addHandle(fed2, i3, InterfaceType::INPUT, "in1", "double", "V");
    h1.addHandle(fed2, i4, InterfaceType::ENDPOINT, "e1", "t1", "");
    h1.addHandle(fed2, i5, InterfaceType::TRANSLATOR, "t1", "type", "V");
    h1.addHandle(fed3, i1, InterfaceType::FILTER, "f1", "type1", "type2");

    h1.addHandle(fed3, InterfaceType::INPUT, "in2", "double", "V");
    h1.addHandle(fed3, InterfaceType::ENDPOINT, "e2", "t1", "");
    h1.addHandle(fed3, InterfaceType::FILTER, "f2", "type1", "type2");
    h1.addHandle(fed3, InterfaceType::TRANSLATOR, "t2", "type", "V");
    return h1;
}

TEST(handleManager, finding)
{
    auto h1 = generateExampleHandleManager();

    EXPECT_EQ(h1.size(), 10U);
    const auto* ep = h1.getInterfaceHandle("e1", InterfaceType::ENDPOINT);
    EXPECT_NE(ep, nullptr);

    const auto* p1 = h1.getInterfaceHandle("p1", InterfaceType::PUBLICATION);
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(p1->key, "p1");

    const auto* in1 = h1.getInterfaceHandle("in1", InterfaceType::INPUT);
    EXPECT_NE(in1, nullptr);

    const auto* f1 = h1.getInterfaceHandle("f1", InterfaceType::FILTER);
    EXPECT_NE(f1, nullptr);

    const auto* tr1 = h1.getInterfaceHandle("t1", InterfaceType::TRANSLATOR);
    EXPECT_NE(tr1, nullptr);
}

TEST(handleManager, notfinding)
{
    auto h1 = generateExampleHandleManager();

    const auto* ep = h1.getInterfaceHandle("e1", InterfaceType::PUBLICATION);
    EXPECT_EQ(ep, nullptr);

    const auto* p1 = h1.getInterfaceHandle("p1", InterfaceType::ENDPOINT);
    EXPECT_EQ(p1, nullptr);

    const auto* in1 = h1.getInterfaceHandle("in1", InterfaceType::TRANSLATOR);
    EXPECT_EQ(in1, nullptr);

    const auto* f1 = h1.getInterfaceHandle("f1", InterfaceType::INPUT);
    EXPECT_EQ(f1, nullptr);

    const auto* tr1 = h1.getInterfaceHandle("t1", InterfaceType::FILTER);
    EXPECT_EQ(tr1, nullptr);
}

TEST(handleManager, constFinding)
{
    const auto h1 = generateExampleHandleManager();

    EXPECT_EQ(h1.size(), 10U);
    const auto* ep = h1.getInterfaceHandle("e1", InterfaceType::ENDPOINT);
    EXPECT_NE(ep, nullptr);

    const auto* p1 = h1.getInterfaceHandle("p1", InterfaceType::PUBLICATION);
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(p1->key, "p1");

    const auto* in1 = h1.getInterfaceHandle("in1", InterfaceType::INPUT);
    EXPECT_NE(in1, nullptr);

    const auto* f1 = h1.getInterfaceHandle("f1", InterfaceType::FILTER);
    EXPECT_NE(f1, nullptr);

    const auto* tr1 = h1.getInterfaceHandle("t1", InterfaceType::TRANSLATOR);
    EXPECT_NE(tr1, nullptr);
}

TEST(handleManager, constNotfinding)
{
    const auto h1 = generateExampleHandleManager();

    const auto* ep = h1.getInterfaceHandle("e1", InterfaceType::PUBLICATION);
    EXPECT_EQ(ep, nullptr);

    const auto* p1 = h1.getInterfaceHandle("p1", InterfaceType::ENDPOINT);
    EXPECT_EQ(p1, nullptr);

    const auto* in1 = h1.getInterfaceHandle("in1", InterfaceType::TRANSLATOR);
    EXPECT_EQ(in1, nullptr);

    const auto* f1 = h1.getInterfaceHandle("f1", InterfaceType::INPUT);
    EXPECT_EQ(f1, nullptr);

    const auto* tr1 = h1.getInterfaceHandle("t1", InterfaceType::FILTER);
    EXPECT_EQ(tr1, nullptr);
}

TEST(handleManager, translatorfinding)
{
    // translators should map to all the types except filter
    auto h1 = generateExampleHandleManager();
    const auto* in1 = h1.getInterfaceHandle("t1", InterfaceType::INPUT);
    EXPECT_NE(in1, nullptr);
    const auto* p1 = h1.getInterfaceHandle("t1", InterfaceType::PUBLICATION);
    EXPECT_NE(p1, nullptr);
    const auto* e1 = h1.getInterfaceHandle("t1", InterfaceType::ENDPOINT);
    EXPECT_NE(e1, nullptr);

    EXPECT_EQ(e1, p1);
    EXPECT_EQ(p1, in1);
}

TEST(handleManager, iterators)
{
    auto h1 = generateExampleHandleManager();
    auto itbegin = h1.begin();
    auto itend = h1.end();
    EXPECT_NE(itbegin, itend);
    int cnt{0};
    for (auto it = itbegin; it != itend; ++it) {
        EXPECT_EQ(&(*it), h1.getHandleInfo(cnt));
        EXPECT_EQ(it->handle, h1[cnt].handle);
        ++cnt;
    }
}

TEST(handleManager, constIterators)
{
    const auto h1 = generateExampleHandleManager();
    auto itbegin = h1.begin();
    auto itend = h1.end();
    EXPECT_NE(itbegin, itend);
    int cnt{0};
    for (auto it = itbegin; it != itend; ++it) {
        EXPECT_EQ(&(*it), h1.getHandleInfo(cnt));
        EXPECT_EQ(it->handle, h1[cnt].handle);
        ++cnt;
    }
}

TEST(handleManager, getByHandle)
{
    auto h1 = generateExampleHandleManager();
    auto& bi1 = h1.addHandle(fed2, InterfaceType::FILTER, "key1", "type1", "type2");

    auto* bi2 = h1.getHandleInfo(bi1.getInterfaceHandle());
    EXPECT_EQ(&bi1, bi2);

    auto* t2 = h1.findHandle(GlobalHandle(fed2, i1));
    EXPECT_EQ(t2->key, "p1");
}

TEST(handleManager, constGetByHandle)
{
    auto h1 = generateExampleHandleManager();
    auto& bi1 = h1.addHandle(fed2, InterfaceType::FILTER, "key1", "type1", "type2");

    const auto& h2 = h1;

    const auto* bi2 = h2.getHandleInfo(bi1.getInterfaceHandle());
    EXPECT_EQ(&bi1, bi2);

    const auto* t2 = h2.findHandle(GlobalHandle(fed2, i1));
    EXPECT_EQ(t2->key, "p1");
}

TEST(handleManager, alias1)
{
    auto h1 = generateExampleHandleManager();
    h1.addAlias("p1", "pub1");

    const auto* p1 = h1.getInterfaceHandle("pub1", InterfaceType::PUBLICATION);

    EXPECT_EQ(p1->key, "p1");

    h1.addAlias("in1", "input1");

    p1 = h1.getInterfaceHandle("input1", InterfaceType::INPUT);

    EXPECT_EQ(p1->key, "in1");

    h1.addAlias("e1", "end1");

    p1 = h1.getInterfaceHandle("end1", InterfaceType::ENDPOINT);

    EXPECT_EQ(p1->key, "e1");

    h1.addAlias("f1", "filt1");

    p1 = h1.getInterfaceHandle("filt1", InterfaceType::FILTER);

    EXPECT_EQ(p1->key, "f1");

    h1.addAlias("t1", "translator1");

    p1 = h1.getInterfaceHandle("translator1", InterfaceType::ENDPOINT);

    EXPECT_EQ(p1->key, "t1");
    p1 = h1.getInterfaceHandle("translator1", InterfaceType::INPUT);

    EXPECT_EQ(p1->key, "t1");
    p1 = h1.getInterfaceHandle("translator1", InterfaceType::PUBLICATION);

    EXPECT_EQ(p1->key, "t1");
}

TEST(handleManager, multipleAlias)
{
    auto h1 = generateExampleHandleManager();
    h1.addAlias("p1", "pub1");

    const auto* p1 = h1.getInterfaceHandle("pub1", InterfaceType::PUBLICATION);

    EXPECT_EQ(p1->key, "p1");

    h1.addAlias("p1", "pubA1");

    p1 = h1.getInterfaceHandle("pubA1", InterfaceType::PUBLICATION);

    EXPECT_EQ(p1->key, "p1");

    h1.addAlias("p1", "publication1");

    p1 = h1.getInterfaceHandle("publication1", InterfaceType::PUBLICATION);

    EXPECT_EQ(p1->key, "p1");

    h1.addAlias("p1", "publisher1");

    p1 = h1.getInterfaceHandle("publisher1", InterfaceType::PUBLICATION);

    EXPECT_EQ(p1->key, "p1");
}

// convenience function for simplifying adding a handle
static auto addTestHandle(HandleManager& hm, std::string_view name, InterfaceType type)
{
    return hm.addHandle(fed2, type, name, "type1", "");
}

TEST(handleManager, futureAlias)
{
    auto h1 = generateExampleHandleManager();
    h1.addAlias("pub1", "publisher");

    addTestHandle(h1, "pub1", InterfaceType::PUBLICATION);

    const auto* p1 = h1.getInterfaceHandle("publisher", InterfaceType::PUBLICATION);

    EXPECT_EQ(p1->key, "pub1");
}

TEST(handleManager, multiFutureAlias)
{
    auto h1 = generateExampleHandleManager();
    h1.addAlias("if", "ifonly");
    h1.addAlias("if", "ifandonlyif");
    h1.addAlias("if", "iff");

    addTestHandle(h1, "if", InterfaceType::PUBLICATION);

    const auto* p1 = h1.getInterfaceHandle("ifonly", InterfaceType::PUBLICATION);

    EXPECT_EQ(p1->key, "if");

    p1 = h1.getInterfaceHandle("ifandonlyif", InterfaceType::PUBLICATION);

    EXPECT_EQ(p1->key, "if");

    p1 = h1.getInterfaceHandle("iff", InterfaceType::PUBLICATION);

    EXPECT_EQ(p1->key, "if");
}

TEST(handleManager, multiTypeFutureAlias)
{
    auto h1 = generateExampleHandleManager();
    h1.addAlias("if", "ifonly");

    addTestHandle(h1, "if", InterfaceType::PUBLICATION);
    addTestHandle(h1, "if", InterfaceType::INPUT);
    addTestHandle(h1, "if", InterfaceType::ENDPOINT);
    addTestHandle(h1, "if", InterfaceType::FILTER);

    const auto* p1 = h1.getInterfaceHandle("ifonly", InterfaceType::PUBLICATION);

    EXPECT_EQ(p1->key, "if");

    p1 = h1.getInterfaceHandle("ifonly", InterfaceType::PUBLICATION);
    EXPECT_EQ(p1->key, "if");

    p1 = h1.getInterfaceHandle("ifonly", InterfaceType::INPUT);
    EXPECT_EQ(p1->key, "if");

    p1 = h1.getInterfaceHandle("ifonly", InterfaceType::ENDPOINT);
    EXPECT_EQ(p1->key, "if");

    p1 = h1.getInterfaceHandle("ifonly", InterfaceType::FILTER);
    EXPECT_EQ(p1->key, "if");
}

TEST(handleManager, translatorAlias)
{
    auto h1 = generateExampleHandleManager();
    h1.addAlias("if", "ifonly");

    addTestHandle(h1, "if", InterfaceType::TRANSLATOR);

    const auto* p1 = h1.getInterfaceHandle("ifonly", InterfaceType::PUBLICATION);

    EXPECT_EQ(p1->key, "if");

    p1 = h1.getInterfaceHandle("ifonly", InterfaceType::PUBLICATION);
    EXPECT_EQ(p1->key, "if");

    p1 = h1.getInterfaceHandle("ifonly", InterfaceType::INPUT);
    EXPECT_EQ(p1->key, "if");

    p1 = h1.getInterfaceHandle("ifonly", InterfaceType::ENDPOINT);
    EXPECT_EQ(p1->key, "if");

    p1 = h1.getInterfaceHandle("ifonly", InterfaceType::TRANSLATOR);
    EXPECT_EQ(p1->key, "if");
}

TEST(handleManager, duplicateAliasPub)
{
    auto h1 = generateExampleHandleManager();
    h1.addAlias("p1", "pub");
    // trying to define 2 interfaces to a single alias
    EXPECT_THROW(h1.addAlias("p2", "pub"), std::runtime_error);
}

TEST(handleManager, duplicateAliasTranslator)
{
    auto h1 = generateExampleHandleManager();
    h1.addAlias("t1", "trans");
    // trying to define 2 interfaces to a single alias
    EXPECT_THROW(h1.addAlias("t2", "trans"), std::runtime_error);
}

TEST(handleManager, duplicateAliasInput)
{
    auto h1 = generateExampleHandleManager();
    h1.addAlias("in1", "input");
    // trying to define 2 interfaces to a single alias
    EXPECT_THROW(h1.addAlias("in2", "input"), std::runtime_error);
}

TEST(handleManager, duplicateAliasEndpoint)
{
    auto h1 = generateExampleHandleManager();
    h1.addAlias("e1", "endpoint");
    // trying to define 2 interfaces to a single alias
    EXPECT_THROW(h1.addAlias("e2", "endpoint"), std::runtime_error);
}

TEST(handleManager, duplicateAliasFilter)
{
    auto h1 = generateExampleHandleManager();
    h1.addAlias("f1", "filter");
    // trying to define 2 interfaces to a single alias
    EXPECT_THROW(h1.addAlias("f2", "filter"), std::runtime_error);
}

TEST(handleManager, AliasToInterfacePub)
{
    auto h1 = generateExampleHandleManager();
    // trying to define an alias to a existing target
    EXPECT_THROW(h1.addAlias("p2", "p1"), std::runtime_error);
}

TEST(handleManager, AliasToInterfaceTranslator)
{
    auto h1 = generateExampleHandleManager();
    // trying to define an alias to a existing target
    EXPECT_THROW(h1.addAlias("t2", "t1"), std::runtime_error);
}

TEST(handleManager, AliasToInterfaceInput)
{
    auto h1 = generateExampleHandleManager();
    // trying to define an alias to a existing target
    EXPECT_THROW(h1.addAlias("in2", "in1"), std::runtime_error);
}

TEST(handleManager, AliasToInterfaceEndpoint)
{
    auto h1 = generateExampleHandleManager();
    // trying to define an alias to a existing target
    EXPECT_THROW(h1.addAlias("e2", "e1"), std::runtime_error);
}

TEST(handleManager, AliasToInterfaceFilter)
{
    auto h1 = generateExampleHandleManager();
    // trying to define an alias to a existing target
    EXPECT_THROW(h1.addAlias("f2", "f1"), std::runtime_error);
}

TEST(handleManager, duplicateAliasDelayed)
{
    auto h1 = generateExampleHandleManager();
    h1.addAlias("p1", "pub");
    h1.addAlias("p4", "pub");
    // trying to define 2 interfaces to a single alias delayed
    EXPECT_THROW(addTestHandle(h1, "p4", InterfaceType::PUBLICATION), std::runtime_error);
}

TEST(handleManager, duplicateAliasDifferentInterfaceType)
{
    auto h1 = generateExampleHandleManager();
    h1.addAlias("p1", "pub");
    h1.addAlias("p4", "pub");
    // it is allowed to have different aliases for different interface types
    EXPECT_NO_THROW(addTestHandle(h1, "p4", InterfaceType::ENDPOINT));
}

TEST(handleManager, aliasReciprocity)
{
    // aliases should be reciprocal
    auto h1 = generateExampleHandleManager();
    h1.addAlias("matthew", "matt");
    // defining an interface with the name of an alias is allowed as long as the original doesn't
    // exist, this will create an alias to original interfaceName.
    EXPECT_NO_THROW(addTestHandle(h1, "matt", InterfaceType::ENDPOINT));

    const auto* ep = h1.getInterfaceHandle("matthew", InterfaceType::ENDPOINT);
    ASSERT_NE(ep, nullptr);
    EXPECT_EQ(ep->key, "matt");
}

TEST(handleManager, aliasCascadeExisting)
{
    auto h1 = generateExampleHandleManager();
    h1.addAlias("p1", "pub");
    h1.addAlias("pub", "publisher");

    const auto* p1 = h1.getInterfaceHandle("publisher", InterfaceType::PUBLICATION);
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(p1->key, "p1");

    const auto* p2 = h1.getInterfaceHandle("pub", InterfaceType::PUBLICATION);
    ASSERT_NE(p2, nullptr);
    EXPECT_EQ(p2->key, "p1");
}

TEST(handleManager, aliasCascadeFuture)
{
    auto h1 = generateExampleHandleManager();
    h1.addAlias("pub1", "pub");
    h1.addAlias("pub", "publisher");

    addTestHandle(h1, "pub1", InterfaceType::PUBLICATION);
    const auto* p1 = h1.getInterfaceHandle("publisher", InterfaceType::PUBLICATION);
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(p1->key, "pub1");

    const auto* p2 = h1.getInterfaceHandle("pub", InterfaceType::PUBLICATION);
    ASSERT_NE(p2, nullptr);
    EXPECT_EQ(p2->key, "pub1");
}

TEST(handleManager, aliasCascadeFutureMany)
{
    auto h1 = generateExampleHandleManager();
    h1.addAlias("pub1", "pub");
    h1.addAlias("pub", "publisher");
    h1.addAlias("publisher", "publisher1");
    h1.addAlias("publisher1", "publisher2");
    h1.addAlias("publisher2", "publisher3");

    EXPECT_NO_THROW(addTestHandle(h1, "pub1", InterfaceType::PUBLICATION));
    const auto* p1 = h1.getInterfaceHandle("publisher", InterfaceType::PUBLICATION);
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(p1->key, "pub1");

    p1 = h1.getInterfaceHandle("pub", InterfaceType::PUBLICATION);
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(p1->key, "pub1");
    p1 = h1.getInterfaceHandle("publisher3", InterfaceType::PUBLICATION);
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(p1->key, "pub1");

    p1 = h1.getInterfaceHandle("publisher2", InterfaceType::PUBLICATION);
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(p1->key, "pub1");

    p1 = h1.getInterfaceHandle("publisher1", InterfaceType::PUBLICATION);
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(p1->key, "pub1");

    p1 = h1.getInterfaceHandle("publisher", InterfaceType::PUBLICATION);
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(p1->key, "pub1");
}

TEST(handleManager, aliasCascadeFutureManyOther)
{
    auto h1 = generateExampleHandleManager();
    h1.addAlias("pub1", "pub");
    h1.addAlias("pub", "publisher");
    h1.addAlias("publisher", "publisher1");
    h1.addAlias("publisher1", "publisher2");
    h1.addAlias("publisher2", "publisher3");

    EXPECT_NO_THROW(addTestHandle(h1, "publisher1", InterfaceType::PUBLICATION));
    const auto* p1 = h1.getInterfaceHandle("pub1", InterfaceType::PUBLICATION);
    ASSERT_NE(p1, nullptr);

    p1 = h1.getInterfaceHandle("pub", InterfaceType::PUBLICATION);
    ASSERT_NE(p1, nullptr);

    p1 = h1.getInterfaceHandle("publisher3", InterfaceType::PUBLICATION);
    ASSERT_NE(p1, nullptr);

    p1 = h1.getInterfaceHandle("publisher2", InterfaceType::PUBLICATION);
    ASSERT_NE(p1, nullptr);

    p1 = h1.getInterfaceHandle("publisher1", InterfaceType::PUBLICATION);
    ASSERT_NE(p1, nullptr);

    p1 = h1.getInterfaceHandle("publisher", InterfaceType::PUBLICATION);
    ASSERT_NE(p1, nullptr);
}
