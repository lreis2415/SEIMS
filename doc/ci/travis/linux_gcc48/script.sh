#!/bin/sh

set -e
# Run unittest
./seims_longterm_linux_gcc48/seims/bin/seims_ut_exec
./seims_storm_linux_gcc48/seims/bin/seims_ut_exec
