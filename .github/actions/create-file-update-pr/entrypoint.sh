#!/bin/bash

files_changed=$(git diff --staged --name-only)
if [[ "$files_changed" != "" ]];
then
  sha256sum ${files_changed} > changed_hashes.txt
  cat changed_hashes.txt
  hash_chk=$(cat changed_hashes.txt | sha256sum | cut -c 1-12 -)
  hash=$(sha256sum ${files_changed} | sha256sum | cut -c 1-12 -)
  current_branch=$(git branch --show-current)
  
  echo "Hash=$hash"
  echo "Hash Check=$hash_chk"
  echo "Current branch=$current_branch"
  
  git config user.name "${INPUT_GIT_NAME}"
  git config user.email "${INPUT_GIT_EMAIL}"
  
  pr_branch="${INPUT_BRANCH_PREFIX}update-${current_branch}-${hash}"
  git ls-remote --exit-code . "origin/${pr_branch}"
  echo $?
  echo $pr_branch
fi

cat $GITHUB_EVENT_PATH

echo "Ref:"
jq --raw-output .ref "$GITHUB_EVENT_PATH"

printenv
