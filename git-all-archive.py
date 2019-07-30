#!/usr/bin/env python
# coding: utf-8

# Copyright © 2017-2019,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
# the top-level NOTICE for additional details. All rights reserved.
# SPDX-License-Identifier: BSD-3-Clause
import github
import argparse
import sys
import os 

DESCRIPTION = "Store an extraneous binary file code in the github release page. "

parser = argparse.ArgumentParser(description=DESCRIPTION)
parser.add_argument('--repo', action='store', dest="REPOSITORY", help="github repository [users/repo]")
parser.add_argument('--release', action='store', dest="RELEASE", help="github release/tag [latest]")
parser.add_argument('--prefix', action='store', dest="PREFIX", help="filename prefix [HELICS]")
parser.add_argument('--version', action='store', dest="HELICSTAG", help="github release/tag [v2.1.1]")
parser.add_argument('--token', action='store', dest="TOKEN", help="github Personal Access Token")
parser.add_argument('--client_id', action='store', dest="CLIENTID", help="github Client ID Oauth Apps")
parser.add_argument('--client_secret', action='store', dest="CLIENTSECRET", help="github Client Secret Oauth Apps")
if len(sys.argv)==1:
    parser.print_help(sys.stderr)
    sys.exit(1)

args = parser.parse_args()
REPOSITORY = args.REPOSITORY
RELEASE = args.RELEASE
PREFIX = args.PREFIX
HELICSTAG = args.HELICSTAG
TOKEN = args.TOKEN
CLIENTID = args.CLIENTID
CLIENTSECRET = args.CLIENTSECRET

if PREFIX == None:
    PREFIX="HELICS"
if HELICSTAG == None:
    parser.print_help(sys.stderr)
    print("")
    print("!!! You did not specify a tag.")
    sys.exit(1)

FILENAME = PREFIX + "-" + HELICSTAG + ".tar.gz" 
if not os.path.exists(FILENAME):
    print("File \"" + FILENAME + "\" cannot be found in current directory.")
    sys.exit(1)
else:
    print("> sending " + FILENAME + "....")
# gh=github.Github(api_preview=True, 
#                  login_or_token="6f5e483731a40f1a6d88770d2b9530c589c0d830", 
#                  client_id= "062b872f7e0245054676", 
#                  client_secret="c802fceef3da13b3ab95c96a96b8f929d706f47c")
gh=github.Github(api_preview = True, 
                 login_or_token = TOKEN, 
                 client_id = CLIENTID,
                 client_secret = CLIENTSECRET)
try:
    repo=gh.get_repo(REPOSITORY)
except github.GithubException as e:
    if e.status == 401:
        print("Bad Credention for repository " + REPOSITORY)
    if e.status == 404:
        print(REPOSITORY + " not found on github")
    sys.exit(1)
print("> Found " + REPOSITORY)
releases=repo.get_releases()

found = False
for i in range(releases.totalCount):
    release=releases.get_page(i)[0]
    if release.tag_name == RELEASE:
        assets=release.get_assets()
        found = True
if not found:
    print("release " + release.tag_name + " not found!")
    sys.exit(1)

print("> Found " + release.tag_name)

print("> uploading " + FILENAME + " to " + REPOSITORY + " release " + release.tag_name)
try: 
    release.upload_asset(FILENAME, content_type="application/x-gzip")
except github.GithubException as e:
    if e.status == 404:
        print("!!  ERROR: check authentication asset not found!")
        sys.exit(1)
    if e.data['errors'][0]['code'] == "already_exists":
        print(">     already exist... patching file...")

    assets=release.get_assets()
    for asset in assets:
        if asset.name == FILENAME:
            asset.update_asset(FILENAME)

print("> Done.")

