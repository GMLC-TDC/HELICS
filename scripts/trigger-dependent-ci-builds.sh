#!/bin/bash

# Trigger HELICS-FMI build
body='{
"request": {
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

