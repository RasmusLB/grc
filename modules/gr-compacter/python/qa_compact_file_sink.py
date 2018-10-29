#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2018 <+YOU OR YOUR COMPANY+>.
# 
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
# 

from gnuradio import gr, gr_unittest
from gnuradio import blocks
from bitstring import Bits
from shutil import copyfile
import numpy as np
import compacter_swig as compacter
import struct

class qa_compact_file_sink (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def test_001_t (self):
        # set up fg
        src_data = (-50, -99, -63, -90, -97, -77, -55, -89)
        expected_result = (-50, -55)
        src = blocks.vector_source_f(src_data)
        dst = compacter.compact_file_sink("tester123.rtl",False)
        self.tb.connect(src,dst)
        self.tb.run ()
        
        # check data
        copyfile("tester123.rtl", "tester1234.rtl")
        f = open("tester1234.rtl", "rb") # r: read   b: binary mode Absolute path: /home/ns3/Documents/grc/modules/gr-compacter/build/python/
        try:
            time_stamp = struct.unpack_from('d',f.read(8)) # 64 bit doubleprecision float: timestamp
            sample_rate = struct.unpack_from('Q',f.read(8)) # 64 bit unsigned int: sample rate
            fft_size = struct.unpack_from('I',f.read(4)) # 32 bit unsigned int: fft size
            center_freq = struct.unpack_from('Q',f.read(8)) # 64 bit unsigned int: center frequency
            max_fft_bins = np.log2(fft_size)
            '''fft_vec_no = struct.unpack_from('I',f.read(4)) # 32 bit unsigned int: the fft output is computed on vectors of size fft_size. This variable gives the vector number
            while fft_vec_no != "":
                compact_format = b.read(1)
                length = b.read(max_fft_bins)
                while length > 0:
                    fft_vec_no = b.read(max_fft_bins)
                    value = b.read(32) # 32 bit float
                    length -= 1
                fft_vec_no = b.read(32)'''
            result_data = (-50, -55) ####################
            self.assertEqual((2000000,), sample_rate)
            self.assertFloatTuplesAlmostEqual (expected_result, result_data, 2)

        finally:
            f.close()


if __name__ == '__main__':
    gr_unittest.run(qa_compact_file_sink, "qa_compact_file_sink.xml")
