# Introduction

This is a speech synthesis voice for macOS. It does not output any audio;
instead, it sends TCP messages on the local network when any software uses the
macOS text-to-speech system.

## Prequisites

You must secure Apple Inc.'s permission to develop for their platform. [Create
an Apple ID](https://appleid.apple.com). As of October 2022, the company does
not charge a fee for this account.

## Installation

To build and install the voice on the local system:

1. Install Xcode through the Apple App Store
2. Run `sudo xcodebuild install DSTROOT=/`
3. Run `sudo pkill -f com.apple.speech.speechsynthesisd`

## Usage

1. Set up a local TCP server listening on port 4449.
2. Set speech output to Cher.
3. All speech requests will be sent as TCP messages to the local server.

## Packaging

It is possible to create a package suitable for installation in another macOS
environment (macOS 10.15 or later) which does not have the requisite build
tools.

Prerequisites:

- 20 gigabytes of available hard drive space
- a connection to the Internet

Procedure:

1. **Download the Mac OSX 10.15 SDK** - this is included in Xcode 11.7 and can
   be downloaded from Apple using a valid AppleID using [the link maintained
   here](https://github.com/devernay/xcodelegacy))
2. **Install the SDK** - expand the `.xip` file and move the SDK directory to
   the Xcode application's "developer platform" directory, e.g. via the
   following command:

       mv Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk
3. **Build the package** - run the following command:

       make

This will produce a file named `dist/AutomationVoice.pkg` which can be used to
install the voice on other macOS systems.
