#! /bin/bash

pushd "$(dirname "$1")" || exit
FILE=$(basename "$1")
CONTENT_TYPE="$(file -b --mime-type "$FILE")"
CONTENT_LENGTH="$(stat -c%s "$FILE")"
curl \
    -sSL \
    -XPOST \
    -H "Authorization: token ${GITHUB_TOKEN}" \
    -H "Content-Length ${CONTENT_LENGTH}" \
    -H "Content-Type: ${CONTENT_TYPE}" \
    --upload-file "${FILE}" \
    "${UPLOAD_URL%\{*}?name=${FILE}"
popd || exit
