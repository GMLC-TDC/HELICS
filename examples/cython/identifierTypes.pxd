# Copyright (C) 2017, Battelle Memorial Institute
# All rights reserved.


cdef extern from "helics/application_api/identifierTypes.hpp" namespace "helics":

    cdef cppclass publication_id_t:
        pass

    cdef cppclass subscription_id_t:
        pass

