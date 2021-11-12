#!/bin/sh

../../src/helics/core/helics_broker 2 --loglevel=4 >broker.out &
./comboFed_shared --name fed1 --target fed2 >fed1.out &
./comboFed_shared --name fed2 --target fed1 >fed2.out
