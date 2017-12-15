Style Guide
===========

The goal of the style guide is to describe in detail conventions for developing HELICS.

Naming Conventions
------------------

1) All functions should be ``camelCase``

.. code-block:: cpp

    publication_id_t registerGlobalPublication (const std::string &name, const std::string &type, const std::string &units = "");


2) All classes should be ``PascalCase``

.. code-block:: cpp

    class ValueFederate : public virtual Federate
    {
    public:
        ValueFederate (const FederateInfo &fi);
    }

3) All general variables should be underscore separated words in lower case

.. code-block:: cpp

    /* Type definitions */
    typedef enum {
        helics_ok,
        helics_discard,
        helics_warning,
        helics_error,
    } helics_status;

    typedef void *helics_subscription;
    typedef void *helics_publication;
    typedef void *helics_endpoint;
    typedef void *helics_source_filter;
    typedef void *helics_destination_filter;
    typedef void *helics_core;
    typedef void *helics_broker;


