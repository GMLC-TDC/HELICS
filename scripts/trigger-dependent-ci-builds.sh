#!/bin/bash

#BUILD_MESSAGE="Test ${TRAVIS_REPO_SLUG} ${TRAVIS_COMMIT_RANGE}"

# Trigger HELICS-FMI build
#body='{
#"request": {
#"message":"'
#body+="${BUILD_MESSAGE}"
#body+='",
#"branch":"master"
#}}'

#curl -s -X POST \
#    -H "Content-Type: application/json" \
#    -H "Accept: application/json" \
#    -H "Travis-API-Version: 3" \
#    -H "Authorization: token ${HELICSBOT_TRAVIS_ORG_TOKEN}" \
#    -d "$body" \
#    https://api.travis-ci.org/repo/GMLC-TDC%2FHELICS-FMI/requests

# Takes 2 arguments, a Azure org/project slug and pipeline/build definition id
trigger_azure_build() {
    local azure_slug=$1
    local def_id=$2
    local body='{
    "definition": {
    "id": '${def_id}'
    },
    '
    body+='"parameters": "{'${BUILD_PARAMS}'}",'
    body+='
    "reason": "individualCI",
    "sourceBranch": "refs/heads/'${TRAVIS_BRANCH}'"
    }'

    curl -s -X POST \
        -H "Content-Type: application/json" \
        -H "Accept: application/json" \
        -H "Authorization: Basic ${HELICSBOT_AZURE_TOKEN}" \
        -d "$body" \
        "https://dev.azure.com/${azure_slug}/_apis/build/builds?api-version=4.1"

    echo "===Triggering Azure build==="
    echo "Slug: $azure_slug"
    echo "Definition ID: $def_id"
    echo "Request body: $body"
}

################################
# Setup Azure PR build variables
################################
BUILD_PARAMS='\"HELICS_COMMITISH\": \"'$(git rev-parse HEAD)'\"'
if [[ "${TRAVIS_EVENT_TYPE}" == "push" ]]; then
    BUILD_PARAMS='\"HELICS_COMMITISH\": \"'${TRAVIS_COMMIT}'\"'
elif [[ "${TRAVIS_EVENT_TYPE}" == "pull_request" && "${TRAVIS_PULL_REQUEST}" != "false" ]]; then
    BUILD_PARAMS='\"HELICS_COMMITISH\": \"'${TRAVIS_PULL_REQUEST_SHA}'\",'
    BUILD_PARAMS+='\"HELICS_PR_SOURCE\": \"'${TRAVIS_PULL_REQUEST_BRANCH}'\",'
    BUILD_PARAMS+='\"HELICS_PR_TARGET\": \"'${TRAVIS_BRANCH}'\",'
    BUILD_PARAMS+='\"HELICS_PR_NUM\": \"'${TRAVIS_PULL_REQUEST}'\",'
    BUILD_PARAMS+='\"HELICS_PR_SLUG\": \"'${TRAVIS_REPO_SLUG}'\"'
fi

##########################################
# Trigger HELICS-Examples repository build
##########################################
trigger_azure_build "HELICS-test/HELICS-Examples" 2

#################################
# Trigger helics-ns3 module build
#################################
# Only trigger for commits/PRs to master
if [[ "$TRAVIS_BRANCH" == "master" || "$TRAVIS_PULL_REQUEST_BRANCH" == "master" ]]; then
    trigger_azure_build "HELICS-test/helics-ns3" 1
fi
