/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "MessageDistributor.hpp"
#include "MessageGenerator.hpp"
namespace helics
{
namespace test
{
void MessageDistributor::run (MessageGenerator &gen, int cnt)
{
    int counter = 0;
    while ((cnt < 0) || (counter < cnt))
    {
        auto Message = gen.nextMessage ();
        if (Message.action () == CMD_IGNORE)
        {
            break;
        }
        auto fnd = messageDistribution.find (Message.dest_id);
        if (fnd != messageDistribution.end ())
        {
            fnd->second (std::move (Message));
        }
        ++counter;
    }
}

}  // namespace test
}  // namespace helics
