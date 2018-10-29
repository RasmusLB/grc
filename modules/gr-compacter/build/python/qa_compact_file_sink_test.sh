#!/bin/sh
export VOLK_GENERIC=1
export GR_DONT_LOAD_PREFS=1
export srcdir=/home/ns3/Documents/grc/modules/gr-compacter/python
export PATH=/home/ns3/Documents/grc/modules/gr-compacter/build/python:$PATH
export LD_LIBRARY_PATH=/home/ns3/Documents/grc/modules/gr-compacter/build/lib:$LD_LIBRARY_PATH
export PYTHONPATH=/home/ns3/Documents/grc/modules/gr-compacter/build/swig:$PYTHONPATH
/usr/bin/python2 /home/ns3/Documents/grc/modules/gr-compacter/python/qa_compact_file_sink.py 
