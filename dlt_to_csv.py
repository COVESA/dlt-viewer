#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Author: Michael Male <michael.male@met.police.uk>
# Copyright (c) 2026 Crown Copyright (Metropolitan Police Service)
#
# Produced using an AI coding assistant (Google Antigravity).
#
# This file is part of dlt-to-csv, derived from COVESA Project Dlt Viewer
# (https://github.com/COVESA/dlt-viewer).
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# For further information see http://www.covesa.global/.

"""DLT-to-CSV Forensic Converter CLI Entrypoint.

Usage:
    python dlt_to_csv.py <folder_or_files> [options]

For help:
    python dlt_to_csv.py --help
"""

import multiprocessing
import sys
from dlt_to_csv.cli import main

if __name__ == "__main__":
    multiprocessing.freeze_support()
    sys.exit(main())
