#!/bin/bash

XCODE_MACOS_PLATFORM_PATH="/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform"
XCODE_MACOS_SDK_PATH="${XCODE_MACOS_PLATFORM_PATH}/Developer/SDKs"

SDK_VER=10.9

# Download macOS SDK
wget "https://github.com/phracker/MacOSX-SDKs/releases/download/10.15/MacOSX${SDK_VER}.sdk.tar.xz"

# Extract macOS SDK to the XCode macOS SDKs folder
tar -xf "MacOSX${SDK_VER}.sdk.tar.xz" -C "${XCODE_MACOS_SDK_PATH}"

# Set the MinimumSDKVersion for XCode
/usr/libexec/PlistBuddy -c "Set :MinimumSDKVersion ${SDK_VER}" "${XCODE_MACOS_PLATFORM_PATH}/Info.plist"
