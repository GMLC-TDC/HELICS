#!/bin/sh

../../src/helics/core/helics_broker 2 --loglevel=4 > broker.out &
./hello_world_sender > hello_world_sender.out &
./hello_world_receiver > hello_world_receiver.out

