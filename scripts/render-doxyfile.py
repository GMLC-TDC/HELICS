import os

dirname = os.path.realpath(os.path.dirname(__file__))


def get_version():

    with open(os.path.join(dirname, "../CMakeLists.txt")) as f:
        data = f.read()

    for l in data.splitlines():
        if l.startswith("project") and "HELICS VERSION" in l:
            helics_version = l.split()[-1].strip(")")

    return helics_version


def main():

    helics_version = get_version()
    current_source_dir = os.path.abspath(os.path.join(dirname, "../"))
    output_dir = os.path.abspath(os.path.join(dirname, "../build-doxygen/docs"))

    with open(os.path.join(dirname, "../config/Doxyfile.in")) as f:
        doxyfile_template = f.read()

    doxyfile = []

    for l in doxyfile_template.splitlines():
        l = l.replace("@HELICS_VERSION@", helics_version)
        l = l.replace("@CMAKE_CURRENT_SOURCE_DIR@", current_source_dir)
        l = l.replace("@DOXYGEN_OUTPUT_DIR@", output_dir)
        doxyfile.append(l)

    with open(os.path.join(dirname, "../Doxyfile"), "w") as f:
        f.write("\n".join(doxyfile))


if __name__ == "__main__":

    main()
