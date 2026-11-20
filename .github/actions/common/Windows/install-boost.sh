#!/bin/bash
# Install the Boost headers from the official Boost archive, verifying the download
BOOST_VERSION="1.92.0"
BOOST_SHA256="5c1d40cb8e19adbf740a4ec2da35b3e58f3f5804b1dce44deb53df72193cbc6c"
BOOST_ROOT="/c/boost"
BOOST_URL="https://archives.boost.io/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION//./_}.tar.bz2"
(
    cd "$(mktemp -d)" || exit
    curl --fail --location --output "download.tar.bz2" "$BOOST_URL" || exit
    echo "${BOOST_SHA256}  download.tar.bz2" | sha256sum -c - || exit
    tar xfj "download.tar.bz2" || exit
    mkdir -p "$BOOST_ROOT"
    cp -r boost_*/* "$BOOST_ROOT"
) || exit
export BOOST_ROOT
