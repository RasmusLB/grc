#!/bin/sh
export VOLK_GENERIC=1
export GR_DONT_LOAD_PREFS=1
export srcdir=/home/rlb/grc/modules/gr-compacter/lib
export PATH=/home/rlb/grc/modules/gr-compacter/build/lib:$PATH
export LD_LIBRARY_PATH=/home/rlb/grc/modules/gr-compacter/build/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$PYTHONPATH
test-compacter 
