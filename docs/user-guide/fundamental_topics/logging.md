# Logging

Logging in HELICS provides a way to understand the operation of a federate and is normally handled through an independent thread. The thread prints message to the console and or to a file as events within a federate occur. This section discusses how to use the log files to confirm the co-simulation executed properly and to debug when it doesn't.

- [Log Levels](#log-levels)
- [Setting up the Simulator for Logging](#setting-up-the-simulator-for-logging)
- [Setting up the Federate for Logging](#setting-up-the-federate-for-logging)
- [Setting up the Core/Broker for Logging](#setting-up-the-core-broker-for-logging)

## Log Levels

There are several levels used inside HELICS for logging. The level can be set with the enumerations when using an API to set the logging level. When configuring the log level via an external JSON config, the enumerations are slightly different:

```{eval-rst}
+-----------------------------------+-----------------------+
| API enumeration                   | JSON config keyword   |
+===================================+=======================+
| ``HELICS_LOG_LEVEL_NO_PRINT`      | ``no_print``          |
+-----------------------------------+-----------------------+
| ``HELICS_LOG_LEVEL_ERROR``        | ``error``             |
+-----------------------------------+-----------------------+
| ``HELICS_LOG_LEVEL_WARNING``      | ``warning``           |
+-----------------------------------+-----------------------+
| ``HELICS_LOG_LEVEL_SUMMARY``      | ``summary``           |
+------------------------------------------+----------------+
| ``HELICS_LOG_LEVEL_CONNECTIONS``  | ``connections``       |
+------------------------------------------+----------------+
| ``HELICS_LOG_LEVEL_INTERFACES``   | ``interfaces``        |
+------------------------------------------+----------------+
| ``HELICS_LOG_LEVEL_TIMING``       | ``timing``            |
+------------------------------------------+----------------+
| ``HELICS_LOG_LEVEL_DATA``         | ``data``              |
+------------------------------------------+----------------+
| ``HELICS_LOG_LEVEL_TRACE``        | ``trace``             |
+------------------------------------------+----------------+
```

- `helics_log_level_no_print` Don't log anything
- `helics_log_level_error` Log error and faults from within HELICS
- `helics_log_level_warning` Log warning messages of things that might be incorrect or unusual
- `helics_log_level_summary` Log summary messages on startup and shutdown. The Broker will also generate a summary with the number of federates connected and a few other items of information
- `helics_log_level_connections` Log a message for each connection event (federate connection/disconnection)
- `helics_log_level_interfaces` Log messages when interfaces, such as endpoints, publications, and filters are created
- `helics_log_level_timing` Log messages related to timing information such as mode transition and time advancement
- `helics_log_level_data` Log messages related to data passage and information being sent or received
- `helics_log_level_trace` Log all internal message being sent

NOTE: these levels currently correspond to (-1 through 7) but this may change in future major version numbers to allow more fine grained control.

`timing`, `data` and `trace` log levels can generate a large number of messages and should primarily be used for debugging. `trace` will produce a very large number of messages most of which deal with internal communications and is primarily for debugging timing in HELICS.

Log lines will often look like

```text
echo1 (131072) (0)::Time mismatch detected granted time >requested time 5.5 vs 5.0
```

or

```text
commMessage||26516-enRPa-PzaBB-ZG190-lj14t:got new broker information
```

which includes a name and internal id code for the federate followed by a time in parenthesis and the message. If it is a warning or error, there will be an indicator before the object name. Names for brokers or cores are often auto generated and look like `26516-enRPa-PzaBB-ZG190-lj14t` which is essentially a random string with a thread id in the front. In this case, the `commMessage` indicates it came from one of the communication modules.

## Setting up the Simulator for Logging

The [Fundamental Base Example](../examples/fundamental_examples/fundamental_default.md) incorporates simple logging for the two federate co-simulation. These federates, Battery and Charger, are user-written in Python with PyHELICS, so we have the luxury of setting up the simulator for logging.

In the Battery simulator, we need to import `logging` and set up the logger:

```python
import logging


logger = logging.getLogger(__name__)
logger.addHandler(logging.StreamHandler())
logger.setLevel(logging.DEBUG)
```

Now we can use the `logger` to print different levels of detail about the co-simulation execution to log files. These files will be generated for each federate in the co-simulation and the broker with the naming convention "name assigned to federate/broker"`.log`.

A set of functions are available for individual federates to generate log messages. These functions must be placed in the simulator. In the [Fundamental Base Example](../examples/fundamental_examples/fundamental_default.md), the `logger.info()` and `logger.debug()` methods are used. Stipulating different types of log messages allows the user to change the output of the log files in one location -- the config file for the federate. These will log a message at the `log_level` specified in the config file.

```python
logger.info("Only prints to log file if log_level = 2 or summary")
logger.debug("Only prints to log file if log_level = 6 or data")
logger.error("Only prints to log file if log_level = 0 or error")
logger.warning("Only prints to log file if log_level = 1 or warning")
```

## Setting up the Federate for Logging

Most of the time the log for a federate is the same as for its core. This is managed through a few properties in the [`HelicsFederateInfo` class](https://python.helics.org/api/capi-py#HelicsFederateInfo) which can also be directly specified through the property functions.

- `helics_property_int_log_level` - General logging level applicable to both file and console logs

- `helics_property_int_file_log_level` Level to log to the file

- `helics_property_int_console_log_level` Level to log to the console

These properties can be set using the JSON configuration for each federate:

```json
{
  "name": "Battery",
  "log_level": 1,
  ...
}
```

Or with the API interface functions for each federate:

```python
h.helicsFederateInfoSetIntegerProperty(fed, h.helics_property_int_log_level, 1)
```

## Setting up the Core/Broker for Logging

It is possible to specify a log file to use on a core.
This can be specified through the coreinit string `--logfile logfile.txt`

or on a core object

```python
h.helicsCoreSetLogFile(core, "logfile.txt")
```

A similar function is available for a broker. The Federate version will set the logFile on the connected core.

With the API:

```python
h.helicsFederateSetLogFile(fed, "logfile.txt")
```

Within the `helics_cli` runner JSON:

```json
{
  "federates": [
    {
      "directory": ".",
      "exec": "helics_broker -f 2 --loglevel=7",
      "host": "localhost",
      "name": "broker"
    },
    ...
    ],
  "name": "fundamental_default"
}
```

A federate also can set a logging callback so log messages can be processed in whatever fashion is desired by a federate.

In C++ the method on a federate is:

```cpp
setLoggingCallback (const std::function<void(int, const std::string &, const std::string &)> &logFunction);
```

In PyHELICS:

```python
h.helicsFederateSetLoggingCallback(fed, logger, user_data)
```

The callback take 3 parameters about a message and in the case of `C` callbacks a pointer to user data.

- loglevel an integer code describing the level of the message as described above.
- identifier a string with the name of the object generating the message (may be empty)
- message the actual message to log
