# OSVR-Vive
> Maintained at <https://github.com/OSVR/OSVR-Vive>
>
> For details, see <http://osvr.github.io>
>
> Vive specific documentation at <https://github.com/OSVR/OSVR-Docs/blob/master/Configuring/HTC-Vive.md>

This is a plugin for OSVR that provides access to the tracker data on HTC Vive HMDs and controllers. It also contains tools to extract a display descriptor with distortion mesh data from a Vive, as well as additional tools that build from source but are not installed into binary snapshots.

## Compiling

To compile, this project requires OSVR, Eigen, and Boost, as well as the submodules included in the repository (clone with `git clone --recursive`). Compile as you would other CMake-based projects, setting `CMAKE_PREFIX_PATH` to show the way to dependencies in general. You may need to set `EIGEN3_INCLUDE_DIR` specifically.

You may also use a pre-compiled set of binaries from the project. They're available from <http://access.osvr.com/binary/vive>

## Developer links

These may be useful in keeping track of upstream changes to the lighthouse driver library.

- [SteamDB Info on SteamVR](https://steamdb.info/app/250820/depots/) - Scroll down to Branches to see the version numbers and build IDs.
    - [Public/Stable branch](https://steamdb.info/app/250820/depots/?branch=public)
    - [Beta branch](https://steamdb.info/app/250820/depots/?branch=beta)
- SteamDB Info for "OpenVR Depots" (contain the Lighthouse driver library used) for various platforms. Revisions include minimal file change details:
    - [Windows](https://steamdb.info/depot/250821/)
    - [Linux](https://steamdb.info/depot/250823/)
    - [OS X](https://steamdb.info/depot/250822/)

## Licenses
This plugin: Licensed under the Apache License, Version 2.0.

Vendored projects:
- Valve SteamVR `openvr` (specifically `openvr_driver` headers) - MIT license.

Note: At runtime, this plugin dynamically loads the Lighthouse SteamVR plugin distributed with SteamVR, as well as loads some SteamVR configuration settings (room calibration, etc) from JSON files.
