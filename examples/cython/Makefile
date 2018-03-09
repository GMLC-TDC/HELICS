# Copyright (C) 2017, Battelle Memorial Institute
# All rights reserved.


all: clean clean-cpp build

clean:
	rm -f -r build/
	rm -f helics.so

clean-cpp:
	rm -f helics.cpp

clean-all: clean clean-cpp

.PHONY: build
build: clean
	python setup.py build_ext --inplace

