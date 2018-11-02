/* -*- c++ -*- */
/* 
 * Copyright 2018 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "compact_file_sink_impl.h"
#include <gnuradio/io_signature.h>
#include <stdexcept>

namespace gr {
  namespace compacter {

    compact_file_sink::sptr
    compact_file_sink::make(size_t itemsize, const char *filename, bool append)
    {
      return gnuradio::get_initial_sptr
        (new compact_file_sink_impl(itemsize, filename, append));
    }

    /*
     * The private constructor
     */
    compact_file_sink_impl::compact_file_sink_impl(size_t itemsize, const char *filename, bool append)
      : gr::sync_block("compact_file_sink",
              gr::io_signature::make(1, 1, sizeof(float) * itemsize),
              gr::io_signature::make(0, 0, 0)),
        file_sink_base(filename, true, append),
        d_itemsize(itemsize)
    {
      // Set variables - should be overwritten
      d_time_stamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();   
      set_sample_rate(2000000);
      set_fft_size(1024);
      set_center_freq(868300000);
	 set_compact_threshold(-63);
    }

    /*
     * Our virtual destructor.
     */
    compact_file_sink_impl::~compact_file_sink_impl()
    {
      
    }

    /*
     * Setters and getters
     */
    void 
    compact_file_sink_impl::set_sample_rate(uint64_t sample_rate)
    {
      d_sample_rate = sample_rate;
    }

    uint64_t 
    compact_file_sink_impl::sample_rate()
    {
      return d_sample_rate;
    }
   
    void 
    compact_file_sink_impl::set_fft_size(uint32_t fft_size)
    {
      d_bin_no_bit_size = static_cast<uint32_t>(std::ceil(log2(fft_size)));
      d_fft_size = fft_size;
    }

    uint32_t 
    compact_file_sink_impl::fft_size()
    {
      return d_fft_size;
    }
      

    void 
    compact_file_sink_impl::set_center_freq(uint64_t center_freq)
    {
      d_center_freq = center_freq;
    }
	 

    uint64_t 
    compact_file_sink_impl::center_freq()
    {
      return d_center_freq;
    }

    void 
    compact_file_sink_impl::set_compact_threshold(float compact_threshold)
    {
      d_compact_threshold = compact_threshold;
    }


    float 
    compact_file_sink_impl::compact_threshold()
    {
      return d_compact_threshold;
    }

    void 
    compact_file_sink_impl::set_unbuffered(bool unbuffered)
    {
      d_unbuffered = unbuffered;
    }

    /*
     * Actual work carried out here
     * noutput_items is the number of items pointed to by &input_items
     */
    int
    compact_file_sink_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
