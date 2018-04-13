#!/bin/sh

../../src/helics/core/helics_broker 2 --loglevel=4 > broker.out &
./messageFed --name fed1 --target fed2 > fed1.out &
./messageFed --name fed2 --target fed1 > fed2.out

