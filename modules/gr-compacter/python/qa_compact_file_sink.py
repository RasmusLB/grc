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
from bitstring import BitArray
from bitstring import ConstBitStream
from shutil import copyfile
import numpy as np
import tempfile
import compacter_swig as compacter
import struct
import os

class qa_compact_file_sink (gr_unittest.TestCase):

    def setUp (self):
        os.environ['GR_CONF_CONTROLPORT_ON'] = 'False'
        self.tb = gr.top_block ()


    def tearDown (self):
        self.tb = None


    def test_001_t (self):
        print "Test_001: Check that unpacking format=1 works"
        src_data = (-50, -99, -63, -90, -97, -77, -55, -89, -50, -20, -105, - 25) + (-87,) * 1011 + (-12,)
        self.feed_data_t(src_data)
        print "Test_001: Completed \n" 


    def test_002_t (self):
        print "Test_002: Check that unpacking format=0 works"
        src_data = (-50, -99, -63, -90, -97, -77, -55, -89, -50, -20, -105, - 25) + (-8,) * 1011 + (-12,)
        self.feed_data_t(src_data)
        print "Test_002: Completed \n"


    def test_003_t (self):
        print "Test_003: Check that unpacking a file without values works"
        src_data = (-65, -99, -63, -90, -97, -77, -82, -89, -82, -86, -105, - 96) + (-87,) * 1011 + (-112,)
        self.feed_data_t(src_data)
        print "Test_003: Completed \n"


    def feed_data_t (self,src_data):
        # set up test
        d_fft_size = 1024
        d_sample_rate = 2000000
        d_center_freq = 868300000
        d_compact_threshold = -63
        expected_result = self.expected_result(src_data,d_fft_size,d_compact_threshold)
        with tempfile.NamedTemporaryFile() as temp:
            src = blocks.vector_source_f(src_data)
            dst = compacter.compact_file_sink(temp.name,False)
            dst.set_unbuffered(True)
            dst.set_sample_rate(d_sample_rate)
            dst.set_fft_size(d_fft_size)
            dst.set_center_freq(d_center_freq)
            dst.set_compact_threshold(d_compact_threshold)
            self.tb.connect(src,dst)
            self.tb.run ()
            # check data
            file_size = os.stat(temp.name).st_size
            assert(file_size > 0)
            time_stamp, sample_rate, fft_size, center_freq, dataDict = self.extract_file(temp.name)
            self.assertEqual(d_sample_rate, sample_rate)
            self.assertEqual(d_fft_size, fft_size)
            self.assertEqual(d_center_freq, center_freq)
            for vector_no in expected_result:
                for i in range(len(expected_result[vector_no])):
                    self.assertEqual(expected_result[vector_no][i],dataDict[vector_no][i])


    def expected_result(self,src_data,fft_size,compact_threshold):
        compact_items = 0
        for i in range(len(src_data)):
            if src_data[i] > compact_threshold:
                compact_items = compact_items + 1
        format_0_byte_size = 8 + 4 + fft_size * 4
        bit_length = 64 + 32 + compact_items * (32 + np.ceil(np.log2(fft_size)))
        padding_bits = (8 - (bit_length % 8))%8
        format_1_byte_size = (bit_length + padding_bits) / 8 
        lst = []
        if format_0_byte_size < format_1_byte_size:
            for i in range(len(src_data)):
                lst.append((i,src_data[i]))
        else:
            for i in range(len(src_data)):
                if src_data[i] > compact_threshold:
                    lst.append((i,src_data[i]))
        return {0:lst}

    def read_values(self,compact_format,compact_length,bin_size_bits,f):
        print "format:" + str(compact_format)
        index_value_list = []
        if compact_format == False:
            bytes_to_read = (int) (4*compact_length)
            print "bytes to read: " + str(bytes_to_read)
            data = self.read_to_bitarray(f,bytes_to_read)
            for i in range(compact_length):
                start_bit = (int) (i * (32))
                stop_bit = (int) (i * (32) + 32)
                bin_no = i
                value = self.bits_to_float(data[start_bit:stop_bit])
                index_value_list.append((bin_no,value))
        elif compact_format == True:
            bytes_to_read = (int) (np.ceil(compact_length * (32 + bin_size_bits)/8))
            data = self.read_to_bitarray(f,bytes_to_read)
            print "bytes to read: " + str(bytes_to_read)
            for i in range(compact_length):
                start_bit = (int) (i * (bin_size_bits + 32))
                stop_bit = (int) (i * (bin_size_bits + 32) + bin_size_bits)
                bin_no = self.bits_to_int(data[start_bit:stop_bit])
                start_bit = stop_bit
                stop_bit = stop_bit + 32
                value = self.bits_to_float(data[start_bit:stop_bit])
                index_value_list.append((bin_no,value))
        else:
            print "ERROR: Unknown compact_format: " + str(compact_format)
            return [-1,]
        return index_value_list


    def extract_file(self,filename):
        # check data
        #copyfile("tester123.rtl", "tester1234.rtl")
        f = open(filename, "rb") # r: read   b: binary mode Absolute path: /home/ns3/Documents/grc/modules/gr-compacter/build/python/
        try:
            time_stamp = struct.unpack_from('d',f.read(8))[0] # 64 bit doubleprecision float: timestamp
            sample_rate = struct.unpack_from('Q',f.read(8))[0] # 64 bit unsigned int: sample rate
            fft_size = struct.unpack_from('I',f.read(4))[0] # 32 bit unsigned int: fft size
            center_freq = struct.unpack_from('Q',f.read(8))[0] # 64 bit unsigned int: center frequency
            bin_size_bits = np.ceil(np.log2(fft_size))
            vector_no = self.read_vector_no(f) # 64 bit unsigned int: the fft output is computed on vectors of size fft_size. This variable gives the vector number

            dataDict = {}
            while vector_no != -1:
                compact_length, compact_format = self.read_length_and_format(f)
                print "compact_length: " + str(compact_length)
                print "compact_format: " + str(compact_format)
                dataDict[vector_no] = self.read_values(compact_format,compact_length,bin_size_bits,f)
                vector_no = self.read_vector_no(f)
            return time_stamp, sample_rate, fft_size, center_freq, dataDict
        finally:
            f.close()

    def bits_to_int (self,bitarray):
        bitarray.fill()
        bitarray.reverse()
        if bitarray.length() == 16:
            fmt = '>H'
        elif bitarray.length() == 8:
            fmt = '>B'
        return struct.unpack_from(fmt,bitarray.tobytes())[0]

    def bits_to_float (self,bitarray):
        bitarray.reverse()
        #print "value: " + bitarray.to01()
        return struct.unpack_from('>f',bitarray.tobytes())[0]

    def read_length_and_format (self,f):
        packed = struct.unpack_from('I',f.read(4))[0] # 31 bit unsigned int + 1 bit flag = 32 bit
        compact_format = packed & 0x1
        compact_length = packed >> 1
        return compact_length, compact_format

    def read_to_bitarray(self,f,bytes_to_read):
        data = bitarray()
        data.fromfile(f,bytes_to_read)
        for i in range(bytes_to_read):
            tmp = data[(i * 8):(i * 8 + 8)]
            tmp.reverse()
            data[(i * 8):(i * 8 + 8)] = tmp
        return data

    def read_vector_no(self,f):
        vector_no = f.read(8)
        print "vector_no being read"
        if vector_no == "":
            return -1
        return struct.unpack_from('Q',vector_no)[0]

if __name__ == '__main__':
    gr_unittest.run(qa_compact_file_sink, "qa_compact_file_sink.xml")
