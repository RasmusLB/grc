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
from bitarray import bitarray
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
        dst = compacter.compact_file_sink("tester1234.rtl",False)
        self.tb.connect(src,dst)
        self.tb.run ()
        
        # check data
        #copyfile("tester123.rtl", "tester1234.rtl")
        f = open("tester123.rtl", "rb") # r: read   b: binary mode Absolute path: /home/ns3/Documents/grc/modules/gr-compacter/build/python/
        try:
            time_stamp = struct.unpack_from('d',f.read(8)) # 64 bit doubleprecision float: timestamp
            sample_rate = struct.unpack_from('Q',f.read(8)) # 64 bit unsigned int: sample rate
            fft_size = struct.unpack_from('I',f.read(4)) # 32 bit unsigned int: fft size
            center_freq = struct.unpack_from('Q',f.read(8)) # 64 bit unsigned int: center frequency
            # max_fft_bins = np.ceilnp.log2(fft_size)
            vector_no = struct.unpack_from('Q',f.read(8)) # 64 bit unsigned int: the fft output is computed on vectors of size fft_size. This variable gives the vector number
            print "time_stamp: " + str(time_stamp)
            print "sample_rate: " + str(sample_rate)
            print "fft_size: " + str(fft_size)
            print "center_freq: " + str(center_freq)
            print "vector_no: " + str(vector_no)
            print "struct.unpack_from(): " + str(struct.unpack_from('Q','\x00\x00\x00\x00\x00\x00\x00\x00'))
            dataDict = {}
            while vector_no != "":
                length_format = bitarray()
                length_format.fromfile(f, 4)
                compact_format = length_format[0]
                print "length_format: " + length_format.to01()
                print "compact_format: " + str(compact_format)
                length = length_format.tobytes() >> 1

                values = {}
                bin_no_size = np.ceil(np.log2(fft_size))
                bytes_to_read = np.ceil(( bin_no_size + 32) / 8)
                data = bitarray()
                data.fromfile(f,bytes_to_read)
                for i in range(length):
                    start = i * (bin_no_size + 32)
                    stop = i * (bin_no_size + 32) + bin_no_size
                    bin_no = data[start:stop]
                    print "bin_no: " + bin_no
                    value = b.read(32) # 32 bit float
                vector_no = ""
            result_data = (-50, -55) ####################
            self.assertEqual((2000000,), sample_rate)
            self.assertFloatTuplesAlmostEqual (expected_result, result_data, 2)

        finally:
            f.close()


if __name__ == '__main__':
    gr_unittest.run(qa_compact_file_sink, "qa_compact_file_sink.xml")
