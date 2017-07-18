#!/bin/sh

set -e
# Run unittest
./seims_longterm_osx_xcode/seims/bin/seims_ut_exec
./seims_storm_osx_xcode/seims/bin/seims_ut_exec
