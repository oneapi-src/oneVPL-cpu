# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed

- Version of GoogleTest to 1.12.1

### Fixed

- Linker hardening flags being incorrectly set on Linux

## [2023.1.1] - 2022-12-23

### Changed

- README to add FFmpeg disclaimer.


## [2023.1.0] - 2022-11-10

### Fixed

- Wrong VPP out surface cropW/H info

### Added

- Changlog file

## [2023.0.0] - 2022-10-14

### Changed
- Versions of third party components used to more recent versions

### Fixed
- Build scripts not returning error on bad args

## [2022.2.0] - 2022-07-22

### Changed
- Build scripts to check for bash

## [2022.1.6] - 2022-07-08

### Added
- Additional code hardening on Windows*


## [2022.1.4] - 2022-05-27

### Changed

- NumFrameSuggested to increase by AsyncDepth

## [2022.1.3] - 2022-05-13

### Fixed

- Incorrect license being reported in capabilities query

## [2022.1.2] - 2022-04-29

### Changed

- Versions of third party components used to more recent versions


## [2022.1.1] - 2022-04-15

### Added

- Encode capabilities reporting support

## [2022.1.0] - 2022-03-18

### Changed

- `POSITION_INDEPENDENT_CODE` property to be set for all targets

## [2022.0.6] - 2022-03-04

### Changed

- Build to always support H264 encode by default

## [2022.0.5] - 2022-02-18

### Added
- Optional generation of IVF headers for AV1 encode

## [2022.0.4] - 2022-02-04

### Added
- H264 encoder with a permissive license

### Deprecated
- Support for Microsoft Visual Studio* 2017

## [2022.0.3] - 2022-01-21

### Added
- MPEG2 decode support

### Fixed
- HEVC encode crashing if GOP size = 1 with CPU implementation

## [2022.0.2] - 2021-12-16

### Added
- Extended device ID query support

## [2022.0.0] - 2021-12-06

### Changed
- Build scripts to be more robust and flexible

## [2021.6.0] - 2021-09-13

### Added
- oneVPL API 2.4 support
- HEVC 4:2:2 planar decode support

## [2021.5.0] - 2021-08-03

### Added
- Improved warnings for incompatible parameters

### Changed
- API version dependency to 2.4

### Fixed
- Numerous minor bugs

## [2021.4.0] - 2021-06-25

### Added

- oneVPL API 2.4 support

## [2021.2.2] - 2021-04-01

### Added
- oneVPL API 2.2 support
- 32-bit Windows* support
- Ubuntu 20.10 support

## [2021.1] - 2020-12-08

### Added
- CPU runtime implementation supporting oneVPL API 2.0
- H.264/AVC encode and decode support
- H.265/HEVC encode and decode support
- MJPEG encode and decode support
- AV1 encode and decode support



[Unreleased]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2023.1.0...HEAD
[2023.1.0]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2023.0.0...v2023.1.0
[2023.0.0]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2022.2.4...v2023.0.0
[2022.2.4]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2022.2.3...v2022.2.4
[2022.2.3]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2022.2.2...v2022.2.3
[2022.2.2]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2022.2.1...v2022.2.2
[2022.2.1]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2022.2.0...v2022.2.1
[2022.2.0]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2022.1.6...v2022.2.0
[2022.1.6]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2022.1.5...v2022.1.6
[2022.1.5]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2022.1.4...v2022.1.5
[2022.1.4]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2022.1.3...v2022.1.4
[2022.1.3]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2022.1.2...v2022.1.3
[2022.1.2]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2022.1.1...v2022.1.2
[2022.1.1]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2022.1.0...v2022.1.1
[2022.1.0]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2022.0.6...v2022.1.0
[2022.0.6]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2022.0.5...v2022.0.6
[2022.0.5]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2022.0.4...v2022.0.5
[2022.0.4]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2022.0.3...v2022.0.4
[2022.0.3]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2022.0.2...v2022.0.3
[2022.0.2]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2022.0.0...v2022.0.2
[2022.0.0]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2021.6.0...v2022.0.0
[2021.6.0]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2021.5.0...v2021.6.0
[2021.5.0]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2021.4.0...v2021.5.0
[2021.4.0]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2021.2.2...v2021.4.0
[2021.2.2]: https://github.com/oneapi-src/oneVPL-cpu/compare/v2021.1]:...v2021.2.2
[2021.1]: https://github.com/oneapi-src/oneVPL-cpu/releases/tag/v2021.1
