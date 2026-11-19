# shellcheck shell=bash
# Source this before configuring an Azure Linux CI build.

if ! command -v gdb >/dev/null; then
    apt=(apt-get)
    if (( EUID != 0 )); then
        apt=(sudo -n apt-get)
    fi
    if ! "${apt[@]}" update || ! "${apt[@]}" install -y --no-install-recommends gdb; then
        echo '##vso[task.logissue type=warning]GDB installation failed. Running filterTranslatorTests-ci directly.'
        return 0
    fi
fi

# Check that the container allows GDB to launch a child before wrapping the test.
if ! gdb_probe=$(gdb -nx -batch -return-child-result -ex 'set disable-randomization off' \
    -ex run --args /bin/true 2>&1); then
    printf '%s\n' "$gdb_probe"
    echo '##vso[task.logissue type=warning]GDB could not launch a child. Running filterTranslatorTests-ci directly.'
    return 0
fi

HELICS_FILTER_TEST_GDB=$(command -v gdb)
export HELICS_FILTER_TEST_GDB
export CXXFLAGS="${CXXFLAGS:+$CXXFLAGS }-g1"
