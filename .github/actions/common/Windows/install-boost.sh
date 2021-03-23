#!/bin/bash
# Install Boost
BOOST_VERSION="1.74.0"
BOOST_ROOT="/c/boost"
BOOST_URL="https://sourceforge.net/projects/boost/files/boost/${BOOST_VERSION}/boost_${BOOST_VERSION//./_}.tar.bz2/download"
(
    cd "$(mktemp -d)" || exit
    curl --location --output "download.tar.bz2" "$BOOST_URL"
    tar xfj "download.tar.bz2"
    mkdir -p "$BOOST_ROOT"
    cp -r boost_*/* "$BOOST_ROOT"
) || exit
export BOOST_ROOT
