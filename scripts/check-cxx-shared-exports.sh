#!/usr/bin/env bash
# Check optional Linux C++ shared libraries linked with STATIC_STANDARD_LIB=static.
set -euo pipefail

if [[ $# -ne 2 || ! -f $1 || ! -f $2 ]]; then
    echo "usage: $0 libhelicscpp.so libhelicscpp-apps.so" >&2
    exit 2
fi

temporary=$(mktemp -d)
trap 'rm -rf "$temporary"' EXIT

for library in "$@"; do
    nm -D --defined-only "$library" > "$temporary/symbols"

    # Strong C++ runtime definitions from libstdc++/libgcc must remain local.
    awk '$2 ~ /^[BDRT]$/ && $3 ~ /^(__cxa_|__gxx_personality_v0$|_Unwind_|_ZSt|_ZNSt|_ZTISt|_ZTVSt|_ZTTSt|_ZN9__gnu_cxx)/ { print $3 }' \
        "$temporary/symbols" > "$temporary/runtime"
    if [[ -s $temporary/runtime ]]; then
        echo "exported C++ runtime symbols in $library:" >&2
        head -n 20 "$temporary/runtime" >&2
        exit 1
    fi

done

nm -D --defined-only "$1" | awk '{ print $NF }' | c++filt > "$temporary/cxx-api"
if ! grep -Fq 'helics::cleanupHelicsLibrary()' "$temporary/cxx-api"; then
    echo "missing public C++ application API in $1" >&2
    exit 1
fi

nm -D --defined-only "$2" | awk '{ print $NF }' | c++filt > "$temporary/apps-api"
if ! grep -Fq 'helics::apps::Recorder::Recorder(' "$temporary/apps-api"; then
    echo "missing public C++ apps API in $2" >&2
    exit 1
fi

echo "public HELICS C++ APIs exported without strong C++ runtime definitions"
