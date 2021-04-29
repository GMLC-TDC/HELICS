/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "BenchmarkFederate.hpp"
#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/core/ActionMessage.hpp"

#include <algorithm>
#include <random>
#include <string>
#include <utility>
#include <vector>

/** class implementing a Watts-Strogatz-like communication pattern with messages*/
class WattsStrogatzFederate: public BenchmarkFederate {
  public:
    int initialMessageCount{
        10};  // number of messages the federate should send when it starts (too high results in
              // lower degrees being slower than high, possibly queue or buffer related for a
              // socket, and can make the udp benchmark hang indefinitely)

    // A typical Watts-Strogatz graph uses degree K for total connections to neighbors, K/2 on each
    // side. Since this requires more coordination to setup than is easily done from within
    // Federates, this simplified version has each node setup the connectiosn on its right side only
    // (so k=K/2 in this benchmark)
    int k{1};  // degree
    double b{0.6};  // re-wire probability for each of k rightmost neighbors

    // Classes related to the exponential and uniform distribution random number generator
    bool generateRandomSeed{true};
    unsigned int seed{0xABad5eed};  // suggestions for seed choice were not having a majority of the
                                    // bits as 0 is better
    std::mt19937 rand_gen;
    std::exponential_distribution<double> rand_exp;
    std::uniform_real_distribution<double> rand_rewire;
    std::uniform_int_distribution<unsigned int> rand_available_link;
    std::uniform_int_distribution<unsigned int> rand_transmit_link;

  private:
    helics::Endpoint* ept{nullptr};
    std::vector<std::string> links;  // links to other federates

  public:
    WattsStrogatzFederate(): BenchmarkFederate("WattsStrogatzFederate") {}

    // functions for setting parameters
    void setGenerateRandomSeed(bool genSeed) { generateRandomSeed = genSeed; }
    void setRandomSeed(unsigned int s) { seed = s; }
    void setDegree(int val) { k = val; }
    void setRewireProbability(double val) { b = val; }

    std::string getName() override { return getNameForIndex(index); }
    static std::string getNameForIndex(int i) { return "watts_" + std::to_string(i); }

    void setupArgumentParsing() override
    {
        deltaTime = helics::Time(10, time_units::ns);
        finalTime = helics::Time(5000, time_units::ns);

        app->add_flag("--gen_rand_seed", generateRandomSeed, "enable generating a random seed");
        app->add_option("--set_rand_seed", seed, "set the random seed");
        app->add_option("--degree",
                        k,
                        "set the degree (K/2), the number of edges to the right of each node");
        app->add_option("--rewire_probability", b, "set the probability of rewiring an edge");
        app->add_option("--initial_message_count",
                        initialMessageCount,
                        "the initial number of messages this federate should send");
        opt_index->required();
        opt_max_index->required();
    }

    void doParamInit(helics::FederateInfo& /*fi*/) override
    {
        if (k < 1 || k > maxIndex - 1) {
            std::cerr << "ERROR: Degree can't be less than 1 or more than the federate count - 1"
                      << std::endl;
            exit(1);
        }

        if (app->get_option("--set_rand_seed")->count() == 0) {
            std::mt19937 random_engine(0x600d5eed);
            std::uniform_int_distribution<unsigned int> rand_seed_uniform;
            for (int i = 0; i < index; i++) {
                // to silence [[nodiscard]] warnings
                static_cast<void>(rand_seed_uniform(random_engine));
            }
            setRandomSeed(rand_seed_uniform(random_engine));
        }

        // set up based on given params
        // en.cppreference.com/w/cpp/numeric/random/exponential_distribution
        if (generateRandomSeed) {
            std::random_device rd;
            rand_gen.seed(rd());
        } else {
            rand_gen.seed(seed);
        }
        rand_rewire = std::uniform_real_distribution<double>(0.0, 1.0);
        // create random number distribution for rewiring links (there are maxIndex-k-2 available
        // links)
        if (maxIndex - k > 1) {
            rand_available_link = std::uniform_int_distribution<unsigned int>(0, maxIndex - k - 2);
        }
        // create random number distribution for picking a link to transmit on
        rand_transmit_link = std::uniform_int_distribution<unsigned int>(0, k - 1);
    }

    void doFedInit() override
    {
        // Construct the Watts-Strogatz graph.
        // For benchmark setup reasons from within the Federates, it is a directed graph (typically
        // it is undirected). The doParamInit check on k < maxIndex - 1 ensures that currentEdges
        // and availableEdges will not include our own index, avoiding self-loops.
        std::vector<unsigned int> currentEdges;
        std::vector<unsigned int> availableEdges;
        currentEdges.reserve(k);
        availableEdges.reserve(maxIndex - k - 1);

        // Construct half of a ring lattice
        for (int i = 1; i <= k; i++) {
            currentEdges.push_back((index + i) % maxIndex);
        }

        // Create the set of links available as rewiring choices
        for (int i = 1; i <= maxIndex - k - 1; i++) {
            auto edge = index - i;
            if (edge < 0) {
                edge += maxIndex;
            }
            availableEdges.push_back(edge);
        }

        // Re-wire links
        if (!availableEdges.empty()) {
            // NOLINTNEXTLINE(modernize-loop-convert)
            for (unsigned int i = 0; i < currentEdges.size(); i++) {
                // Decide if the link should be rewired or not
                if (rand_rewire(rand_gen) < b) {
                    // Criteria for the new link:
                    // 1. Avoid self-loops
                    // 2. Avoid duplication with links that exist at this point in the algorithm

                    // Pick a new link from the available edges
                    auto newLink = rand_available_link(rand_gen);

                    // Swap current link with available edge
                    auto tmp = availableEdges[newLink];
                    availableEdges[newLink] = currentEdges[i];
                    currentEdges[i] = tmp;
                } else {
                    // No link rewiring for this link
                }
            }
        }

        // Confirm one last time that the number of edges matches the degree
        if (currentEdges.size() != static_cast<size_t>(k)) {
            std::cerr << "ERROR: The number of edges doesn't match the degree\n";
            exit(1);
        }

        // Setup list of connected edges based on currentEdges set
        links.reserve(k);
        for (auto edgeIndex : currentEdges) {
            links.push_back(getNameForIndex(edgeIndex) + "/ept");
        }

        // Create endpoint for our federate
        ept = &fed->registerEndpoint("ept");
    }

    void doMakeReady() override
    {
        // send initial messages
        std::string txstring(100, '1');
        for (int i = 0; i < initialMessageCount; i++) {
            helics::Time evTime = fed->getCurrentTime() + deltaTime;
            ept->send(links[rand_transmit_link(rand_gen)], txstring, evTime);
        }
    }

    void doMainLoop() override
    {
        auto nextTime = helics::timeZero;

        while (nextTime < finalTime) {
            nextTime = fed->requestTime(finalTime);
            while (ept->hasMessage()) {
                // Pick a random link to pass the message along
                auto transmit_link = links[rand_transmit_link(rand_gen)];
                auto m = fed->getMessage(*ept);
                m->dest = transmit_link;
                m->source = ept->getName();
                m->time = fed->getCurrentTime() + deltaTime;
                ept->send(std::move(m));
            }
        }
    }
};
