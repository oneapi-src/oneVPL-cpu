# Introduction

This folder contains scripts that represent common tasks a developer (or CI)
needs to perform when working on the project.

The scripts are designed around the model described by
[Github](https://github.blog/2015-06-30-scripts-to-rule-them-all/)


# Scripts

- `lint` - Analyze project source for potential issues, including compliance with coding style. Requires Python modules
  in `requirements-dev.txt`.

- `bootstrap` - Build the project dependencies and place in the location
  specified by the `VPL_BUILD_DEPENDENCIES` environment variable. If environment
  variable is undefined, define it as and place output in `_deps`.

- `build` - Build the project and place output in `_build`.

- `package` - Create zip packages and place in `_build`.

  -  `*-all.zip` Contains the files in the complete product installation

  -  `*-dev.zip` Contains the files for the developer package

  -  `*-runtime.zip` Contains the files for the runtime package

- `test` - Run smoke tests.

- `cibuild` - Run CI steps. CI calls this script when it is triggered on
  a merge request.

- `stress` - Run stress tests.

  -  stress `n` : run `n` time(s) of stress test 

# Environment Variables

While no environment variables are strictly required to use these scripts 
there are environment variables that, if set, allow control of how and
where the scripts build and place files.

## Common

These settings are shared with the dispatcher.

- `VPL_INSTALL_DIR` - The root folder in which to install after building.
By default files will be installed in a system location such as `/` or `~/`
however developers will often want to segregate code being developed.
If this is set VPL build scripts will place files under that folder and
use files found under that folder to find cross dependencies.

## CPU Specific

- `VPL_CPU_DEPS_BUILD_DIR` - The folder where bootstrap will download and build dependencies. (Defaults to `<script-dir>/../_extbuild/`)

- `VPL_BUILD_DEPENDENCIES` - Folder where built dependencies are stored. (Defaults
to `<script-dir>/../_deps/`)

- `VPL_CPU_BUILD_DIR` - The folder to be used by CMake for building. 
(Defaults to `<script-dir>/../_build/`)


**LEGAL NOTICE:  By downloading and using any scripts (the “Software Package”) and the included software or software made available for download, you agree to the terms and conditions of the software license agreements for the Software Package, which may also include notices, disclaimers, or license terms for third party software (together, the “Agreements”) included in this README file.
If the Software Package is installed through a silent install, your download and use of the Software Package indicates your acceptance of the Agreements.**
