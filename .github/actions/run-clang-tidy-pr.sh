#!/bin/bash
FILES_URL="$(jq -r '.pull_request._links.self.href' "$GITHUB_EVENT_PATH")/files"
FILES=$(curl -s -X GET -G "$FILES_URL" | jq -r '.[] | .filename')
echo "====Files Changed in PR===="
echo "$FILES"
filecount=$(echo "$FILES" | grep -c -E '\.(cpp|hpp|c|h)$' || true)
echo "Total changed: $filecount"
tidyerr=0
if ((filecount > 0 && filecount <= 20)); then
    echo "====Configure CMake===="
    mkdir build && cd build || exit
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DHELICS_BUILD_BENCHMARKS=ON -DHELICS_BUILD_EXAMPLES=ON -DHELICS_BUILD_TESTS=ON ..
    cd ..
    echo "====Run clang-tidy===="
    while read -r line; do
        if echo "$line" | grep -E '\.(cpp|hpp|c|h)$'; then
            python3 /usr/share/clang/run-clang-tidy.py "$line" -p build -quiet
            rc=$?
            echo "clang-tidy exit code: $rc"
            if [[ "$rc" != "0" ]]; then
                tidyerr=1
            fi
        fi
    done <<<"$FILES"
fi
exit $tidyerr
