#!/bin/bash

BUILD_MESSAGE="Test integration with ${TRAVIS_REPO_SLUG} commits ${TRAVIS_COMMIT_RANGE}"

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
