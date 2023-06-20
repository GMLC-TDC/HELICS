# Logging

Logging in HELICS provides a way to understand the operation of a federate and is normally handled through an independent thread. The thread prints message to the console and or to a file as events within a federate occur. This section discusses how to use the log files to confirm the co-simulation executed properly and to debug when it doesn't.

- [Log Levels](#log-levels)
- [Setting up the Simulator for Logging](#setting-up-the-simulator-for-logging)
- [Setting up the Federate for Logging](#setting-up-the-federate-for-logging)
- [Setting up the Core/Broker for Logging](#setting-up-the-core-or-broker-for-logging)

## Log Levels

There are several levels used inside HELICS for logging. The level can be set with the enumerations when using an API to set the logging level. When configuring the log level via an external JSON config, the enumerations are slightly different:

```{eval-rst}
+-----------------------------------+-----------------------+
| API enumeration                   | JSON config keyword   |
+===================================+=======================+
| ``HELICS_LOG_LEVEL_NO_PRINT``     | ``no_print``          |
+-----------------------------------+-----------------------+
| ``HELICS_LOG_LEVEL_ERROR``        | ``error``             |
+-----------------------------------+-----------------------+
| ``HELICS_LOG_LEVEL_PROFILING``    | ``profiling``         |
+-----------------------------------+-----------------------+
| ``HELICS_LOG_LEVEL_WARNING``      | ``warning``           |
+-----------------------------------+-----------------------+
| ``HELICS_LOG_LEVEL_SUMMARY``      | ``summary``           |
+-----------------------------------+-----------------------+
| ``HELICS_LOG_LEVEL_CONNECTIONS``  | ``connections``       |
+-----------------------------------+-----------------------+
| ``HELICS_LOG_LEVEL_INTERFACES``   | ``interfaces``        |
+-----------------------------------+-----------------------+
| ``HELICS_LOG_LEVEL_TIMING``       | ``timing``            |
+-----------------------------------+-----------------------+
| ``HELICS_LOG_LEVEL_DEBUG``        | ``debug``             |
+-----------------------------------+-----------------------+
| ``HELICS_LOG_LEVEL_DATA``         | ``data``              |
+-----------------------------------+-----------------------+
| ``HELICS_LOG_LEVEL_TRACE``        | ``trace``             |
+-----------------------------------+-----------------------+
```

- `HELICS_LOG_LEVEL_NO_PRINT` Don't log anything
- `HELICS_LOG_LEVEL_ERROR` Log error and faults from within HELICS
- `HELICS_LOG_LEVEL_PROFILING` Log profiling messages
- `HELICS_LOG_LEVEL_WARNING` Log warning messages of things that might be incorrect or unusual
- `HELICS_LOG_LEVEL_SUMMARY` Log summary messages on startup and shutdown. The Broker will also generate a summary with the number of federates connected and a few other items of information
- `HELICS_LOG_LEVEL_CONNECTIONS` Log a message for each connection event (federate connection/disconnection)
- `HELICS_LOG_LEVEL_INTERFACES` Log messages when interfaces, such as endpoints, publications, and filters are created
- `HELICS_LOG_LEVEL_DEBUG` Log messages related to debugging, similar to timing
- `HELICS_LOG_LEVEL_TIMING` Log messages related to timing information such as mode transition and time advancement
- `HELICS_LOG_LEVEL_DATA` Log messages related to data passage and information being sent or received
- `HELICS_LOG_LEVEL_TRACE` Log all internal messages being sent

NOTE: the numerical values of these levels is subject to change

`timing`, `data` and `trace` log levels can generate a large number of messages and should primarily be used for debugging. `trace` will produce a very large number of messages most of which deal with internal communications and is primarily for debugging timing in HELICS.

Log lines will often look like

```text
echo1 (131072) (0)[t=4.0]::Time mismatch detected granted time >requested time 5.5 vs 5.0
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

- `HELICS_PROPERTY_INT_LOG_LEVEL` - General logging level applicable to both file and console logs

- `HELICS_PROPERTY_INT_FILE_LOG_LEVEL` Level to log to the file

- `HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL` Level to log to the console

These properties can be set using the JSON configuration for each federate:

```json
{
  "name": "Battery",
  "log_level": 1
}
```

Or with the API interface functions for each federate:

```python
h.helicsFederateInfoSetIntegerProperty(fed, h.HELICS_PROPERTY_INT_LOG_LEVEL, 1)
```

## Setting up the Core or Broker for Logging

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

Within the HELICS runner JSON:

```json
{
  "federates": [
    {
      "directory": ".",
      "exec": "helics_broker -f 2 --loglevel=7",
      "host": "localhost",
      "name": "broker"
    }
  ],
  "name": "fundamental_default"
}
```

A federate also can set a logging callback so log messages can be processed in whatever fashion is desired by a federate.

In C++ the method on a federate is:

```cpp
setLoggingCallback (const std::function<void(int, const std::string &, const std::string &)> &logFunction);
```

The logging callback function take 3 parameters about a message and in the case of `C` callbacks a pointer to user data.

- **loglevel**: an integer describing the level of the message
- **identifier**: a string that may contain information on the source of the log message and state/time information
- **message**: the actual message to log

In PyHELICS:

```python
h.helicsFederateSetLoggingCallback(fed, logger, user_data)
```

- **fed**: the helics.HelicsFederate that is created with helics.helicsCreateValueFederate, helics.helicsCreateMessageFederate or helics.helicsCreateCombinationFederate
- **logger**: a callback with signature void(int, const char _, const char _, void \*); the function arguments are loglevel, an identifier string, and a message string to log, and a pointer to user data
- **user_data**: a pointer to user data that is passed to the function when executing

## Log Buffer

As of Version 3.2,
HELICS cores, brokers, and federates have the capability to buffer log messages. This can be activated via the `--logbuffer` flag, or `--logbuffer=X` option. The default size is 10 messages for the `--logbuffer` flag. For cores and federates there is a `HELICS_PROPERTY_INT_LOG_BUFFER` property that can be set. And it can also be activated via the [`logbuffer <X>` command](../advanced_topics/commandInterface.md) remotely. `<X>` is the desired size of the buffer. `logbuffer stop` will deactivate the buffer in a remote command. The logs can be retrieved via the ["logs"](../advanced_topics/queries.md) query. The buffer is available even after disconnect for cores and brokers from the local object.

## Remote Logging

As of version 3.2 it is possible to have a HELICS object clone its logging to another object. For example a federate can request the broker log messages be sent to a federate log via a [`remotelog` command](../advanced_topics/commandInterface.md). Multiple objects can receive the same log and a single object can receive multiple remote logs. The federate/broker/core that sends the remote log command is the one who will receive the logs and can include logging level to receive debugging logs from a particular location. This can be stopped as well via the `remotelog stop` command.

## Additional Broker Features

### Time monitor

As of Version 3.2 Brokers have the capability to specify a federate as a time monitor, this does not affect the co-simulation but instructs the Broker to get time messages from a particular federate.
There are two new command line arguments for brokers `--timemonitor=fedName` and `--timemonitorperiod=<time>` The second can only be used if the first is specified as well.

When specified the broker creates a link to that federate and will generate a log message of form

```text
TIME: exec granted
TIME: granted time=2
TIME: disconnected, last time=4
```

The monitor can be stopped via the [command](../advanced_topics/commandInterface.md) interface through the `monitor` keyword.

```cpp
//changing the federate to use as a monitor or setting one up the first time
broker->sendCommand("broker","monitor newfed");
//changing the federate and period to use as a monitor or setting one up the first time
broker->sendCommand("broker","monitor newfed 4sec");
//stopping the time_monitor
broker->sendCommand("broker","monitor stop");
```

The time generated by the monitor federate is also the time displayed in some log messages generated by the broker and can be queried via the `time_monitor` query.
