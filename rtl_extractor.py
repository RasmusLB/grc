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
import struct
import os

class rtl_extractor ():

  
    def expected_result(self,src_data,fft_size,compact_threshold):
        compact_items = 0 # A compact item is a reading which falls within the threshold
        dataDict = {} 
        for j in range(len(src_data)/fft_size):
            for i in range(fft_size):
                if src_data[i+j*fft_size] > compact_threshold:
                    compact_items = compact_items + 1
            format_0_byte_size = 8 + 4 + fft_size * 4
            bit_length = 64 + 32 + compact_items * (32 + np.ceil(np.log2(fft_size)))
            padding_bits = (8 - (bit_length % 8))%8
            format_1_byte_size = (bit_length + padding_bits) / 8 
            lst = []
            if format_0_byte_size < format_1_byte_size:
                for i in range(fft_size):
                    lst.append((i,src_data[i+j*fft_size]))
            else:
                for i in range(fft_size):
                    if src_data[i+j*fft_size] > compact_threshold:
                        lst.append((i,src_data[i+j*fft_size]))
            dataDict[j] = lst
        return dataDict

    def read_values(self,compact_format,compact_length,bin_size_bits,f):
        index_value_list = []
        if compact_format == False:
            bytes_to_read = (int) (4*compact_length)
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

if __name__ == '__main__':
    extr = rtl_extractor()
    time_stamp, sample_rate, fft_size, center_freq, dataDict = extr.extract_file("testfile.rtl")
    print "time_stamp: " + str(time_stamp)
    print "sample_rate: " + str(sample_rate)
    print "fft_size: " + str(fft_size)
    print "center_freq: " + str(center_freq)
    for i in range (len(dataDict)):
        if len(dataDict[i]) > 0:
            print "dataDict[" + str(i) + "]: " + str(dataDict[i])


