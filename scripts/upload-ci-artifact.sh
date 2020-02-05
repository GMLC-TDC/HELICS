#!/bin/bash

CI_DIR=${TRAVIS_BUILD_DIR}

# Upload artifact to Bintray
BT_URL="https://api.bintray.com/content/helics/develop/snapshot/${TRAVIS_BRANCH}/${TRAVIS_BRANCH}/helics-install-$(uname -s).sh?publish=1&override=1"
curl -T "$(ls "${CI_DIR}"/build/cpack-output/Helics*.sh)" -u"${BT_USER}:${BT_APIKEY}" "${BT_URL}"
