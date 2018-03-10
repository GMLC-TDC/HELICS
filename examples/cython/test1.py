# Copyright © 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
# All rights reserved. See LICENSE file and DISCLAIMER for more details.


from worker import worker

if __name__ == '__main__':

    import logging
    logging.basicConfig(level=logging.DEBUG)

    worker('test1', 'pub')

