# Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
# All rights reserved. See LICENSE file and DISCLAIMER for more details.

cimport cython

from cython.operator cimport dereference as deref
from libcpp.memory cimport make_shared, shared_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector

from valuefederate cimport ValueFederate
from federate cimport FederateInfo, Time
from identifierTypes cimport publication_id_t, subscription_id_t

import logging

logger = logging.getLogger(__name__)

states = {0: 'startup',
 1: 'initialization',
 2: 'execution',
 3: 'finalize',
 4: 'error'}

cdef class PyFederateInfo(object):
    cdef FederateInfo * ptr_fi

    def __cinit__(self, int number):
        logger.debug("Cython: running {}.__alloc__".format(self.__class__.__name__))
        self.ptr_fi = new FederateInfo()

        deref(self.ptr_fi).coreType = "zmq".encode('ascii');
        deref(self.ptr_fi).coreInitString = str(number).encode('ascii');

    # def __del__(self):
        # logger.debug("Python: running {}.__del__".format(self.__class__.__name__))

    # def __dealloc__(self):
        # logger.debug("Cython: running {}.__dealloc__".format(self.__class__.__name__))
        # if self.ptr_fi is not NULL:
            # del self.ptr_fi

    def setName(self, name):
        deref(self.ptr_fi).name = name.encode('ascii');


cdef class PyValueFederate(object):

    cdef FederateInfo * ptr_fi
    cdef shared_ptr[ValueFederate] vFed
    cdef publication_id_t _pubid
    cdef subscription_id_t _subid
    cdef vector[publication_id_t] publication_vector
    cdef vector[subscription_id_t] subscription_vector
    cdef vector[string] p_string
    cdef vector[string] s_string
    cdef string _string_type
    cdef string _empty_string

    def __cinit__(self, PyFederateInfo fi=None,
            int defaultValue=1,
            double timeDelta=1.0,
            list subscriptions=[],
            list publications=[]):


        self._string_type = "string".encode('ascii')
        self._empty_string = "".encode('ascii')

        logger.debug("Cython: running {}.__alloc__".format(self.__class__.__name__))

        logger.debug("Storing reference to FederateInfo in ptr_fi")
        self.ptr_fi = <FederateInfo *>fi.ptr_fi

        logger.debug("Creating shared reference to ValueFederate in ptr_fi")
        self.vFed = make_shared[ValueFederate](deref(self.ptr_fi))

        logger.debug("Storing a reference to all publications")
        for i, p in enumerate(publications):
            logger.debug("Storing {p}".format(p=p))
            self._pubid = deref(self.vFed).registerGlobalPublication(p.encode('ascii'), self._string_type)
            self.publication_vector.push_back(self._pubid)
            self.p_string.push_back(p.encode('ascii'))

        logger.debug("Storing a reference to all subscriptions")
        for i, s in enumerate(subscriptions):
            logger.debug("Storing {s}".format(s=s))
            self._subid = deref(self.vFed).registerRequiredSubscription(s.encode('ascii'), self._string_type)
            logger.debug("Stored {s} locally".format(s=s))
            # deref(self.vFed).setDefaultValue[int](self._subid, defaultValue)
            # logger.debug("Set default value")
            self.subscription_vector.push_back(self._subid)
            self.s_string.push_back(s.encode('ascii'))

        logger.debug("Setting timeDelta")
        deref(self.vFed).setTimeDelta(timeDelta)

    # def __del__(self):
        # print("Python: running {}.__del__".format(self.__class__.__name__))

    # def __dealloc__(self):
        # logger.debug("Cython: running {}.__dealloc__".format(self.__class__.__name__))
        # if self.vFed is not NULL:
        # del self.vFed
        # if self.ptr_fi is not NULL:
            # del self.ptr_fi

    def send(self, value, publication_id):
        exists = False
        for i, p in enumerate(self.p_string):
            if <string> p == <string> publication_id.encode('ascii'):
                exists = True
                break
        if exists is False:
            raise KeyError("Unable to find '{}' in list of publications".format(<string> publication_id.encode('ascii')))
        else:
            self._pubid = self.publication_vector[i]
            deref(self.vFed).publish(self._pubid, <string> value.encode('ascii'))

    def recv_all(self):
        for i, s in enumerate(self.s_string):
            self._subid = self.subscription_vector[i]
            value = self.cGetValue(self._subid)
            yield s, value

    def recv(self, subscription_id=None):
        if subscription_id is None:
            return self.recv_all()
        else:
            exists = False
            for i, s in enumerate(self.s_string):
                if <string> s == <string> subscription_id.encode('ascii'):
                    exists = True
                    break
            if exists is False:
                raise KeyError("Unable to find '{}' in list of subscriptions".format(<string> subscription_id.encode('ascii')))
            else:
                self._subid = self.subscription_vector[i]
                value = self.cGetValue(self._subid)
                return value

    cdef string cGetValue(self, subscription_id_t subid) nogil:
        cdef string value
        with nogil:
            deref(self.vFed).getValue[string](subid, value)
        with gil:
            if map(ord, value) == [1, 0, 0, 0, 1, 0, 0, 0]:
                return ''
            else:
                return value

    def getState(self):
        return self.get_state(<int>deref(self.vFed).currentState())

    def enterExecutionState(self):
        self.cEnterExecutionState()

    cdef void cEnterExecutionState(self) nogil:
        with nogil:
            deref(self.vFed).enterExecutionState()

    def get_state(self, i):
        return states[<int>i]

    def finalize(self):
        self.cFinalize();

    cdef void cFinalize(self):
        deref(self.vFed).finalize()

    def requestTime(self, double i):
        return self.cRequestTime(i)

    cdef double cRequestTime(self, double nextInternalTimeStep) nogil:
        cdef double i = 0
        with nogil:
            i = deref(self.vFed).requestTime(nextInternalTimeStep)
        return i

