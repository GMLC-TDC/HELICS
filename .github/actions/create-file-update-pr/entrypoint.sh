#!/bin/bash

PR_URL=$(jq --raw-output .repository.pulls_url "$GITHUB_EVENT_PATH")
PR_URL=${PR_URL%\{*}
API_VERSION=v3
API_HEADER="Accept: application/vnd.github.${API_VERSION}+json; application/vnd.github.shadow-cat-preview+json; application/vnd.github.symmetra-preview+json; application/vnd.github.sailor-v-preview+json"
AUTH_HEADER="Authorization: token ${GITHUB_TOKEN}"

files_changed=$(git diff --staged --name-only)
if [[ "$files_changed" != "" ]];
then
  hash=$(sha256sum ${files_changed} | sha256sum | cut -c 1-12 -)
  current_branch=${GITHUB_REF#refs/heads/}
  
  echo "Hash=$hash"
  echo "Current branch=$current_branch"
  
  git remote set-url origin "https://x-access-token:${GITHUB_TOKEN}@github.com/${GITHUB_REPOSITORY}"
  git config user.name "${INPUT_GIT_NAME}"
  git config user.email "${INPUT_GIT_EMAIL}"
  
  pr_branch="${INPUT_BRANCH_PREFIX}update-${current_branch}-${hash}"
  git ls-remote --exit-code . "origin/${pr_branch}"
  if [[ "$?" != "0" ]];
  then
    git checkout -b "${pr_branch}"
    git commit -m "${INPUT_COMMIT_MSG}"
    git push -u origin "${pr_branch}"
    
    pr_api_data="{\"base\":${current_branch}, \"head\":${pr_branch}, \"title\":${INPUT_PR_TITLE}, \"body\":${INPUT_PR_BODY}}"
    curl -XPOST -fsSL \
	 -H "${AUTH_HEADER}" \
	 -H "${API_HEADER}" \
	 --data "${pr_api_data}" \
	 "${PR_URL}"
  fi
  
  curl -XGET -fsSL \
       -H "${AUTH_HEADER}" \
       -H "${API_HEADER}" \
       "${PR_URL}?state=open&base=${current_branch}&head=${pr_branch}"
      
  echo $pr_branch
fi
