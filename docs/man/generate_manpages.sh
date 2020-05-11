#!/bin/bash
MAN_DEST_DIR=manpage_out
mkdir -p "${MAN_DEST_DIR}"
for filename in helics*.adoc; do
    a2x --destination-dir="${MAN_DEST_DIR}" --doctype=manpage --format=manpage --no-xmllint "${filename}"
done
