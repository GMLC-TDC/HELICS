#! /bin/bash
# Upload a file as a release asset; fails (non-zero exit) if the upload is rejected
set -euo pipefail

pushd "$(dirname "$1")"
FILE=$(basename "$1")
CONTENT_TYPE="$(file -b --mime-type "$FILE")"
curl \
    -sSL \
    --fail-with-body \
    -XPOST \
    -H "Authorization: token ${GITHUB_TOKEN}" \
    -H "Content-Type: ${CONTENT_TYPE}" \
    --upload-file "${FILE}" \
    "${UPLOAD_URL%\{*}?name=${FILE}"
popd
