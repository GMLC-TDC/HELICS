# Copyright © 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
# All rights reserved. See LICENSE file and DISCLAIMER for more details.


cdef extern from "helics/application_api/identifierTypes.hpp" namespace "helics":

    cdef cppclass publication_id_t:
        pass

    cdef cppclass subscription_id_t:
        pass

