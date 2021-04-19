# Targeted Endpoints

Endpoints can be configured to send messages to other specific endpoints be two means, `default` and `targeted`.

- `default` - Convenient way of specifying the intended destination of a message from the configuration file. Allows the API call that sends the message to leave the destination field blank. DOES NOT create a specific dependency between the sending and receiving endpoint for the HELICS core library to use when figuring out which federate should be granted what time.

- `targeted` - Creates a dependency link between the sending and receiving endpoints, just like in value exchanges. This allows the HELICS core library to more efficient decisions about when to grant times to federates and can lead to more efficient co-simulation execution. Targeted endpoints can specify a list of federates that all their messages go to.

Targeted endpoint configuration example:

```json
{
  "name": "EV_Controller",
  "coreType": "zmq",
  "timeDelta": 1.0,
  "endpoints": [
    {
      "name": "EV_Controller/EV6",
      "global": true,
      "destinationTarget": "charger/ep"
    }
  ]
}
```
