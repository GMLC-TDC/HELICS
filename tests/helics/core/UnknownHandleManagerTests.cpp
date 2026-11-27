/*
Copyright (c) 2017-2026,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Energy
Innovation LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helics/core/UnknownHandleManager.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace helics;

TEST(unknownHandleManager, clear_closed_handle)
{
    UnknownHandleManager manager;
    const GlobalHandle closed{GlobalFederateId{2}, InterfaceHandle{1}};
    const GlobalHandle sameFederate{GlobalFederateId{2}, InterfaceHandle{2}};
    const GlobalHandle otherFederate{GlobalFederateId{3}, InterfaceHandle{1}};

    for (auto add : {&UnknownHandleManager::addUnknownPublication,
                     &UnknownHandleManager::addUnknownInput,
                     &UnknownHandleManager::addUnknownEndpoint,
                     &UnknownHandleManager::addUnknownFilter,
                     &UnknownHandleManager::addReconnectablePublication,
                     &UnknownHandleManager::addReconnectableInput,
                     &UnknownHandleManager::addReconnectableEndpoint,
                     &UnknownHandleManager::addReconnectableFilter}) {
        (manager.*add)("target", closed, 0);
        (manager.*add)("other_target", closed, 0);
        (manager.*add)("target", sameFederate, 1);
        (manager.*add)("target", otherFederate, 2);
    }

    manager.clearHandle(closed);
    manager.clearHandle(closed);

    for (auto check : {&UnknownHandleManager::checkForPublications,
                       &UnknownHandleManager::checkForInputs,
                       &UnknownHandleManager::checkForEndpoints,
                       &UnknownHandleManager::checkForFilters,
                       &UnknownHandleManager::checkForReconnectionPublications,
                       &UnknownHandleManager::checkForReconnectionInputs,
                       &UnknownHandleManager::checkForReconnectionEndpoints,
                       &UnknownHandleManager::checkForReconnectionFilters}) {
        EXPECT_TRUE((manager.*check)("other_target").empty());
        EXPECT_THAT(
            (manager.*check)("target"),
            ::testing::UnorderedElementsAre(UnknownHandleManager::TargetInfo{sameFederate, 1},
                                            UnknownHandleManager::TargetInfo{otherFederate, 2}));
    }
}
