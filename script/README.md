# Introduction

This folder contains scripts that represent common tasks a developer (or CI)
needs to perform when working on the project.

These are designed around the model described by
[Github](https://github.blog/2015-06-30-scripts-to-rule-them-all/)


# Scripts

- `lint` - Analyze project source for potential issues. Requires Python modules
  in `requirements-dev.txt`.

- `build` - Build the project and place output in `_build`.

- `package` - Create zip packages and place in `_build`.

- `cibuild` - Run CI steps. This is the script CI calls when it is triggered on
  a merge request.
