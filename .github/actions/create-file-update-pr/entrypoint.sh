#!/bin/bash

git config --global credential.helper store
echo "https://${GITHUB_TOKEN}:x-oauth-basic@github.com" >> $HOME\.git-credentials

files_changed=$(git diff --staged --name-only)
if [[ "$files_changed" != "" ]];
then
  hash=$(sha256sum ${files_changed} | sha256sum | cut -c 1-12 -)
  current_branch=${GITHUB_REF#refs/heads/}
  
  echo "Hash=$hash"
  echo "Current branch=$current_branch"
  
  git config user.name "${INPUT_GIT_NAME}"
  git config user.email "${INPUT_GIT_EMAIL}"
  
  pr_branch="${INPUT_BRANCH_PREFIX}update-${current_branch}-${hash}"
  git ls-remote --exit-code . "origin/${pr_branch}"
  if [[ "$?" != "0" ]];
  then
    git checkout -b "${pr_branch}"
    git commit -m "${INPUT_COMMIT_MSG}"
    git push -u origin "${pr_branch}"
  fi
  echo $pr_branch
fi
