#!/usr/bin/env python

import os
import subprocess
import shlex

CURRENT_DIRECTORY = os.path.realpath(os.path.dirname(__file__))

DEPENDENCIES = os.path.abspath(os.path.join(CURRENT_DIRECTORY, "../dependencies/"))


def get_libraries(executable, substring):

    output = subprocess.check_output(shlex.split("otool -L {}".format(executable))).decode("utf-8")

    print(output)

    libraries = []
    for line in output.splitlines()[1:]:
        line = line.strip()
        if line.startswith("/"):
            continue
        library = line.split(" ")[0]
        if substring in library:
            libraries.append(library)

    return libraries


def fix_install_name(executable, with_rpath=True):

    BOOST_LIBRARIES = get_libraries(executable, "boost")

    for library in BOOST_LIBRARIES:
        cmd = "install_name_tool -change {} @rpath/{} {}".format(
            library, library, executable  # os.path.join(DEPENDENCIES, "boost/lib", library),
        )
        subprocess.call(shlex.split(cmd))

    ZMQ_LIBRARIES = get_libraries(executable, "zmq")

    for library in ZMQ_LIBRARIES:
        cmd = "install_name_tool -change {} @rpath/{} {}".format(
            library, library, executable  # os.path.join(DEPENDENCIES, "zmq/lib", library),
        )
        subprocess.call(shlex.split(cmd))

    if with_rpath is True:

        rpaths = []
        for d in ["zmq", "boost"]:
            rpaths.append("{}".format(os.path.abspath(os.path.join(DEPENDENCIES, d, "lib"))))

        cmd = "install_name_tool -add_rpath {} {}".format(" -add_rpath ".join(rpaths), executable)

        subprocess.call(shlex.split(cmd))


def main(executable_directory=None):

    print("Fixing install names ...")

    if executable_directory is None:
        BUILD_DIRECTORY = os.path.abspath(os.path.join(CURRENT_DIRECTORY, "../build"))
        APPS = os.path.abspath(os.path.join(BUILD_DIRECTORY, "src/helics/apps"))
    else:
        APPS = os.path.abspath(executable_directory)

    for filename in os.listdir(APPS):
        if filename.startswith("helics_") and os.access(
            os.path.abspath(os.path.join(APPS, filename)), os.X_OK
        ):

            print("Fixing for {}".format(filename))
            fix_install_name(os.path.abspath(os.path.join(APPS, filename)))


if __name__ == "__main__":

    import sys

    try:
        executable_directory = sys.argv[1]
    except:
        executable_directory = None
    main(executable_directory)
