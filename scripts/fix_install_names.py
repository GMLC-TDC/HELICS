#!/usr/bin/env python

import os
import subprocess
import shlex

CURRENT_DIRECTORY = os.path.realpath(os.path.dirname(__file__))

DEPENDENCIES = os.path.abspath(os.path.join(CURRENT_DIRECTORY, "../dependencies/"))
BUILD_DIRECTORY = os.path.abspath(os.path.join(CURRENT_DIRECTORY, "../build"))
APPS = os.path.abspath(os.path.join(BUILD_DIRECTORY, "src/helics/apps"))

BOOST_LIBRARIES = [
    "libboost_program_options.dylib",
    "libboost_filesystem.dylib",
    "libboost_system.dylib",
    "libboost_date_time.dylib",
    "libboost_timer.dylib",
    "libboost_chrono.dylib"
]

ZMQ_LIBRARIES = [
"libzmq.5.1.3.dylib"
]

def fix_install_name(executable, with_rpath=True):

    for library in BOOST_LIBRARIES:
        cmd = "install_name_tool -change {} @rpath/{} {}".format(
            library,
            library, # os.path.join(DEPENDENCIES, "boost/lib", library),
            executable
        )
        subprocess.call(shlex.split(cmd))

    for library in ZMQ_LIBRARIES:
        cmd = "install_name_tool -change {} @rpath/{} {}".format(
            library,
            library, # os.path.join(DEPENDENCIES, "zmq/lib", library),
            executable
        )
        subprocess.call(shlex.split(cmd))

    if with_rpath is True:

        rpaths = []
        for d in ["zmq", "boost"]:
            rpaths.append("{}".format(os.path.abspath(os.path.join(DEPENDENCIES, d, "lib"))))

        cmd = "install_name_tool -add_rpath {} {}".format(" -add_rpath ".join(rpaths), executable)

        subprocess.call(shlex.split(cmd))


def main():

    print("Fixing install names ...")

    for filename in os.listdir(APPS):
        if filename.startswith("helics_") and os.access(os.path.abspath(os.path.join(APPS, filename)), os.X_OK):

            print("Fixing for {}".format(filename))
            fix_install_name(os.path.abspath(os.path.join(APPS, filename)))


if __name__ == "__main__":

    main()
