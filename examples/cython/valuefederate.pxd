# Copyright © 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
# All rights reserved. See LICENSE file and DISCLAIMER for more details.


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

