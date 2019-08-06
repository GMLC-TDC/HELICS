#!/usr/bin/env python
# coding: utf-8

# Copyright © 2017-2019,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
# the top-level NOTICE for additional details. All rights reserved.
# SPDX-License-Identifier: BSD-3-Clause

import pdb
import os
import sys
import github
import argparse
import git
import shutil
import shlex
import glob
import tarfile
from urllib.parse import urlparse
from subprocess import Popen, PIPE


####
# Run a shell command
####
def runcmd(cmd):
    archiveargs = shlex.split(cmd)
    p = Popen(archiveargs, stdout=PIPE, stderr=PIPE)
    stdoutdata, stderrdata = p.communicate()
    return stdoutdata, stderrdata

###
# Send file to github.com
###
def SendFile(FILENAME, CLONE, RELEASE, TOKEN, CLIENTID, CLIENTSECRET):
    REPO = urlparse(CLONE).path
    REPO = REPO.split(".")[0]
    if (REPO[0] == '/'):
        REPO = REPO[1:]
    REPOSITORY = REPO

    if not os.path.exists(FILENAME):
        print("File \"" + FILENAME + "\" cannot be found in current directory.")
        sys.exit(1)
    else:
        print("> sending " + FILENAME + "....")

    # gh=github.Github(api_preview=True,
    #                  login_or_token="6f5e483731a40f1a6d88770d2b9530c589c0d830",
    #                  client_id= "062b872f7e0245054676",
    #                  client_secret="c802fceef3da13b3ab95c96a96b8f929d706f47c")

    # Open Github organization/user
    gh = github.Github(api_preview=True,
                    login_or_token=TOKEN,
                    client_id=CLIENTID,
                    client_secret=CLIENTSECRET)

    # find Repository
    try:
        repo = gh.get_repo(REPOSITORY)
    except github.GithubException as e:
        if e.status == 401:
            print("Bad Credention for repository " + REPOSITORY)
        if e.status == 404:
            print(REPOSITORY + " not found on github")
        sys.exit(1)
    print("> Found " + REPOSITORY)

    # Find release tag (version)
    #
    releases = repo.get_releases()
    found = False
    for i in range(releases.totalCount):
        release = releases.get_page(i)[0]
        if release.tag_name == RELEASE:
            assets = release.get_assets()
            found = True
    if not found:
        print("release " + RELEASE + " not found!")
        sys.exit(1)

    print("> Found " + release.tag_name)

    # Upload file to release tag.
    #
    print("> uploading " + FILENAME + " to " + REPOSITORY + " release " + release.tag_name)
    try:
        release.upload_asset(FILENAME, content_type="application/x-gzip")
    except github.GithubException as e:
        if e.status == 404:
            print("!!  ERROR: check authentication asset not found!")
            sys.exit(1)
        if e.data['errors'][0]['code'] == "already_exists":
            print(">     already exist... patching file...")

        assets = release.get_assets()
        for asset in assets:
            if asset.name == FILENAME:
                asset.update_asset(FILENAME)
    except BaseException as e:
        print("Error, check if file already exist in repository")
        sys.exit(1)


