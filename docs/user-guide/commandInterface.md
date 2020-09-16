# Commands

The command interface for HELICS was introduced in HELICS 3. It is an asynchronous communication mechanism to send commands or other information to other components.
Cores and Brokers will respond to a small subset of commands known by HELICS, but federates have a command interface to allow retrieval of commands for interpretation by a federate.

The general function appears like

```cpp
void sendCommand(const std::string& target, const std::string& commandStr)
```

the same function is available for federates, cores, and brokers.

## Targets

A target is specified, and can be one of the following. A federate named one of the key words is valid for the federation, but cannot be queried using the name.

```eval_rst
+------------------------------------------+---------------------------------------------------------------------------------------+
| target                                   | Description                                                                           |
+==========================================+=======================================================================================+
| ``broker``                               | The first broker encountered in the hierarchy from the caller                         |
+------------------------------------------+---------------------------------------------------------------------------------------+
| ``root``, ``federation``                 | The root broker of the federation                                                     |
+------------------------------------------+---------------------------------------------------------------------------------------+
| ``core``                                 | The core of a federate, this is not a valid target if called from a broker          |
+------------------------------------------+---------------------------------------------------------------------------------------+
| ``<object name>``                        | any named object in the federation can also be queried, brokers, cores, and federates |
+------------------------------------------+---------------------------------------------------------------------------------------+
```

## Command String

The `commandStr` is a generic string, so can be anything that can be contained in a string object. It is expected that most command strings will have a json format, though a few simple ones are just plain strings.

### HELICS supported commands

The following queries are defined for federates. Federates may specify a callback function which allows arbitrary user-defined queries. The queries defined here are available inside of HELICS.

```eval_rst
+--------------------+------------------------------------------------------------+
| Command String     | Description                                                |
+====================+============================================================+
| ``terminate``      | [all objects] disconnect the object from the federation                    |
+--------------------+------------------------------------------------------------+
| ``echo``           | [all objects] send a command with a `commandStr`=`echo_reply` back to the sender |
+--------------------+------------------------------------------------------------+
| ``command_status`` | [federates] when received will send a string back to the source of the command
|                    | looking like \"X unprocessed commands\" where X is the number of unprocessed commands          |
+--------------------+------------------------------------------------------------+
```

### Future

How this will get used is somewhat up in the air yet. It is expected that future commands to the objects will help support debugging and other diagnostics but beyond that it is expected to evolve considerably.

## Usage Notes

Commands that must traverse the network travel along priority paths.

### Application API

There are two basic calls in the application API as part of a [federate object](../doxygen/classhelics_1_1Federate.html).
To retrieve a command addressed to a federate there are two commands

```cpp
std::pair<std::string,std::string>     getCommand();
std::pair<std::string,std::string>     waitCommand();
```

The first will return immediately but the strings will be empty if there is no command available.
The second is a blocking call and will only return if a command is available.

Equivalent calls in the C API are

```c
const char *helicsFederateGetCommand(helics_federate fed, helics_error *err);
const char *helicsFederateWaitCommand(helics_federate fed, helics_error *err);
```

The only error paths are if the federate is not valid or not in a state to receive commands.
The python calls are similar to other python calls.
