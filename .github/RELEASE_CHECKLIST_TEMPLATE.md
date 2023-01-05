- [ ] Check that all expected [release assets](https://github.com/GMLC-TDC/HELICS/releases/${HELICS_VERSION}) were compiled for all platforms, the `*-source.tar.gz` file, and the `*-SHA-256.txt` file; if any are missing see `Steps to manually add a missing asset to a release` below
- [ ] Make sure <https://github.com/GMLC-TDC/helics-packaging/blob/main/HELICS_VERSION> has been updated with the new version number; if it hasn't, manually update it (and double check that all release assets were successfully compiled)

- [ ] Update the helics-apps PyPI package by [creating a new release](https://github.com/GMLC-TDC/helics-packaging/releases/new?tag=${HELICS_VERSION}&title=HELICS%20${HELICS_VERSION}%20release&body=HELICS%20${HELICS_VERSION}%20release%20changes:%20https://github.com/GMLC-TDC/HELICS/releases/tag/${HELICS_VERSION}) with the new version tag, title, and description similar to past releases.
  - [ ] Verify that the [Build Python Packages workflow](https://github.com/GMLC-TDC/helics-packaging/actions/workflows/pythonpackage.yml) for the release (not tag) runs to completion
  - [ ] Verify the files uploaded to PyPI at <https://pypi.org/project/helics-apps/#files> include wheel files for win32, win_amd64, macosx, and manylinux
- [ ] Run the [Update HELICS Packages workflow](https://github.com/GMLC-TDC/helics-packaging/actions/workflows/update-helics.yml)
  - [ ] Verify that a PR was opened at <https://github.com/spack/spack/pulls> that looks okay and leave a review approving it. If the workflow failed or the package PR has CI errors, debug and fix
  - [ ] Verify that a PR was opened at <https://github.com/msys2/MINGW-packages/pulls> that looks okay and leave a review approving it. If the workflow failed or the package PR has CI build errors, debug and fix
  - [ ] Verify that a PR was opened at <https://github.com/JuliaPackaging/Yggdrasil/pulls> that looks okay and leave a review approving it. If the workflow failed or the package PR has CI build errors, debug and fix
  - [ ] Verify [pyhelics](https://github.com/GMLC-TDC/pyhelics) has been updated to include new/removed features, then [create a new release](https://github.com/GMLC-TDC/pyhelics/releases/new?tag=${HELICS_VERSION}&title=${HELICS_VERSION}&body=**Full%20Changelog**:%20https://github.com/GMLC-TDC/pyhelics/compare/${HELICS_PREV_VERSION}...${HELICS_VERSION}) with a matching version tag, and link to a changelog showing the difference between the two versions
  - [ ] Regenerate the [matHELICS](https://github.com/GMLC-TDC/matHELICS) bindings using the C header file from HELICS, then [create a new release](<https://github.com/GMLC-TDC/matHELICS/releases/new?tag=${HELICS_VERSION}&title=${HELICS_VERSION}&body=Release%20for%20[HELICS%20${HELICS_VERSION}](https://github.com/GMLC-TDC/HELICS/releases/tag/${HELICS_VERSION})>) with a version tag and link to the HELICS release notes.
- [ ] Run [helics-ns3 CI tests](https://github.com/GMLC-TDC/helics-ns3/actions/workflows/ci.yml) and fix any issues
- [ ] Run [Test HELICS Install Action](https://github.com/GMLC-TDC/helics-action/actions/workflows/test-helics-install.yml) with both binary and source options selected; fix any issues

---

If part of the [Update HELICS Packages workflow](https://github.com/GMLC-TDC/helics-packaging/actions/workflows/update-helics.yml) fails, the failing part can be re-run on its own:

- [Spack Package Update workflow](https://github.com/GMLC-TDC/helics-packaging/actions/workflows/update-spack-package.yml)
- [MINGW Package Update workflow](https://github.com/GMLC-TDC/helics-packaging/actions/workflows/update-mingw-package.yml)
- [Julia/Yggdrasil Package Update workflow](https://github.com/GMLC-TDC/helics-packaging/actions/workflows/update-yggdrasil-package.yml)
- [pyhelics Update HELICS Version workflow](https://github.com/GMLC-TDC/pyhelics/actions/workflows/update-helics.yml)
- [matHELICS Update HELICS Version workflow](https://github.com/GMLC-TDC/matHELICS/actions/workflows/update-helics.yml)

---

Steps to manually add a missing asset to a release:

1. If it was a temporary issue such as a resource being unavailable, manually trigger the [release build workflow](https://github.com/GMLC-TDC/HELICS/actions/workflows/release-build.yml) **using the workflow from `Tag: ${HELICS_VERSION}`**.
2. If it was a more major issue, create a branch based on the tagged release and modify the workflow and scripts in the .github subfolder as needed to make it build. Manually trigger the [release build workflow](https://github.com/GMLC-TDC/HELICS/actions/workflows/release-build.yml) **using the workflow from the branch with these fixes** and for the commit-ish field enter the release tag **${HELICS_VERSION}**.
3. Download the artifact from the workflow re-run that was missing and upload it to the release.
4. Download the SHA-256.txt file for the release and add an entry for the new asset (run the `sha256sum` command on Linux to get the line to add). Replace the old SHA-256.txt file for the release with the updated version.

@GMLC-TDC/helics-releases check release tasks related to interfaces/repositories you work on.
