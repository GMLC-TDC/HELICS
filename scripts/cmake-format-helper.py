#!/usr/bin/env python3

import os
import subprocess
import shlex

CURRENT_DIRECTORY = os.path.realpath(os.path.dirname(__file__))


def format_file(filename):
    """ Format cmake files """

    cmd = f"cmake-format -i {filename}"
    print(f"Formatting {filename}")
    p = subprocess.Popen(shlex.split(cmd))
    p.__filename = filename
    return p


def main():
    """ cmake-format helper script """

    processes = []

    for r, _, files in os.walk(os.path.join(CURRENT_DIRECTORY, "..")):
        for filename in files:
            if filename == "CMakeLists.txt":
                processes.append(format_file(os.path.abspath(os.path.join(r, filename))))

    for p in processes:
        p.wait()
        if p.returncode != 0:
            print(p.__filename)


if __name__ == "__main__":

    main()
