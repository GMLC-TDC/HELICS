# Federate Message + Communication Configuration

```{eval-rst}
.. toctree::
    :maxdepth: 1

    fundamental_endpoints
    fundamental_combo
    fundamental_native_filter
    fundamental_filter_federate

```

In the [Base Example](./fundamental_default.md), we saw information passed between two federates using publications and subscriptions (pubs/subs). In addition to pubs/subs, where information is passed as _values_ (physical parameters of the system), federates can also pass information between **endpoints**, where this information is now a _message_.

This section on message and communication configuration will walk through how to set up two federates to pass messages using [endpoints](./fundamental_endpoints.md), and how to set up three federates which pass between them values and messages with a [combination](./fundamental_combo.md) of the two configurations. It will also demonstrate how to use native HELICS filters and how to set up a [custom filter federate](./fundamental_filter_federate.md) to act on messages in-flight between federates.
