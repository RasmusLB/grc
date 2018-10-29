/* -*- c++ -*- */

#define COMPACTER_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "compacter_swig_doc.i"

%{
#include "compacter/compact_file_sink.h"
%}


%include "compacter/compact_file_sink.h"
GR_SWIG_BLOCK_MAGIC2(compacter, compact_file_sink);
