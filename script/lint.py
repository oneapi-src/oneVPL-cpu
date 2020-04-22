#!/usr/bin/env python3
###############################################################################
# Copyright (C) 2020 Intel Corporation
#
# SPDX-License-Identifier: MIT
###############################################################################
"""
Analyze project source for potential issues.

Just calls .bat or .sh files of the same name.

"""

import platform
import sys
import subprocess
import os


def run():
    """Main entry point
    """
    script_basename = os.path.splitext(os.path.abspath(__file__))[0]
    if platform.system() == 'Windows':
        script_name = script_basename + '.bat'
    else:
        script_name = script_basename
    result = subprocess.run(script_name, check=False)
    return result.returncode


if __name__ == '__main__':
    sys.exit(run())
