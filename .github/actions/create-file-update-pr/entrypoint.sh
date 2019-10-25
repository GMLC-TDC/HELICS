#!/bin/bash

files_changed=$(git diff --staged --name-only)
if [[ "$files_changed" != "" ]];
then
  hash=$(sha256sum ${files_changed} | sha256sum | cut -c 1-5 -)
  echo "ChangeHash=$hash"
fi

printenv