std::cout << "noutput_items: " << noutput_items << std::endl;
      const float *inbuf = (const float *) input_items[0];
      int  nwritten = 0;

	 do_update();

	 int meta_bytes = 0;
	 // Write actual data to file
	 if (d_vector_no == 0)
	 {
	   // Write meta data to file
        meta_bytes = fwrite(&d_time_stamp, sizeof(d_time_stamp), 1, d_fp) * sizeof(d_time_stamp) ;      
        meta_bytes += fwrite(&d_sample_rate, sizeof(d_sample_rate), 1, d_fp) * sizeof(d_sample_rate);
        meta_bytes += fwrite(&d_fft_size, sizeof(d_fft_size), 1, d_fp) * sizeof(d_fft_size);
        meta_bytes += fwrite(&d_center_freq, sizeof(d_center_freq), 1, d_fp) * sizeof(d_center_freq);
	 }
      // Create one long byte array, and write it to file
	 for (int i = 0; i < noutput_items; i++)
	 {
 	   int compact_size = 0; // number of bytes in compact_buf
        boost::shared_ptr<char[]> compact_buf = compact(&inbuf[i*d_itemsize], &compact_size, d_itemsize);
	   int count = fwrite(compact_buf.get(), compact_size, 1, d_fp);
	 }
      // Option to force writing to file (used in test functions)
	 if (d_unbuffered)
	   fflush(d_fp);
      // Tell runtime system how many output items we produced.
      return noutput_items;
    }


    boost::shared_ptr<char[]>
    compact_file_sink_impl::compact(const float *inbuf, int *compact_size, int noutput_items)
    {
      int compact_items = 0; // count the number of values within threshold
	 for(int i = 0; i < noutput_items; i++)
      {
        if(inbuf[i] > d_compact_threshold)
        {
          compact_items++;
        }
      }

	 int format = 0; // see .rtl format specification for description of the two formats
	 int format_0_byte_size = 8 + 4 + d_fft_size * 4;
	 int bit_length = 64 + 32 + compact_items * (32 + ceil(log2(d_fft_size)));
	 int padding_bits = (8 - (bit_length % 8))%8;
	 int format_1_byte_size = (bit_length + padding_bits) / 8 ;

	 if (format_0_byte_size < format_1_byte_size)
      {
	   *compact_size = format_0_byte_size;
	   format = 0;
	 } else {
	   *compact_size = format_1_byte_size;
	   format = 1;
	 }

	 size_t bitset_length = *compact_size * 8; // number of bits needed to store the values
	 // The compact bitset is allocated once and values are then injected
      boost::dynamic_bitset<uint8_t> compact_bitset(bitset_length);
	 int offset = 0;
	 
	 // vector number
	 boost::dynamic_bitset<> vector_no(sizeof(d_vector_no)*8, d_vector_no);
	 injectLoopDyn(compact_bitset, vector_no, offset); 
	 offset += vector_no.size();
	 d_vector_no++;
	 	 
	 if (format == 0)
	 {
	   // length and format
	   boost::dynamic_bitset<> length_format(32, d_fft_size << 1);
	   length_format[0] = format;
        injectLoopDyn(compact_bitset, length_format, offset); 
	   offset += length_format.size();
	   int j = 0;
        for(int i = 0; i < noutput_items; i++)
        {
		boost::dynamic_bitset<> value(sizeof(float)*CHAR_BIT, *const_cast<unsigned long*>(reinterpret_cast<const unsigned long*> (&inbuf[i]) ));
		injectLoopDyn(compact_bitset, value, offset);
	   	offset += value.size();	
        }
	 } else if (format == 1) {
	   // length and format
	   boost::dynamic_bitset<> length_format(32, compact_items << 1);
	   length_format[0] = format;
        injectLoopDyn(compact_bitset, length_format, offset); 
	   offset += length_format.size();
 	   int j = 0;
        for(int i = 0; i < noutput_items; i++)
        {
          if(inbuf[i] > d_compact_threshold)
          {
		  boost::dynamic_bitset<> bin_no(d_bin_no_bit_size, (unsigned long) i);
		  boost::dynamic_bitset<> value(sizeof(float)*CHAR_BIT, *const_cast<unsigned long*>(reinterpret_cast<const unsigned long*> (&inbuf[i]) ));
		  injectLoopDyn(compact_bitset, bin_no, offset); 
		  offset += bin_no.size();
		  injectLoopDyn(compact_bitset, value, offset);
	   	  offset += value.size();	
            j++;
          }
        }
	   boost::dynamic_bitset<> padding(padding_bits);
	   injectLoopDyn(compact_bitset, padding, offset);
	 }

      // Convert the bitset into a char (uint8_t) buffer
	 boost::shared_ptr<char[]> compact_buf( new char[*compact_size] );
	 typedef uint8_t Block;
	 std::vector<Block> bytes;
      boost::to_block_range(compact_bitset, std::back_inserter(bytes));
	 for(int i = 0; i <*compact_size;i++)
      {
	   compact_buf[i] = bytes[i];
      }
      return compact_buf;
    }

    void 
    compact_file_sink_impl::injectLoopDyn( boost::dynamic_bitset<uint8_t>& bs1,const boost::dynamic_bitset<>& bs2,int start)
    {
      for(size_t i=0;i<bs2.size();i++)
	 {
        bs1[i+start]=bs2[i];
	 }
    }

    // Author: https://stackoverflow.com/users/1029901/fericelli
    void 
    compact_file_sink_impl::injectLoopDyn( boost::dynamic_bitset<>& bs1,const boost::dynamic_bitset<>& bs2,int start)
    {
      for(size_t i=0;i<bs2.size();i++)
        bs1[i+start]=bs2[i];
    }


  } /* namespace compacter */
} /* namespace gr */

