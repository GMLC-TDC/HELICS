#!/bin/bash
# Copyright (c) 2017-2019,
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
# installing github for python

usage="$(basename "$0") [-h] [-t s] [-r s] [-l s] -- archive and upload source tarball to github
where:
    -h  show this help text
    -t  source code tag (v2.1.1)
    -r  repo (GMLC-TDC/HELICS)
    -l  release tag (can be different than checkout tag ex: v0.0.0archive)
    -k  github Personal access token (read/write permission)
    -i  github client id (github OAuth [optional])
    -s  github client secret (github OAut [optional])

    URL: https://github.com/GMLC-TDC/HELICS/releases"

while getopts ':ht:r:l:k:i:s:' option; do
    case "$option" in
    h)
        echo "$usage"
        exit
        ;;
    t)
        tag=$OPTARG
        ;;
    r)
        repo=$OPTARG
        ;;
    l)
        release=$OPTARG
        ;;
    k)
        GITHUB_TOKEN=$OPTARG
        ;;
    i)
        CLIENT_ID=$OPTARG
        ;;
    s)
        CLIENT_SECRET=$OPTARG
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

pip install pygithub

echo "> creating root archive"
ROOT_ARCHIVE_DIR="$(pwd)"
export ROOT_ARCHIVE_DIR

git checkout "$tag"
git submodule update --init
export OUTPUT_FILE="HELICS-${tag}.tar.gz"
# create root archive
git archive --verbose --prefix "repo/" --format "tar" --output "$ROOT_ARCHIVE_DIR/repo-output.tar" "master"

echo "> appending submodule archives"
# for each of git submodules append to the root archive
# shellcheck disable=SC2016
git submodule foreach --recursive 'git archive --verbose --prefix=repo/$path/ --format tar master --output $ROOT_ARCHIVE_DIR/repo-output-sub-$sha1.tar'

if (($(find repo-output-sub*.tar | wc -l) != 0)); then
    # combine all archives into one tar
    echo
    echo "> combining all tars"
    tar --file repo-output.tar --append repo-output-sub*.tar

    # remove sub tars
    echo "> removing all sub tars"
    rm -rf repo-output-sub*.tar
fi

# gzip the tar
echo "> gzipping final tar"
gzip --force --verbose repo-output.tar

echo "> moving output file to $OUTPUT_FILE"
mv repo-output.tar.gz "$OUTPUT_FILE"

cmd="python git-all-archive.py --repo $repo --release $release --version $tag"
if [ "${GITHUB_TOKEN}a" != "a" ]; then
    cmd="${cmd} --token $GITHUB_TOKEN"
fi
if [[ "${CLIENT_ID}a" != "a" ]]; then
    cmd="${cmd} --client_id $CLIENT_ID"
fi
if [[ "${CLIENT_SECRET}a" != "a" ]]; then
    cmd="$cmd --client_secret $CLIENT_SECRET"
fi
echo
echo "> running $cmd"
$cmd
