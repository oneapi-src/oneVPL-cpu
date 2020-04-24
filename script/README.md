# Introduction

This folder contains scripts that represent common tasks a developer (or CI)
needs to perform when working on the project.

These are designed around the model described by
[Github](https://github.blog/2015-06-30-scripts-to-rule-them-all/)


# Scripts

Many scripts have both native and python versions. In that case the python
version is simply a wrapper that calls the most appropriate native script.

- `lint` - Analyze project source for potential issues. Requires Python modules
  in `requirements-dev.txt`.
