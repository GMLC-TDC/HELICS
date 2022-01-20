#!/bin/sh
# Run this example from the examples/comboFederates1/ folder

helics_broker -f3 --loglevel=warning >broker.out &
../../build/bin/comboFed --name=fed1 --target=fed2 >fed1.out &
../../build/bin/comboFed --name=fed2 --target=fed3 >fed2.out &
../../build/bin/comboFed --name=fed3 --target=fed1 >fed3.out &
