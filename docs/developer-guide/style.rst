Style Guide
===========

The goal of the style guide is to describe in detail naming conventions for developing HELICS.
Style conventions are encapsulated in the .clang_format files in the project

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

3) class methods should be ``camelCase``

    .. code-block:: cpp

        publication_id_t registerGlobalPublication (const std::string &name, const std::string &type, const std::string &units = "");

4) All fundamental types and enumerations should be underscore separated words in lower case

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

5) All C++ functions and types should be contained in the helics namespace with subnamespaces used as appropriate

.. code-block:: cpp

    namespace helics
    {
        ...
    } // namespace helics

6) C interface functions should begin with helicsXXXX

.. code-block:: c

    int helicsBrokerIsConnected (helics_broker broker);

7) C interface function should be of the format helics{Class}{Action} or helics{Action} if no class is appropriate

.. code-block:: c

    int helicsBrokerIsConnected (helics_broker broker);

    const char *helicsGetVersion ();
