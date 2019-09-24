#
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

usage="$(basename "$0") [-h] [-t s] [-r s] [-l s] -- create archive source tarball with all submodules
where:
    -h  show this help text
    -t  source code tag (v2.1.1)
    -r  repo (GMLC-TDC/HELICS)
    -l  release tag (can be different than checkout tag ex: v0.0.0archive)

    URL: https://github.com/GMLC-TDC/HELICS/releases"

while getopts ':ht:r:l:' option; do
  case "$option" in
    h) echo "$usage"
       exit
       ;;
    t) tag=$OPTARG
       ;;
    r) repo=$OPTARG
       ;;
    l) release=$OPTARG
       ;;
    :) printf "missing argument for -%s\n" "$OPTARG" >&2
       echo "$usage" >&2
       exit 1
       ;;
   \?) printf "illegal option: -%s\n" "$OPTARG" >&2
       echo "$usage" >&2
       exit 1
       ;;
  esac
done
shift $((OPTIND - 1))

echo "> creating root archive"
export ROOT_ARCHIVE_DIR="$(pwd)"

git checkout $tag
git submodule update --init
OUTPUT_BASENAME="HELICS-${release}-source"
export OUTPUT_FILE="${OUTPUT_BASENAME}.tar.gz"
# create root archive
git archive --verbose --format "tar" --output "${ROOT_ARCHIVE_DIR}/${OUTPUT_BASENAME}.tar" "$(git rev-parse --abbrev-ref HEAD)"

echo "> appending submodule archives"
# for each of git submodules append to the root archive
git submodule foreach --recursive 'git archive --verbose --prefix=$path/ --format tar "$(git rev-parse --abbrev-ref HEAD)" --output $ROOT_ARCHIVE_DIR/repo-output-sub-$sha1.tar'


if (( $(ls repo-output-sub*.tar | wc -l) != 0  )); then
  # combine all archives into one tar
  echo
  echo "> combining all tars"
  for archivetar in $(ls repo-output-sub*.tar); do
    echo $archivetar
    tar --concatenate --file="${OUTPUT_BASENAME}.tar" "$archivetar"
  done

  # remove sub tars
  echo "> removing all sub tars"
  rm -rf repo-output-sub*.tar
fi

# gzip the tar
echo "> gzipping final tar"
gzip --force --verbose "${OUTPUT_BASENAME}.tar"

