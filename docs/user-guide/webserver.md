# Interacting with a Running Simulation

Starting in HELICS 2.4 there is a webserver that can be run with the broker_server  or helics_broker_server.  This requires using a boost version >=1.70.  The Webserver can be disabled by the `HELICS_DISABLE_BOOST=ON` or `HELICS_DISABLE_WEBSERVER=ON` options being set.  

## Startup
The webserver can be started with the option `--http` to start a restful interface using HTTP, or `--web` to start a server using websockets.  For example to run a broker server with zmq and the webserver active for 30 minutes.  The `--duration` is optional and the default is 30 minutes but any time can be specified.  

```sh
helics_broker_server --http --zmq --duration 30minutes
```

The web server is configured by default on the localhost address port 80.  If you want to configure this it can be done through a configuration file.  The format is json

```json
{
    "http":{
        "port":8080,
        "interface":"0.0.0.0"
    }
    "webscoket":{
        "port":8008,
        "interface":"0.0.0.0"
    }
}
```
then specified on the command line such as

```sh
helics_broker_server --http --web --zmq --config broker_server_config.json
```
the configuration will then make the rest webserver accessible on any interface on port 8080 and a websocket server on port 8008.  

## Rest API
The running webserver will start a process that can respond to http requests.

### Http actions
```eval_rst
+------------+---------------------------------------------------------------------------------------+
| HTTP VERB  | Description                                                                           |
+============+=======================================================================================+
| ``GET``    | Make a query, usually with nothing in the message body                                |
+------------+---------------------------------------------------------------------------------------+
| ``PUSH``   | most general command,  usually for creating a broker, but other actions are possible  |
+------------+---------------------------------------------------------------------------------------+
| ``SEARCH`` | make a query mostly with data in the body                                             |
+------------+---------------------------------------------------------------------------------------+
| ``PUT``    | create a broker                                                                       |
+------------+---------------------------------------------------------------------------------------+
| ``DELETE`` | remove a broker                                                                       |
+------------+---------------------------------------------------------------------------------------+
```

### Parameters
```eval_rst
+-------------+---------------------------------------------------------------------------------------+
| parameter   | Description                                                                           |
+=============+=======================================================================================+
| ``command`` | specify the command to use if not implied from the http action, primarily PUSH        |
+-------------+---------------------------------------------------------------------------------------+
| ``broker``  | The broker to target a focused request, or the name of a broker to create or delete   |
+-------------+---------------------------------------------------------------------------------------+
| ``type``    | For commands that create a broker, this is the type of the broker to create           |
+-------------+---------------------------------------------------------------------------------------+
| ``target``  | The actual object to target in a query                                                |
+-------------+---------------------------------------------------------------------------------------+
| ``query``   | The query to execute on the specified target                                          |
+-------------+---------------------------------------------------------------------------------------+
| ``args``    | The command line args to pass into a created broker                                   |
+-------------+---------------------------------------------------------------------------------------+
```

Valid commands for the ``command`` parameter in either JSON or the URI
-   `query`, `search` -  run a query
-   `create` - create a broker
-   `delete`, `remove` - remove a broker

## Websocket API
The websocket API will always respond in a JSON packet.  For search/get operations where the response is a JSON value that JSON will be returned. for other responses, they are converted to a JSON, for create/delete commands the response will be

```json
{
    "status":0
}
```
or
```json
{
    "status":401, //or some other code
    "error":"error message"
}
```

for queries that are not a json value
```json
{
    "status":0,
    "value":"<query result>"
}
```
or

```json
{
    "status":404,
    "error":"<error string>"
}
```

if the query did not result in a valid response
## Making queries

As a demo case there is a brokerServerTestCase executable built as part of the HELICS_EXAMPLES.   Running this example starts a webserver on the localhost using port 80.  

Currently only queries are supported, though eventually the plan is to support other debugging and control actions via a similar interface.  

The response to queries is a string either in plain text or json.  For example
```
localhost/brokers
```
will return
```json
{
   "brokers" :
   [
      {
         "address" : "tcp://127.0.0.1:23408",
         "isConnected" : true,
         "isOpen" : false,
         "isRoot" : true,
         "name" : "brokerA"
      },
      {
         "address" : "tcp://127.0.0.1:23410",
         "isConnected" : true,
         "isOpen" : true,
         "isRoot" : true,
         "name" : "brokerB"
      }
   ]
}
```

other queries should be directed to a specific broker such as
```
http://localhost/brokerA/brokers
```

which will produce a string vector
```
[41888-wfQ8t-GIGjS-dndI3-e7zuk;41888-e9KF2-HAfm8-Rft0w-JLV4a]
```

```
http://localhost/brokerA/federate_map
```
will produce a map of the federates in the federation
```json
{
   "brokers" : [],
   "cores" :
   [
      {
         "federates" :
         [
            {
               "id" : 131072,
               "name" : "fedA_1",
               "parent" : 1879048192
            }
         ],
         "id" : 1879048192,
         "name" : "41888-wfQ8t-GIGjS-dndI3-e7zuk",
         "parent" : 1
      },
      {
         "federates" :
         [
            {
               "id" : 131073,
               "name" : "fedA_2",
               "parent" : 1879048193
            }
         ],
         "id" : 1879048193,
         "name" : "41888-e9KF2-HAfm8-Rft0w-JLV4a",
         "parent" : 1
      }
   ],
   "id" : 1,
   "name" : "brokerA"
}
```

making an invalid query will produce
`http://localhost/brokerA/i_dont_care`  -> `#invalid`

Queries can be make in a number of different formats, the following are equivalent
-   `http://localhost/brokerA/publications`
-   `http://localhost/brokerA`
-   `http://localhost/brokerA?query=publications`
-   `http://localhost/brokerA/publications`
-   `http://localhost/brokerA/root/publications`
-   `http://localhost?broker=brokerA&query=publications&target=root`

in the example these will all produce
`[pub1;fedA_1/pub_string]`
which is a list of the publications

POST requests can also be made using a similar format.  

## Queries

Currently any query is accessible through this interface.  Queries have a target and a query.  The target is some named object in the federation and the query is a question.  The available queries are listed [HERE](queries.md).  More are expected to be added.  

## Json
for both the websockets and REST api they can accept arguments in JSON format.
For the REST API the parameters can be a combination of arguments in the URI and JSON in the body of the request

The most likely use case for this will be as a component for a more sophisticated control interface, so a more user friendly setup will be using the webserver as a back-end for control, debugging, information, and visualization of a running co-simulation.  
