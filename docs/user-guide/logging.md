# Logging

Logging in HELICS is normally handled through an independent thread. The thread prints message to the console and or to a file.

## Federate Logging

Most of the time the log for a federate is the same as for its core. This is managed through a few properties in the FederateInfo structure which can also be directly specified through the property functions.

- `helics_property_int_log_level` General logging level applicable to both file and console logs

- `helics_property_int_file_log_level` Level to log to the file

- `helics_property_int_console_log_level` Level to log to the console

These properties can be set using the API interface functions

```c
helicsFederateInfoSetIntegerProperty(fi,helics_property_int_log_level, helics_log_level_data,&err);
```

```python
h.helicsFederateInfoSetIntegerProperty(fi,h.helics_property_int_log_level, h.helics_log_level_data)
```

NOTE: logging level properties set in a federateInfo will apply to a core as well if it is the first federate registered in the core. After registration log level properties must be set separately for the core and federate.

There are several levels used inside HELICS for logging

- `helics_log_level_no_print` Don't print anything
- `helics_log_level_error` Error and faults from within HELICS
- `helics_log_level_warning` Warning messages of things that might be incorrect or unusual
- `helics_log_level_summary` Summary messages on startup and shutdown. The Broker will also generate a summary with the number of federates connected and a few other items of information
- `helics_log_level_connections` Log a message for each connection event (federate connection/disconnection)
- `helics_log_level_interfaces` Log messages when interfaces, such as endpoints, publications, and filters are created
- `helics_log_level_timing` Log messages related to timing information such as mode transition and time advancement
- `helics_log_level_data` Log messages related to data passage and information being sent or received
- `helics_log_level_trace` Log all internal message being sent

NOTE: these levels currently correspond to (-1 through 7) but this may change in future major version numbers to allow more fine grained control

`timing`, `data` and `trace` log levels can generate a large number of messages and should primarily be used for debugging. `trace` will produce a very large number of messages most of which deal with internal communications and is primarily for debugging message timing in HELICS.

Lines will often look like

```text
echo1 (131072) (0)::Time mismatch detected granted time >requested time 5.5 vs 5.0
```

or

```text
commMessage||26516-enRPa-PzaBB-ZG190-lj14t:got new broker information
```

which includes a name and internal id code for the federate then a time in parenthesis and the message. if it is a warning or error there will be an indicator before the object name. Names for brokers or cores are often auto generated and look like `26516-enRPa-PzaBB-ZG190-lj14t` which is essentially a random string with a thread id in the front. In this case the `commMessage` indicates it came from one of the communication modules

## Configuration files

The log levels can be controlled through the federate configuration files as well

```json
{
  //example json configuration file for a value federate all arguments are optional
  "name": "valueFed", // the name of the federate
  "coretype": "zmq", //the type of the core "test","zmq","udp","ipc","tcp","mpi"
  "corename": "core1", //this matters most for ipc and test cores, can be empty
  "coreinit": "--autobroker", // the initialization string for the core in the form of a command line arguments
  "period": 1.0, //the period with which federate may return time
  "log_level": 1 //specify the log level
}
```

toml files are similar. It is also possible to specify at the core level

```json
{
  //example json configuration file for a value federate all arguments are optional
  "name": "valueFed", // the name of the federate
  "coretype": "zmq", //the type of the core "test","zmq","udp","ipc","tcp","mpi"
  "corename": "core1", //this matters most for ipc and test cores, can be empty
  "coreinit": "--autobroker --log_level=trace", // the initialization string for the core in the form of a command line arguments
  "period": 1.0 //the period with which federate may return time
}
```

## log level string representation

when specifying log levels through the command line or through config files it is also possible to use a string representation

- "no_print" :no log messages
- "error" : only error message
- "warning" : errors + warning messages
- "summary" : some additional summary messages (the default)
- "connections" :summary + connection messages for federates connecting and disconnecting
- "interfaces" : connections + interface creation messages
- "timing" : interfaces+ some timing messages
- "debug" : same as data
- "data" : timing + data transfer logging
- "trace" : all internal messages

```json
{
  //example json configuration file for a value federate all arguments are optional
  "name": "valueFed", // the name of the federate
  "coretype": "zmq", //the type of the core "test","zmq","udp","ipc","tcp","mpi"
  "corename": "core1", //this matters most for ipc and test cores, can be empty
  "coreinit": "--autobroker", // the initialization string for the core in the form of a command line arguments
  "period": 1.0, //the period with which federate may return time
  "log_level": "connections" //specify the log level as a string
}
```

## Log Files

It is possible to specify a log file to use on a core.
This can be specified through the coreinit string `--logfile logfile.txt`

or on a core object

```c
helicsCoreSetLogFile(core,"logfile.txt",&err);
```

A similar function is available for a broker. The Federate version will set the logFile on the connected core.

```c
helicsFederateSetLogFile(fed,"logfile.txt",&err);
```

A federate also can set a logging callback so log messages can be processed in whatever fashion is desired by a federate. In C++ the method on a federate is

```cpp
 void
setLoggingCallback (const std::function<void(int, const std::string &, const std::string &)> &logFunction);
```

```c
void
helicsFederateSetLoggingCallback (helics_federate fed,
                                  void (*logger) (int loglevel, const char *identifier, const char *message, void *userData),
                                  void *userdata,
                                  helics_error *err);
```

These functions are not available in the language API's yet

The callback take 3 parameters about a message and in the case of `C` callbacks a pointer to user data.

- loglevel an integer code describing the level of the message as described above.
- identifier a string with the name of the object generating the message (may be empty)
- message the actual message to log

## User Log Messages

A set of functions are available for individual federates to generate log messages

```cpp
void logMessage (int level, const std::string &message) const;
void logErrorMessage (const std::string &message) const;
void logWarningMessage (const std::string &message) const;
void logInfoMessage (const std::string &message) const;
void logDebugMessage (const std::string &message) const;
```

These will log a message at the appropriate level or at a user specified level.
