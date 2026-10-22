#!/usr/bin/env bash
# Check packaged DSOs from a Linux installer or shared-library release archive.
set -euo pipefail

if [[ $# -ne 1 || ! -f $1 ]]; then
    echo "usage: $0 Helics[-shared]-<version>-Linux-<arch>.tar.gz" >&2
    exit 2
fi

archive=$1
entries=$(tar -tvzf "$archive")
zmq_library=$(printf '%s\n' "$entries" | awk '$1 ~ /^-/ && $NF ~ /\/lib(64)?\/libzmq\.so(\.[0-9]+)+$/ { print $NF }')
helics_library=$(printf '%s\n' "$entries" | awk '$1 ~ /^-/ && $NF ~ /\/lib(64)?\/libhelics\.so(\.[0-9]+)+$/ { print $NF }')
if [[ -z $zmq_library || $zmq_library == *$'\n'* || -z $helics_library || $helics_library == *$'\n'* ]]; then
    echo "expected one regular libzmq.so and libhelics.so versioned file in $archive" >&2
    exit 1
fi

temporary=$(mktemp -d)
trap 'rm -rf "$temporary"' EXIT
tar -xOzf "$archive" "$zmq_library" >"$temporary/libzmq.so"
nm -D --defined-only "$temporary/libzmq.so" | awk '{ print $NF }' >"$temporary/exports"

if ! grep -qx zmq_version "$temporary/exports"; then
    echo "zmq_version is missing from $archive" >&2
    exit 1
fi

# libzmq's public C API is zmq_*; all other exports can interpose on users of
# the wheel, particularly when the release statically links libstdc++.
if grep -Ev '^zmq_[A-Za-z0-9_]+$' "$temporary/exports" >"$temporary/unexpected"; then
    echo "unexpected exports in $archive:" >&2
    head -n 20 "$temporary/unexpected" >&2
    exit 1
fi

tar -xOzf "$archive" "$helics_library" >"$temporary/libhelics.so"
nm -D --defined-only "$temporary/libhelics.so" | awk '{ print $NF }' >"$temporary/exports"
if ! grep -qx helicsGetVersion "$temporary/exports"; then
    echo "helicsGetVersion is missing from $archive" >&2
    exit 1
fi
if grep -Ev '^helics[A-Za-z0-9_]+$' "$temporary/exports" >"$temporary/unexpected"; then
    echo "unexpected libhelics exports in $archive:" >&2
    head -n 20 "$temporary/unexpected" >&2
    exit 1
fi

# Loading the installed DSO also catches any libhelics C++ runtime references
# that previously happened to resolve through libzmq's leaked definitions.
library_directory=${helics_library%/*}
tar -xzf "$archive" -C "$temporary" "$library_directory"
LD_BIND_NOW=1 \
    LD_LIBRARY_PATH="$temporary/$library_directory${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" \
    python3 - "$temporary/$library_directory/libhelics.so" <<'PY'
import ctypes
import sys

helics = ctypes.CDLL(sys.argv[1])
helics.helicsGetVersion.restype = ctypes.c_char_p
assert helics.helicsGetVersion()
PY

echo "$archive: public ZeroMQ and HELICS APIs exported; libhelics loads"
