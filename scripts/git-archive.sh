#!/bin/bash
# Copyright (c) 2017-2019,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
# the top-level NOTICE for additional details. All rights reserved.
# SPDX-License-Identifier: BSD-3-Clause
# Author: Denis Nadeau

set -e
set -C # noclobber

usage="$(basename "$0") [-h ] [-p s] [-d s] [-n s] -- create tar file for archive and submodules
where:
    -h  show this help text
    -p  PREFIX for filename  PREFIX-output.tar [HELICS]
    -d  directory to create tar files [.]
    -n  Name of the tag to checkout [master].

    URL: https://github.com/GMLC-TDC/HELICS/releases"

while getopts p:d:n: options; do
    case ${options} in
    p)
        PREFIX=${OPTARG}
        ;;
    d)
        DIRECTORY=${OPTARG}
        ;;
    n)
        NAME=${OPTARG}
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
cd "${DIRECTORY}"
ROOT_DIRECTORY=$(pwd)
echo "> checkout tag ${NAME}"
git checkout "${NAME}"
echo "> archiving main repository"
git archive --verbose --prefix "repo/" --format "tar" --output "./${PREFIX}-output.tar" "master"
echo "> checking out all submodules"
git submodule update --init
echo "> archiving all submodules"
git submodule foreach --recursive "git archive --verbose --prefix=repo/\$path/ --format tar master --output ${ROOT_DIRECTORY}/repo-output-sub-\$sha1.tar"
