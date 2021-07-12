# helics_app

The HELICS apps executable is one of the HELICS apps available with the library
Its purpose is to provide a common executable for running any of the other as

typical syntax is as follows

```cmd
helics-app.exe <app> <app arguments ...>
```

possible apps are

## Echo

The [Echo](Echo.html) app is a responsive app that will echo any message sent to its endpoints back to the original source with a specified delay

This is useful for testing communication pathways and in combination with filters can be used to create some interesting situations

## Player

The [player](Player.html) app will generate signals through specified interfaces from prescribed data
This is used for generating test signals into a federate

## Recorder

The [Recorder](Recorder.html) app captures signals and data on specified interfaces and can record then to various file formats including text files and JSON files
The files saved can then be used by the Player app at a later time

## Tracer

The [Tracer](Tracer.html) app functions much like the recorder when run as a standalone app with the exception that it displays information to a text window and doesn't capture to a file
The additional purpose is used as a library object as the basis for additional display purposes and interfaces

## Source

The [Source](Source.html) app is a signal generator like the player except that is can generate signals from defined patterns including some random signals in value and timing, and other patterns like sine, square wave, ramps
and others. Used much like the player in situations some test signals are needed.

## Broker

The [Broker](Broker.html) executes a broker like the stand alone Broker app, it does not include the broker terminal application.

## Clone

The [Clone](Clone.html) has the ability to copy another federate and record it to a file that can be used by a Player. It will duplicate all publications and subscriptions of a federate.

## MultiBroker

The Multibroker is an in progress development of a broker that can interact with multiple communication modes. Such as a single broker that can act as a bridge between MPI and ZeroMQ or other network protocols. More documentation will be available as the multibroker is developed
