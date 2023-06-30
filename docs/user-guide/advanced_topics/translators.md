# Translators

Translators are HELICS objects that can be used to bridge the gap between values (publications, subscriptions, inputs) and messages (endpoints). These two types of signals come from the need to model different data: physical data (_e.g._ voltage, speed, energy) use value interfaces and abstract data (_e.g._ control signals, measurements) use message interfaces. Each type of interface has unique properties with advantages and disadvantages so it pays to think through which type of interface best suits a particular piece of data being exchanged.

But the world is messy and not ideal. Maybe you are working with a federation you wrote before you were a HELICS Master. Maybe you're inheriting somebody else's messy federation. Maybe the tools you have to work with only implemented value interfaces but you really need to use a message interface. Translators exist to bridge these two interface types and allow data exchanges that would not otherwise be possible.

## Translator Basics

To HELICS, a translator is a specialized federate with one input, one publication (output) and one endpoint. A value sent to the input will result in a message sent to any targets of the translator's endpoint and any messages sent to the endpoint will result in a publication to any subscribing federates. As a federate, there will always be a one simulated nanosecond delay between when the data arrives at the input or endpoint and is sent out on the endpoint or publication, respectively. That is, the work of the translator takes a non-zero amount of simulated time.

Translators come in three types that determine how the data is translated between values and messages: JSON, custom, and binary. The binary option is unlikely to be useful in virtually all cases and won't be discussed. The custom option utilizes the `helicsTranslatorSetCustomCallback()` function to allow a user to define a translation process directly via a custom function that gets called anytime the translator needs to do it's work. Using a custom translator may be required for special translation needs but is not expected to be needed for the typical use cases.

That leaves the JSON translator type, where the message coming out of or going into the endpoint being structured as a simple JSON formatted as:

```text
{
    "type": <HELICS data type>,
    "value": <value>
}
```

for example:

```json
{
  "type": "double",
  "value": 56.78943
}
```

In this example, if the translator's value input received a data from a publication that was formatted as a double of 56.78943, it would produce the above JSON on the endpoint output. Similarly, if the translator's endpoint received the above JSON, it would publish out a double value of 56.78943.

## Translator Configuration

Translators can be defined by any federate and either linked into the federation via a core or a federate, with the later creating a global translator which simplifies the addressing of the input, publication, and endpoint of the translator.

Translators can add their own sources and destinations for their interfaces but can also be targeted by configuration of other federates. That is, both the translator itself but also the federates that want to use the translator have the ability to connect to its inputs, publications and outputs. Because all federates connected to the output and endpoint will receive the translated data, when designing the data-passing topology of the federation. To keep certain data from going to certain federates, multiple translators may be required.

Just like inputs, publications, endpoints, and filter, translators can be defined via JSON configuration by any federate as follows:

```text
  "translators": [
     {
       "name": "test translator",
       "type": "JSON",
       "global": true,
       "info": "",
       "source_target": <name or key, (string, can be lists)>
       "destination_target: <name or key, (string, can be lists)>
     }
  ]
```

APIs exist for setting all of these parameters and are further defined in the [Configuration Options Reference](../../references/configuration_options_reference).

## DIY Translator

If for some reason the HELICS provided translators are not able to meet your needs, you can always implement a similar but more highly customized functionality through a custom federate. Though this is the use case intended to be supported via setting a custom callback for a translator, there may be other limitations that don't allow the use of a custom callback function. With such a federate, it would be possible to, for example, translate values on the same interface differently based on who sent them, allow message interfaces to conform to a non-JSON format, and implement data type translation. The same APIs you would use for writing any other federates, so in this regard it is just another federate. In this case, though, this custom translator federate doesn't model any system and just acts as a data translator.

## Example
