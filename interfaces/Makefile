# Copyright (c) 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
# All rights reserved. See LICENSE file and DISCLAIMER for more details.


# Makefile for creating our standalone Cython program
PYTHON := python
PYVERSION := $(shell $(PYTHON) -c "import sys; print(sys.version[:3])")
INCDIR := $(shell $(PYTHON) -c "from distutils import sysconfig; print(sysconfig.get_python_inc())")
PLATINCDIR := $(shell $(PYTHON) -c "from distutils import sysconfig; print(sysconfig.get_python_inc(plat_specific=True))")
LIBDIR1 := $(shell $(PYTHON) -c "from distutils import sysconfig; print(sysconfig.get_config_var('LIBDIR'))")
LIBDIR2 := $(shell $(PYTHON) -c "from distutils import sysconfig; print(sysconfig.get_config_var('LIBPL'))")
PYLIB := $(shell $(PYTHON) -c "from distutils import sysconfig; print(sysconfig.get_config_var('LIBRARY')[3:-2])")
LINKCC := $(shell $(PYTHON) -c "import distutils.sysconfig; print(distutils.sysconfig.get_config_var('LINKCC'))")
LINKFORSHARED := $(shell $(PYTHON) -c "import distutils.sysconfig; print(distutils.sysconfig.get_config_var('LINKFORSHARED'))")
SYSLIBS := $(shell $(PYTHON) -c "import distutils.sysconfig; print(distutils.sysconfig.get_config_var('SYSLIBS'))")
CPPINCLUDE := $(shell python-config --include)

all: clean clean-cpp build

clean:
	rm -f -r build/
	rm -f helics.so

clean-cpp:
	rm -f helics.cpp

clean-all: clean clean-cpp

.PHONY: build
build: clean
	swig -I../src/helics/shared_api_library -python helics.i
	gcc -fpic helics_wrap.c $(CPPINCLUDE) -I../src/helics/shared_api_library -shared -lhelicsSharedLib -L../../helics_install/lib -L$(LIBDIR1) -L$(LIBDIR2) -W,-rpath -W,$(LIBDIR1) -W,-rpath -W,$(LIBDIR2) -l$(PYLIB) $(LIBS) $(SYSLIBS) -o _helics.so
