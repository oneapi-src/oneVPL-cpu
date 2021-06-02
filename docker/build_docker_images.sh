#!/bin/bash
# ==============================================================================
# Copyright (C) 2018-2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
# ==============================================================================

set -e
BASEDIR=$(dirname "$0")
echo "Building CentOS 7 GPL Image"
docker build -f ${BASEDIR}/Dockerfile-centos-7-GPL -t vpl-cpu:centos7-GPL ${BASEDIR}/..
echo "Building CentOS 7 Image"
docker build -f ${BASEDIR}/Dockerfile-centos-7     -t vpl-cpu:centos7     ${BASEDIR}/..
echo "Building CentOS 8 Image"
docker build -f ${BASEDIR}/Dockerfile-centos-8     -t vpl-cpu:centos8     ${BASEDIR}/..
echo "Building Ubuntu 18.04 Image"
docker build -f ${BASEDIR}/Dockerfile-ubuntu-18.04 -t vpl-cpu:ubuntu18.04 ${BASEDIR}/..
echo "Building Ubuntu 20.04 Image"
docker build -f ${BASEDIR}/Dockerfile-ubuntu-20.04 -t vpl-cpu:ubuntu20.04 ${BASEDIR}/..
