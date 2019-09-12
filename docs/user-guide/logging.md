Logging
=======

Logging in HELICS is normally handled through an independent thread.  The thread prints message to the console and or to a file.  

## Federate Logging
Most of the time the log for a federate is the same as for its core.  This is managed through a few properties in the FederateInfo structure which can also be directly specified through the property functions.

-  `helics_property_int_log_level`  General logging level applicable to both file and console logs 
      
-  `helics_property_int_file_log_level`  Level to log to the file
      
-  `helics_property_int_console_log_level`  Level to log to the console

These properties can be set using the API interface functions

```c
helicsFederateInfoSetIntegerProperty(fi,helics_property_int_log_level, helics_log_level_data,&err);
```

```python
    h.helicsFederateInfoSetIntegerProperty(fi,h.helics_property_int_log_level, h.helics_log_level_data)
```

There are several levels used inside HELICS for logging

-   `helics_log_level_no_print` Don't print anything but catastrophic errors
-   `helics_log_level_error` Error and faults from within HELICS
-   `helics_log_level_warning` Warning messages of things that might be incorrect or unusual
-   `helics_log_level_summary`  Summary messages on startup and shutdown
-   `helics_log_level_connections`  Log a message for each connection event
-   `helics_log_level_interfaces`  Log messages when interfaces, such as endpoints, publications, and filters are created
-   `helics_log_level_timing`  Log messages related to timing information such as mode transition and time advancement
-   `helics_log_level_data`  Log messages related to data passage and information being sent or received
-   `helics_log_level_trace`  Log all internal message being sent

NOTE:  these levels currently correspond to (-1 through 7) but this may change in future major version numbers to allow more fine grained control

`timing`, `data` and `trace` log levels can generate a large number of messages and should primarily be used for debugging.  `trace` will produce a very large number of messages most of which deal with internal communications and is primarily for debugging message timing in HELICS.

## Log Files
It is possible to specify a log file to use on a core.
This can be specified through the coreinit string `--logfile logfile.txt`

or on a core object
```c
helicsCoreSetLogFile(core,"logfile.txt",&err);
```

A similar function is available on a broker

