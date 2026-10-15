@echo off
REM Author: Michael Male <michael.male@met.police.uk>
REM Copyright (c) 2026 Crown Copyright (Metropolitan Police Service)
REM Produced using an AI coding assistant (Google Antigravity).
REM
REM This file is part of dlt-to-csv, derived from COVESA Project Dlt Viewer
REM (https://github.com/COVESA/dlt-viewer).
REM
REM This Source Code Form is subject to the terms of the Mozilla Public
REM License, v. 2.0. If a copy of the MPL was not distributed with this
REM file, You can obtain one at http://mozilla.org/MPL/2.0/.
REM
REM For further information see http://www.covesa.global/.
REM
REM Build script for standalone dlt-to-csv.exe using PyInstaller
echo ========================================================
echo Building standalone DLT to CSV Forensic CLI Executable
echo ========================================================

python -m PyInstaller --clean --onefile --name dlt-to-csv dlt_to_csv.py

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================================
    echo Build SUCCESS! Standalone executable generated:
    echo dist\dlt-to-csv.exe
    echo ========================================================
) else (
    echo.
    echo [!] Build failed with error code %ERRORLEVEL%.
)
