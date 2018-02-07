# Copyright (C) 2017, Battelle Memorial Institute
# All rights reserved.
# This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.


from libcpp cimport bool
from libcpp.string cimport string

from federate cimport FederateInfo, Federate
from identifierTypes cimport publication_id_t, subscription_id_t
from message cimport data_view, data_block

cdef extern from "helics/application_api/ValueFederate.h" namespace "helics":
    cdef cppclass ValueFederateManager:
        pass

    cdef cppclass ValueFederate(Federate):
        ValueFederate(FederateInfo fi) except +

        publication_id_t registerPublication(const string & name, const string & type) except +
        publication_id_t registerPublication[X](const string & name, const string & units = "" ) except +

        publication_id_t registerGlobalPublication(const string & name, const string & type) except +
        publication_id_t registerGlobalPublication[X](const string & name, const string & units = "" )  except +

        subscription_id_t registerRequiredSubscription(const string & name, const string & type) except +
        subscription_id_t registerRequiredSubscription[X](const string &name, const string &units = "") except +

        void setDefaultValue(subscription_id_t id, data_view block) except +
        void setDefaultValue(subscription_id_t id, const data_block & block) except +
        void setDefaultValue[X](subscription_id_t id, const X & block) except +

        void publish(publication_id_t id, data_view block) except +
        void publish(publication_id_t id, const data_block &block) except +
        void publish(publication_id_t id, const char *data) except +
        void publish(publication_id_t id, const char *data, size_t len) except +
        void publish[X](publication_id_t id, const X &value) except +

        data_view getValue(subscription_id_t id) except +
        void getValue[X](subscription_id_t id, X &obj) nogil except +

