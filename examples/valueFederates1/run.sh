#!/bin/sh

../../src/helics/core/helics_broker 2 > broker.out &
./valueFed -name fed1 > fed1.out &
./valueFed -name fed2 > fed2.out
