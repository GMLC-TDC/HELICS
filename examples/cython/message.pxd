# Copyright © 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
# All rights reserved. See LICENSE file and DISCLAIMER for more details.


cdef extern from "helics/application_api/Message.h" namespace "helics":

    cdef cppclass data_view:
        pass

    cdef cppclass data_block:
        pass

