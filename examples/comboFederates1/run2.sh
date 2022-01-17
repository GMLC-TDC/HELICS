#!/bin/sh
# Run this example from the examples/comboFederates1/ folder

helics_broker -f2 --loglevel=warning > broker.out &
../../build/bin/comboFed --name=fed1 --target=fed2 >fed1.out &
../../build/bin/comboFed --name=fed2 --target=fed1 >fed2.out
