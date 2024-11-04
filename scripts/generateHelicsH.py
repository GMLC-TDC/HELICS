#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Created on Mon Dec  7 14:25:17 2020

@author: Philip Top
"""

import re
import sys
import os

outpath = "./"
inpath = "../"

if len(sys.argv) > 1:
    outpath = sys.argv[1]
    if not os.path.exists(outpath):
        os.makedirs(outpath)

if len(sys.argv) > 2:
    inpath = sys.argv[2]

outfile = open(os.path.join(outpath, "helics.h"), "w")

outfile.write("/*\n")
outfile.write("Copyright (c) 2017-2024,\n")
outfile.write(
    "Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for\n"
)
outfile.write("additional details. All rights reserved.\n")
outfile.write("SPDX-License-Identifier: BSD-3-Clause\n")
outfile.write("*/\n\n")
outfile.write("#ifndef HELICS_C_API_H_\n")
outfile.write("#define HELICS_C_API_H_\n\n")
outfile.write("#pragma once\n\n")

outfile.write("#include <stdint.h>\n")
outfile.write("#include <stdlib.h>\n\n")


outfile.write("#ifndef HELICS_EXPORT\n")
outfile.write("#    if defined _WIN32 || defined __CYGWIN__\n")
outfile.write("#        ifdef __GNUC__\n")
outfile.write("#            define HELICS_EXPORT __attribute__((dllimport))\n")
outfile.write("#        else\n")
outfile.write("#            define HELICS_EXPORT __declspec(dllimport)\n")
outfile.write("#        endif\n")
outfile.write("#    else\n")
outfile.write("#        define HELICS_EXPORT\n")
outfile.write("#    endif\n")
outfile.write("#endif\n\n")

outfile.write("#ifndef HELICS_DEPRECATED\n")
outfile.write("#    if defined _WIN32 || defined __CYGWIN__\n")
outfile.write("#        ifdef __GNUC__\n")
outfile.write("#            define HELICS_DEPRECATED __attribute__((deprecated))\n")
outfile.write("#        else\n")
outfile.write("#            define HELICS_DEPRECATED __declspec(deprecated)\n")
outfile.write("#        endif\n")
outfile.write("#    else\n")
outfile.write("#        define HELICS_DEPRECATED __attribute__((deprecated))\n")
outfile.write("#    endif\n")
outfile.write("#endif\n\n")


outfile.write("#ifdef __cplusplus\n")
outfile.write('extern "C" {\n')
outfile.write("#endif\n")

files = [
    "src/helics/helics_enums.h",
    "src/helics/shared_api_library/api-data.h",
    "src/helics/shared_api_library/helicsData.h",
    "src/helics/shared_api_library/helicsCore.h",
    "src/helics/shared_api_library/helicsApps.h",
    "src/helics/shared_api_library/ValueFederate.h",
    "src/helics/shared_api_library/MessageFederate.h",
    "src/helics/shared_api_library/MessageFilters.h",
    "src/helics/shared_api_library/Translators.h",
    "src/helics/shared_api_library/helicsCallbacks.h",
]

for file in files:
    file_in = open(os.path.join(inpath, file), "r")
    contents = file_in.read()
    contents = re.sub("#ifdef __cplusplus[^#]*#endif", "", contents)
    contents = re.sub("/*Copyright[^\*]*/", "", contents)
    contents = re.sub("#pragma once", "", contents)
    contents = re.sub("#endif[^\n]*", "", contents)
    contents = re.sub("#include[^\n]*\n", "", contents)
    contents = re.sub("^[^#]*#ifndef[^#]*#define[^\n]*\n[\s]*", "", contents)
    contents = re.sub("\n\n\n", "\n\n", contents)
    contents = re.sub("\n\n\n", "\n\n", contents)
    contents = re.sub("\n\n\n", "\n\n", contents)
    contents = re.sub("=\n       ", "=", contents)
    contents = re.sub("^\n", "", contents)
    contents = re.sub(" \n", "\n", contents)
    contents = re.sub("\n\s+\n", "\n\n", contents)
    file_in.close()
    outfile.write(contents)


outfile.write("#ifdef __cplusplus\n")
outfile.write('} /* end of extern "C" { */\n')
outfile.write("#endif\n")

outfile.write("#endif\n")
outfile.close()


outfile2 = open(os.path.join(outpath, "helics_api.h"), "w")

outfile2.write("/*\n")
outfile2.write("Copyright (c) 2017-2024,\n")
outfile2.write(
    "Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for\n"
)
outfile2.write("additional details. All rights reserved.\n")
outfile2.write("SPDX-License-Identifier: BSD-3-Clause\n")
outfile2.write("*/\n\n")

outfile2.write("#include <stdint.h>\n")
outfile2.write("#include <stdlib.h>\n\n")


for file in files:
    file_in = open(os.path.join(inpath, file), "r")
    contents = file_in.read()
    contents = re.sub("#ifdef __cplusplus[^#]*#endif", "", contents)
    contents = re.sub("/\\*[^*]*\\*+(?:[^/*][^*]*\\*+)*/", "", contents)
    contents = re.sub("#pragma once", "", contents)
    contents = re.sub("HELICS_EXPORT ", "", contents)
    contents = re.sub("#endif[^\n]*", "", contents)
    contents = re.sub("#include[^\n]*\n", "", contents)
    contents = re.sub("//[^\n]*\n", "", contents)
    contents = re.sub("^[^#]*#ifndef[^#]*#define[^\n]*\n[\s]*", "", contents)
    contents = re.sub("\n\n\n", "\n\n", contents)
    contents = re.sub("\n\n\n", "\n", contents)
    contents = re.sub("\);\n\n", ");\n", contents)
    contents = re.sub(",\n\s*\n", ",\n", contents)
    contents = re.sub("{\n\s*\n", "{\n", contents)
    contents = re.sub("\n\s*\n}", "\n}", contents)
    contents = re.sub("=\n       ", "=", contents)
    contents = re.sub("\n\n\n", "\n\n", contents)
    contents = re.sub("^\n", "", contents)
    contents = re.sub(" \n", "\n", contents)
    contents = re.sub("\n\s+\n", "\n\n", contents)
    file_in.close()
    outfile2.write(contents)


outfile2.close()
