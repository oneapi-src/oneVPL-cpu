#!/bin/bash
# ==============================================================================
# Copyright (C) 2018-2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================
# oneAPI Video Processing Library (oneVPL) docker sample execution script

docker run -it --rm -v ${PWD}/../test/content:/content vpl-ubuntu:18.04 /opt/intel/onevpl/bin/vpl-decode -i /content/cars_320x240.h265 -o out_320x240.yuv -if h265
docker run -it --rm -v ${PWD}/../test/content:/content vpl-ubuntu:20.04 /opt/intel/onevpl/bin/vpl-decode -i /content/cars_320x240.h265 -o out_320x240.yuv -if h265
docker run -it --rm -v ${PWD}/../test/content:/content vpl-centos:8 /opt/intel/onevpl/bin/vpl-decode -i /content/cars_320x240.h265 -o out_320x240.yuv -if h265

