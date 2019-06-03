#!/bin/bash

BUILD_MESSAGE="Test ${TRAVIS_REPO_SLUG} ${TRAVIS_COMMIT_RANGE}"

# Trigger HELICS-FMI build
body='{
"request": {
"message":"'
body+="${BUILD_MESSAGE}"
body+='",
"branch":"master"
}}'

curl -s -X POST \
    -H "Content-Type: application/json" \
    -H "Accept: application/json" \
    -H "Travis-API-Version: 3" \
    -H "Authorization: token ${HELICSBOT_TRAVIS_ORG_TOKEN}" \
    -d "$body" \
    https://api.travis-ci.org/repo/GMLC-TDC%2FHELICS-FMI/requests

# Trigger helics-ns3 module build
body='{
"definition": {
"id": 1
},
"reason": "individualCI"
}'

curl -s -X POST \
    -H "Content-Type: application/json" \
    -H "Accept: application/json" \
    -H "Authorization: Basic ${HELICSBOT_AZURE_TOKEN}" \
    -d "$body" \
    https://dev.azure.com/HELICS-test/helics-ns3/_apis/build/builds?api-version=4.1


# Trigger HELICS-Examples repository build
if [[ "${TRAVIS_EVENT_TYPE}" == "push" ]]; then
    BUILD_PARAMS='\"HELICS_COMMITISH\": \"'${TRAVIS_COMMIT}'\"'
elif [[ "${TRAVIS_EVENT_TYPE}" == "pull_request" && "${TRAVIS_PULL_REQUEST}" != "false" ]]; then
    BUILD_PARAMS='\"HELICS_COMMITISH\": \"'${TRAVIS_PULL_REQUEST_SHA}'\",'
    BUILD_PARAMS+='\"HELICS_PR_TARGET\": \"'${TRAVIS_BRANCH}'\",'
    BUILD_PARAMS+='\"HELICS_PR_NUM\": \"'${TRAVIS_PULL_REQUEST}'\",'
    BUILD_PARAMS+='\"HELICS_PR_SLUG\": \"'${TRAVIS_REPO_SLUG}'\"'
fi
body='{
"definition": {
"id": 2
},
'
body+='"parameters": "{'${BUILD_PARAMS}'}",'
body+='
"reason": "individualCI",
"sourceBranch": "refs/heads/HELICS_2_1"
}'

echo "----BUILD_PARAMS----"
echo "${BUILD_PARAMS}"
echo "----body----"
echo "${body}"

curl -s -X POST \
    -H "Content-Type: application/json" \
    -H "Accept: application/json" \
    -H "Authorization: Basic ${HELICSBOT_AZURE_TOKEN}" \
    -d "$body" \
    https://dev.azure.com/HELICS-test/HELICS-Examples/_apis/build/builds?api-version=4.1


