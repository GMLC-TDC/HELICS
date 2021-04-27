#!/bin/bash

# Copyright (c) 2017-2021,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
# the top-level NOTICE for additional details. All rights reserved.
# SPDX-License-Identifier: BSD-3-Clause
# Author Khaja Minhajuddin
# Author Denis Nadeau
# File name: git-archive-all
# cd root-git-repo; git-archive-all

set -e
set -C # noclobber
#

usage="$(basename "$0") [-h] [-t s] [-l s] -- create archive source tarball with all submodules
where:
    -h  show this help text
    -t  source code tag (v2.1.1)
    -l  release tag (can be different than checkout tag ex: v0.0.0archive)

    URL: https://github.com/GMLC-TDC/HELICS/releases"

while getopts ':ht:l:' option; do
    case "$option" in
    h)
        echo "$usage"
        exit
        ;;
    t)
        tag=$OPTARG
        ;;
    l)
        release=$OPTARG
        ;;
    :)
        printf "missing argument for -%s\n" "$OPTARG" >&2
        echo "$usage" >&2
        exit 1
        ;;
    \?)
        printf "illegal option: -%s\n" "$OPTARG" >&2
        echo "$usage" >&2
        exit 1
        ;;
    esac
done
shift $((OPTIND - 1))

echo "> creating root archive"
ROOT_ARCHIVE_DIR="$(pwd)"
export ROOT_ARCHIVE_DIR

# Checkout a tag if provided
if [[ -n "$tag" ]]; then
    git checkout "$tag"
fi

# Get git submodule source code
git submodule update --init

# Set the output name based on if a release name was provided
OUTPUT_BASENAME="Helics-source"
if [[ -n "$release" ]]; then
    OUTPUT_BASENAME="Helics-${release}-source"
fi
export OUTPUT_FILE="${OUTPUT_BASENAME}.tar.gz"

# create root archive
git archive --verbose --format "tar" --output "${ROOT_ARCHIVE_DIR}/${OUTPUT_BASENAME}.tar" "$(git rev-parse --abbrev-ref HEAD)"

echo "> appending submodule archives"
# for each of git submodules append to the root archive
# uses single quotes because it is the command run recursively by git submodule foreach and should not expand variables
# shellcheck disable=SC2016
git submodule foreach --recursive 'git archive --verbose --prefix=$path/ --format tar "$(git rev-parse --abbrev-ref HEAD)" --output $ROOT_ARCHIVE_DIR/repo-output-sub-$sha1.tar'

if (($(find repo-output-sub*.tar | wc -l) != 0)); then
    # combine all archives into one tar
    echo
    echo "> combining all tars"
    for archivetar in repo-output-sub*.tar; do
        echo "$archivetar"
        tar --concatenate --file="${OUTPUT_BASENAME}.tar" "$archivetar"
    done

    # remove sub tars
    echo "> removing all sub tars"
    rm -rf repo-output-sub*.tar
fi

# slim down the tar a bit by cleaning up stuff in ThirdParty modules
rmdir_list=(
    'units/FuzzTargets'
    'units/ThirdParty'
    'units/docs'
    'units/test'
    'units/.ci'
    'units/.circleci'
    'utilities/tests'
    'utilities/.ci'
    'toml11/tests'
    'jsoncpp/test'
    'jsoncpp/.travis_scripts'
    'jsoncpp/devtools'
    'jsoncpp/.github'
    'jsoncpp/doc'
    'fmtlib/doc'
    'fmtlib/test'
    'containers/benchmarks'
    'containers/tests'
    'containers/.ci'
    'containers/.circleci'
    'concurrency/tests'
    'concurrency/benchmarks'
    'concurrency/docs'
    'concurrency/.ci'
    'concurrency/.circleci'
)
for i in "${rmdir_list[@]}"; do
    tar --delete -f "${OUTPUT_BASENAME}.tar" "ThirdParty/$i" || true
done

# gzip the tar
echo "> gzipping final tar"
gzip --force --verbose "${OUTPUT_BASENAME}.tar"
