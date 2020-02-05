# Interacting with a Running Simulation

Starting in HELICS 2.4 there is a webserver that can be run with the broker_server  or helics_broker_server.  This requires building with the cmake option `HELICS_ENABLE_WEBSERVER` and using a boost version >=1.70.

## Startup
The webserver can be started with the option `--http`  For example to run a broker server with zmq and the webserver active for 30 minutes.  The `--duration` is optional and the default is 30 minutes but any time can be specified.  

```sh
helics_broker_server --http --zmq --duration 30minutes
```

The HTTP server is configured to only respond on the localhost address port 80.  If you want to configure this it can be done through a configuration file.  The format is json

```json
{
    "http":{
        "port":8080,
        "interface":"0.0.0.0"
    }
}
```
then specified on the command line such as 

```sh
helics_broker_server --http --zmq --config broker_server_config.json
```
the configuration will then make the webserver accessible on any interface on port 8080.  

## Making queries
The running webserver will start a process that can respond to http requests. It can accept GET or POST requests as a REST API.   

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

## Notes
This is an experimental interface and is subject to change and improvements.  Suggestions are welcome.  Also a websocket version of this is expected in the next release to allow websockets to connect.  

The most likely use case for this will be as a component for a more sophisticated control interface, so a more user friendly setup will be using the webserver as a back-end for control, debugging, information, and visualization of a running co-simulation.  
