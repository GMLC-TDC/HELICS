# Copyright (C) 2017, Battelle Memorial Institute
# All rights reserved.
# This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.


cdef extern from "helics/application_api/identifierTypes.hpp" namespace "helics":

    cdef cppclass publication_id_t:
        pass

    cdef cppclass subscription_id_t:
        pass


