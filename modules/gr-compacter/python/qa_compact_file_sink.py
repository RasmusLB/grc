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
import compacter_swig as compacter
import struct

class qa_compact_file_sink (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

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
        if vector_no == "":
            return -1
        return struct.unpack_from('Q',vector_no)[0]

    def test_001_t (self):
        # set up fg
        src_data = (-50, -99, -63, -90, -97, -77, -55, -89, -50, -20, -105, - 25)
        expected_result = (-50, -55)
        src = blocks.vector_source_f(src_data)
        dst = compacter.compact_file_sink("tester123.rtl",False)
        self.tb.connect(src,dst)
        self.tb.run ()
        
        # check data
        #copyfile("tester123.rtl", "tester1234.rtl")
        f = open("tester1234.rtl", "rb") # r: read   b: binary mode Absolute path: /home/ns3/Documents/grc/modules/gr-compacter/build/python/
        try:
            time_stamp = struct.unpack_from('d',f.read(8))[0] # 64 bit doubleprecision float: timestamp
            sample_rate = struct.unpack_from('Q',f.read(8))[0] # 64 bit unsigned int: sample rate
            fft_size = struct.unpack_from('I',f.read(4))[0] # 32 bit unsigned int: fft size
            center_freq = struct.unpack_from('Q',f.read(8))[0] # 64 bit unsigned int: center frequency
            bin_size_bits = np.ceil(np.log2(fft_size))
            vector_no = self.read_vector_no(f) # 64 bit unsigned int: the fft output is computed on vectors of size fft_size. This variable gives the vector number
            print "time_stamp: " + str(time_stamp)
            print "sample_rate: " + str(sample_rate)
            print "fft_size: " + str(fft_size)
            print "center_freq: " + str(center_freq)
            print "vector_no: " + str(vector_no)
            dataDict = {}
            while vector_no != -1:
                compact_length, compact_format = self.read_length_and_format(f)
                print "compact_length: " + str(compact_length)
                print "compact_format: " + str(compact_format)

                index_value_list = []
                bin_no_size = np.ceil(np.log2(fft_size))
                bytes_to_read = 0
                if compact_format == True:
                    
                    bytes_to_read = (int) (np.ceil(compact_length * (32 + bin_size_bits)/8))
                    print "format 1"
                elif compact_format == False:

                    bytes_to_read = (int) (4*compact_length)
                    print "format 0"
                data = self.read_to_bitarray(f,bytes_to_read)
                
                print "bytes to read: " + str(bytes_to_read)
                #print "data: " + data.to01() + " length: " + str(len(data))

                if compact_format == 1:
                    for i in range(compact_length):
                        start_bit = (int) (i * (bin_no_size + 32))
                        stop_bit = (int) (i * (bin_no_size + 32) + bin_no_size)
                        bin_no = self.bits_to_int(data[start_bit:stop_bit])
                        start_bit = stop_bit
                        stop_bit = stop_bit + 32
                        value = self.bits_to_float(data[start_bit:stop_bit])
                        index_value_list.append((bin_no,value))
                elif compact_format == 0:
                    for i in range(compact_length):
                        start_bit = (int) (i * (32))
                        stop_bit = (int) (i * (32) + 32)
                        bin_no = i
                        value = self.bits_to_float(data[start_bit:stop_bit])
                        index_value_list.append((bin_no,value))
                dataDict[vector_no] = index_value_list
                
                vector_no = self.read_vector_no(f)
                print "dataDict: " + str(dataDict)
            result_data = {0: [(0, -50.0), (6, -55.0), (8, -50.0), (9, -20.0), (11, -25.0)]} 
            self.assertEqual(2000000, sample_rate)
            for vector_no in result_data:
                self.assertEqual(result_data[vector_no],dataDict[vector_no])

        finally:
            f.close()


if __name__ == '__main__':
    gr_unittest.run(qa_compact_file_sink, "qa_compact_file_sink.xml")
