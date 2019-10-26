#!/bin/bash

files_changed=$(git diff --staged --name-only)
if [[ "$files_changed" != "" ]];
then
  hash=$(sha256sum ${files_changed} | sha256sum | cut -c 1-12 -)
  echo "ChangeHash=$hash"
fi

echo "Ref:"
jq --raw-output .pull_request.head.ref "$GITHUB_EVENT_PATH"

printenv
