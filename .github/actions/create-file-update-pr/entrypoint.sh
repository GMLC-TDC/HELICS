#!/bin/bash

files_changed=$(git diff --staged --name-only)
if [[ "$files_changed" != "" ]];
then
  hash=$(sha256sum ${files_changed} | sha256sum | cut -c 1-12 -)
  echo "ChangeHash=$hash"
fi

cat $GITHUB_EVENT_PATH

echo "Ref:"
jq --raw-output .ref "$GITHUB_EVENT_PATH"

printenv
