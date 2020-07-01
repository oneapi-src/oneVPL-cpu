#!/bin/bash
# ==============================================================================
# Copyright (C) 2018-2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

docker run -it --rm -v ${PWD}/../examples/content:/content vpl:18.04 /oneVPL/_build/hello_decode h265 /content/cars_1280x720.h265 out_1280x720.yuv
docker run -it --rm -v ${PWD}/../examples/content:/content vpl:20.04 /oneVPL/_build/hello_decode h265 /content/cars_1280x720.h265 out_1280x720.yuv
