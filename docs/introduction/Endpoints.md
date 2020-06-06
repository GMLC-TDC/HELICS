# Endpoints

Endpoints are interfaces used to pass packetized data blocks (messages) to another federate. These messages can represent communication packets, or events.

Endpoints can have a type which is a user defined string. HELICS currently does not recognize any predefined types, though some conventions may be developed in the future.
An endpoint can send data to any other endpoint in the system. The data consists of raw binary data and optionally a send time.

In the Application API, endpoints can subscribe to publications to receive a message every time the publication publishes a value. They can also define destinations to send data to if no other destination is given.

Messages are delivered first by time order, then federate id number, then handle id, then by order of arrival.
