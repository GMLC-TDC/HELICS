# Copyright (C) 2017, Battelle Memorial Institute
# All rights reserved.
# This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.


import subprocess
import shlex

import os
from distutils.core import setup, Extension

import platform

VERSION = os.getenv("PYHELICS_PACKAGE_VERSION", '${PYHELICS_PACKAGE_VERSION}')

if VERSION == '${PYHELICS_PACKAGE_VERSION}':
    print("Unable to find PYHELICS_PACKAGE_VERSION environment variable. Please check the documentation or contact the developers.")
    import sys
    sys.exit(1)


if platform.system() == 'Darwin':
    os_specific_cflags = ''
    os_specific_ldflags = ''  # '-shared'
else:
    os_specific_cflags = ''
    os_specific_ldflags = ''


HELICS_INSTALL = os.path.abspath(os.getenv("HELICS_INSTALL", '${CMAKE_INSTALL_PREFIX}'))
HELICS_INCLUDE_DIR = os.path.abspath(os.getenv("HELICS_INCLUDE", os.path.join(HELICS_INSTALL, "include")))
HELICS_LIB_DIR = os.path.abspath(os.getenv("HELICS_INCLUDE", os.path.join(HELICS_INSTALL, "lib")))

if HELICS_INSTALL is None or "${CMAKE_INSTALL_PREFIX}" in HELICS_INSTALL:

    print("Unable to find HELICS_INSTALL environment variable. Please check the documentation or contact the developers.")
    import sys
    sys.exit(1)


p = subprocess.Popen(
    shlex.split("swig -I{HELICS_INCLUDE_DIR} -I{HELICS_INCLUDE_SHARED_DIR} -python helics.i".format(
        HELICS_INCLUDE_DIR=HELICS_INCLUDE_DIR,
        HELICS_INCLUDE_SHARED_DIR=os.path.join(HELICS_INCLUDE_DIR, "shared_api_library")
    )),
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE
)
p.wait()

if p.returncode != 0:
    print("Unable to generate c files from SWIG.")
    print(p.stderr.read().decode("utf-8"))
    import sys
    sys.exit(1)

os.environ['CFLAGS'] = '-Wall -I"{}" -I"{}" -I"{}" -fPIC {os_specific_cflags}'.format(
    HELICS_INCLUDE_DIR,
    os.path.join(HELICS_INCLUDE_DIR, 'helics'),
    os.path.join(HELICS_INCLUDE_DIR, 'shared_api_library'),
    os_specific_cflags=os_specific_cflags,
)

os.environ['LDFLAGS'] = '{} -lzmq -L"{}"'.format(os_specific_ldflags, HELICS_LIB_DIR)

helics_module = Extension(
    "_helics",
    sources=[
        "helics_wrap.c",
    ],
    libraries=[
        "helics",
        "helicsSharedLib",
    ]
)

setup(
    name='helics',
    version=VERSION,
    author="Dheepak Krishnamurthy",
    ext_modules=[helics_module],
    py_modules=["helics"],
)

