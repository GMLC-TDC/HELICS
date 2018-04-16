#!/bin/sh

../../src/helics/core/helics_broker 3 --loglevel=4 > broker.out &
./messageFed --name fed1 --target fed2 > fed1.out &
./messageFed --name fed2 --target fed3 > fed2.out &
./messageFed --name fed3 --target fed1 > fed3.out &