##
# main program
##
def main():

    # Setup arguments
    DESCRIPTION = "Store an extraneous binary file code in the github release page. "
    parser = argparse.ArgumentParser(description=DESCRIPTION)
    parser.add_argument('--clone', action='store', dest="CLONE",
                        help="clone a github repository [https://github.com/GMLC-TDC/HELICS.git]")
    parser.add_argument("--to_path", action='store', dest="TOPATH", help="clone into a directory [helics_tar_generate")
    parser.add_argument('--releasename', action='store', dest="RELEASE", help="github release/tag to upload if different than version")
    parser.add_argument('--prefix', action='store', dest="PREFIX", help="filename prefix [HELICS]")
    parser.add_argument('--tag', action='store', dest="HELICSTAG", help="github release/tag [v2.1.1]")
    parser.add_argument('--token', action='store', dest="TOKEN", help="github Personal Access Token")
    parser.add_argument('--client_id', action='store', dest="CLIENTID", help="github Client ID Oauth Apps")
    parser.add_argument('--client_secret', action='store', dest="CLIENTSECRET", help="github Client Secret Oauth Apps")


    if len(sys.argv) == 1:
        parser.print_help(sys.stderr)
        sys.exit(1)

    args = parser.parse_args()

    RELEASE = args.RELEASE
    PREFIX = args.PREFIX
    HELICSTAG = args.HELICSTAG
    TOKEN = args.TOKEN
    CLIENTID = args.CLIENTID
    CLIENTSECRET = args.CLIENTSECRET
    CLONE = args.CLONE
    TOPATH = args.TOPATH
    FILENAME = ""

    # Clone repository into ./HELICS by default
    #
    if CLONE is not None:
        from git import Repo
        if TOPATH is None:
            TOPATH = "./HELICS"
            if os.path.exists(TOPATH):
                shutil.rmtree(TOPATH)
        Repo.clone_from(CLONE, TOPATH)
        repo = Repo(TOPATH)
    else:
        repo = Repo(".")

    if PREFIX is None:
        PREFIX = "HELICS"

    # Checkout tag or use master
    #
    if HELICSTAG is None:
        HELICSTAG = repo.active_branch
    else:
        for ref in repo.references:
            if ref.name == HELICSTAG:
                repo.head.reference = ref
                HELICSTAG = ref

    # You can push the tarbal in any release tag
    # By deafult use the HELICSTAG
    if RELEASE is None:
        RELEASE = HELICSTAG.name

    if not (isinstance(HELICSTAG,git.refs.tag.TagReference) or isinstance(HELICSTAG, git.refs.head.Head)):
        print("Could not find tag " + HELICSTAG + " in repository")
        sys.exit(1)

    # Archive repository and submodules
    #
    os.chdir(TOPATH)  # change directory to clone repository
    currentdir = os.getcwd()
    print("> archiving main repository")
    cmd = "git archive --verbose --prefix \"repo/\" --format \"tar\" --output \"" + currentdir + "/" + PREFIX +\
        "-output.tar\" \"master\""
    stdoutdata, stderrdata = runcmd(cmd)
    print("> checkout tag "+ HELICSTAG.name)
    cmd = "git checkout " + HELICSTAG.name
    stdoutdata, stderrdata = runcmd(cmd)
    print("> checking out all submodules")
    cmd = "git submodule update --init"
    stdoutdata, stderrdata = runcmd(cmd)
    print("> archiving all submodules")
    cmd = 'git submodule foreach --recursive \'git archive --verbose --prefix=repo/$path/ --format tar master --output ' +\
        currentdir + '/repo-output-sub-$sha1.tar\''
    stdoutdata, stderrdata = runcmd(cmd)

    # Delete pre-existing work
    #
    if os.path.exists("/tmp/repo"):
        shutil.rmtree("/tmp/repo")

    print("> creating final file.")
    # Extract all archive tar into /tmp/repo
    filelist = glob.glob("*tar")
    for file in filelist:
        tar = tarfile.open(file)
        tar.extractall(path="/tmp")
        tar.close()

    # Create final File  "ex: HELICS-v2.1.1.tar.gz"
    NAME = PREFIX + "-" + HELICSTAG.name
    FILENAME = NAME + ".tar.gz"
    tar = tarfile.open(name=FILENAME, mode="w:gz")
    tar.add("/tmp/repo", arcname=NAME)
    tar.close()

    # If a github token has been passed try to upload on github.
    # RELEASE can be different tag than VERSION.
    if TOKEN is not None:
        SendFile(FILENAME, CLONE, RELEASE, TOKEN, CLIENTID, CLIENTSECRET)
    else:
        print(FILENAME + "has been created in repository")
        print("without a GitHub Toekn, you will need to upload it manually.")

    print("> Done.")
if __name__ == "__main__":
    main()
