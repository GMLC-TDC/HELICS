# Command Interface

The command interface for HELICS was introduced in HELICS 3. It is an asynchronous communication mechanism to send commands or other information to other components.
Cores and Brokers will respond to a small subset of commands known by HELICS, but federates have a command interface to allow retrieval of commands for interpretation by a federate.

The general function appears like

```cpp
void sendCommand(const std::string& target, const std::string& commandStr,
                             HelicsSequencingModes mode)
```

the same function is available for federates, cores, and brokers.
Sequencing Mode determines the priority of the command and can be either

- `HELICS_SEQUENCING_MODE_FAST` : send on priority channel
- `HELICS_SEQUENCING_MODE_ORDERED` : send on normal channels ordered with other communication
- `HELICS_SEQUENCING_MODE_DEFAULT` : use HELICS determined default mode

```c
helicsFederateSendCommand(HelicsFederate fed, const char* target, const char* command, HelicsError* err)
```

All commands in C are sent with the default ordering for now. The use case for ordered commands is primarily testing for the time being so the interface has not been added to the C API as of yet.

## Targets

A target is specified, and can be one of the following. A federate named one of the key words is valid for the federation, but cannot be queried using the name.

```{eval-rst}
+------------------------------------------+---------------------------------------------------------------------------------------+
| target                                   | Description                                                                           |
+==========================================+=======================================================================================+
| ``broker``                               | The first broker encountered in the hierarchy from the caller                         |
+------------------------------------------+---------------------------------------------------------------------------------------+
| ``root``, ``federation``                 | The root broker of the federation                                                     |
+------------------------------------------+---------------------------------------------------------------------------------------+
| ``core``                                 | The core of a federate, this is not a valid target if called from a broker            |
+------------------------------------------+---------------------------------------------------------------------------------------+
| ``<object name>``                        | any named object in the federation can also be queried, brokers, cores, and federates |
+------------------------------------------+---------------------------------------------------------------------------------------+
```

## Command String

The `commandStr` is a generic string, so can be anything that can be contained in a string object. It is expected that most command strings will have a json format, though a few simple ones are just plain strings.

### HELICS supported commands

The following queries are defined directly in HELICS. Federates may specify a callback function which allows arbitrary user-defined queries. The queries defined here are available inside of HELICS.

```{eval-rst}
+---------------------------+-------------------------------------------------------------------------------------------------+
| Command String            | Description                                                                                     |
+===========================+=================================================================================================+
| ``terminate``             | [all objects] disconnect the object from the federation                                         |
+---------------------------+-------------------------------------------------------------------------------------------------+
| ``echo``                  | [all objects] send a command with a `commandStr`=`echo_reply` back to the sender                |
+---------------------------+-------------------------------------------------------------------------------------------------+
| ``log <string>``          | [all objects] generate a log message in a particular object                                     |
+---------------------------+-------------------------------------------------------------------------------------------------+
| ``logbuffer <size>``      | [all objects] set the log buffer to a particular size or `stop`                                 |
+---------------------------+-------------------------------------------------------------------------------------------------+
| ``monitor <args...>``     | [brokers] set up a federate as the time monitor <args...> = <federate names> <logperiod>        |
+---------------------------+-------------------------------------------------------------------------------------------------+
| ``set barrier <args...>`` | [brokers,federates] set a time barrier <args...> = <time> <barrier_id*>                         |
+---------------------------+-------------------------------------------------------------------------------------------------+
| ``clear barrier <id*>``   | [brokers,federates] clear a time barrier <barrier_id*>                                          |
+---------------------------+-------------------------------------------------------------------------------------------------+
| ``remotelog <level>``     | [all objects] instruct the object to send log messages to a remote location in addition to local|
|                           |  logging.  The <level> is a [log level string](../fundamental_topics/logging.md) or `stop`      |
+---------------------------+-------------------------------------------------------------------------------------------------+
| ``command_status``        | [federates] when received will send a string back to the source of the command                  |
|                           | looking like \"X unprocessed commands\" where X is the number of unprocessed commands           |
+---------------------------+-------------------------------------------------------------------------------------------------+
```

`*` argument is optional

### Future

How this will get used is somewhat up in the air yet. It is expected that future commands to the objects will help support debugging and other diagnostics but beyond that it is expected to evolve considerably.

## Usage Notes

Commands that must traverse the network travel along priority paths unless specified with the `HELICS_SEQUENCING_MODE_ORDERED` option in the C++ API.

### Application API

There are two basic calls in the application API as part of a [federate object](https://docs.helics.org/en/latest/doxygen/classhelics_1_1Federate.html).
To retrieve a command addressed to a federate there are two commands

```cpp
std::pair<std::string,std::string>     getCommand();
std::pair<std::string,std::string>     waitCommand();
```

The first will return immediately but the strings will be empty if there is no command available.
The second is a blocking call and will only return if a command is available.

Equivalent calls in the C API are

```c
const char *helicsFederateGetCommand(HelicsFederate fed, HelicsError *err);
const char *helicsFederateWaitCommand(HelicsFederate fed, HelicsError *err);
```

The only error paths are if the federate is not valid or not in a state to receive commands.
The python calls are similar to other python calls.

Only one command is returned per use of the `helicsFederateGetCommand()` or `helicsFederateWaitCommand()` API calls. It is possible multiple commands have been queued and to retrieve all of them, the API must be called multiple times. An API call when the command queue is empty will return a null string.
