#!/bin/bash

PR_URL=$(jq --raw-output .repository.pulls_url "$GITHUB_EVENT_PATH")
PR_URL=${PR_URL%\{*}
API_VERSION=v3
API_HEADER="Accept: application/vnd.github.${API_VERSION}+json; application/vnd.github.shadow-cat-preview+json; application/vnd.github.symmetra-preview+json; application/vnd.github.sailor-v-preview+json"
AUTH_HEADER="Authorization: token ${GITHUB_TOKEN}"

# Only commit and open a PR if files have changed
files_changed=$(git diff --staged --name-only)
if [[ "$files_changed" != "" ]];
then
  hash=$(sha256sum ${files_changed} | sha256sum | cut -c 1-12 -)
  current_branch=${GITHUB_REF#refs/heads/}
  
  # Set the git origin url for committing using a GITHUB_TOKEN
  git remote set-url origin "https://x-access-token:${GITHUB_TOKEN}@github.com/${GITHUB_REPOSITORY}"
  git config user.name "${INPUT_GIT_NAME}"
  git config user.email "${INPUT_GIT_EMAIL}"
  
  # Make sure a branch with the same name (hash + base branch) doesn't already exist
  pr_branch="${INPUT_BRANCH_PREFIX}update-${current_branch}-${hash}"
  git ls-remote --exit-code . "origin/${pr_branch}"
  rv=$?
  if [[ "$rv" != "0" ]];
  then
    # Commit the changed files and push the branch to GitHub
    git checkout -b "${pr_branch}"
    git commit -m "${INPUT_COMMIT_MSG}"
    git push -u origin "${pr_branch}"
  
    # Format string values for GitHub API JSON payload
    PR_TITLE="$(echo -n "${INPUT_PR_TITLE}" | jq --raw-input --slurp ".")"
    PR_BODY="$(echo -n "${INPUT_PR_BODY}" | jq --raw-input --slurp ".")"
    PR_BASE="$(echo -n "${current_branch}" | jq --raw-input --slurp ".")"
    PR_HEAD="$(echo -n "${pr_branch}" | jq --raw-input --slurp ".")"
    pr_api_data="{\"title\":${PR_TITLE}, \"body\":${PR_BODY}, \"base\":${PR_BASE}, \"head\":${PR_HEAD}, \"draft\":false}"
  
    # Open up the GitHub PR
    curl -XPOST -fsSL \
         -H "${AUTH_HEADER}" \
         -H "${API_HEADER}" \
         --user "${INPUT_GIT_NAME}" \
         --data "${pr_api_data}" \
         "${PR_URL}"
  fi
fi
