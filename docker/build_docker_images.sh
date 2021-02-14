#!/bin/bash
# ==============================================================================
# Copyright (C) 2018-2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

set -e
BASEDIR=$(dirname "$0")
docker build -f ${BASEDIR}/Dockerfile-centos-7-GPL -t vpl-cpu:centos7-GPL ${BASEDIR}/..
docker build -f ${BASEDIR}/Dockerfile-centos-7     -t vpl-cpu:centos7     ${BASEDIR}/..
docker build -f ${BASEDIR}/Dockerfile-centos-8     -t vpl-cpu:centos8     ${BASEDIR}/..
docker build -f ${BASEDIR}/Dockerfile-ubuntu-18.04 -t vpl-cpu:ubuntu18.04 ${BASEDIR}/..

if [ "$(uname -m)" == "x86_64" ]; then
	docker build -f ${BASEDIR}/Dockerfile-ubuntu-19.10 -t vpl-cpu:ubuntu19.10 ${BASEDIR}/..
fi

docker build -f ${BASEDIR}/Dockerfile-ubuntu-20.04 -t vpl-cpu:ubuntu20.04 ${BASEDIR}/..
