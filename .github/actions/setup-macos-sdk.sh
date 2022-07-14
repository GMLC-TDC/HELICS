#!/bin/bash

XCODE_MACOS_PLATFORM_PATH="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform"
XCODE_MACOS_SDK_PATH="${XCODE_MACOS_PLATFORM_PATH}/Developer/SDKs"

if [[ -z "${MACOS_SDK_VER}" ]]; then
    MACOS_SDK_VER=10.15
fi

# Download macOS SDK
wget "https://github.com/phracker/MacOSX-SDKs/releases/download/10.15/MacOSX${MACOS_SDK_VER}.sdk.tar.xz"

# Extract macOS SDK to the XCode macOS SDKs folder
tar -xf "MacOSX${MACOS_SDK_VER}.sdk.tar.xz" -C "${XCODE_MACOS_SDK_PATH}"

# Set the MinimumSDKVersion for XCode
/usr/libexec/PlistBuddy -c "Set :MinimumSDKVersion ${MACOS_SDK_VER}" "${XCODE_MACOS_PLATFORM_PATH}/Info.plist"
