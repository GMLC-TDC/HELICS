# distutils: language = c++

# Copyright (C) 2017, Battelle Memorial Institute
# All rights reserved.
# This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.


from libcpp.string cimport string
from libcpp cimport bool

cdef extern from "helics/common/timeRepresentation.hpp" namespace "helics":
    cdef cppclass timeRepresentation[Tconv]:
        timeRepresentation()

    cdef cppclass count_time[N, base]:
        pass

cdef extern from "helics/core/helics-time.h" namespace "helics":
    ctypedef timeRepresentation Time


cdef extern from "helics/application_api/Federate.h" namespace "helics":
    cdef cppclass FederateInfo:
        FederateInfo() except +
        FederateInfo(string fedname) except +
        string name                     # !< federate name
        bool obeserver                  # !< indicator that the federate is an observer and doesn't participate in time advancement
        bool rollback                   #!< indicator that the federate has rollback features
        bool timeAgnostic               #!< indicator that the federate doesn't use time
        bool iterative                  #!< indicator that the federate can have iterative loops

        # Federate_timing_t timing
        string coreType                 #!< the type of the core
        string coreName                 #!< the name of the core
        # Time period                     #!< the period of the federate
        # Time lookAhead                  #!< the lookahead value
        # Time impactWindow               #!< the impact window
        string coreInitString           #!< an initialization string for the core API object


    cdef cppclass Federate:
        Federate(FederateInfo fi) except +
        Federate(const string& file) except +

        cppclass op_states:
            pass

        void enterInitializationState() except +
        void enterExecutionState(bool ProcessComplete=True) nogil except +
        void finalize() except +
        void error(int errorcode) except +
        void error(int errorcode, const string& message) except +
        op_states currentState() const

        void setTimeDelta(double tdelta) except +
        double requestTime(double nextInternalTimeStep) nogil except +
