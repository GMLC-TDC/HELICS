# Copyright (C) 2017, Battelle Memorial Institute
# All rights reserved.


from worker import worker

if __name__ == '__main__':

    import logging
    logging.basicConfig(level=logging.DEBUG)

    worker('test1', 'pub')

