#!/bin/sh
export VOLK_GENERIC=1
export GR_DONT_LOAD_PREFS=1
export srcdir=/home/rlb/grc/modules/gr-compacter/python
export PATH=/home/rlb/grc/modules/gr-compacter/build/python:$PATH
export LD_LIBRARY_PATH=/home/rlb/grc/modules/gr-compacter/build/lib:$LD_LIBRARY_PATH
export PYTHONPATH=/home/rlb/grc/modules/gr-compacter/build/swig:$PYTHONPATH
/usr/bin/python2 /home/rlb/grc/modules/gr-compacter/python/qa_compact_file_sink.py 
