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

#ifndef INCLUDED_COMPACTER_COMPACT_FILE_SINK_IMPL_H
#define INCLUDED_COMPACTER_COMPACT_FILE_SINK_IMPL_H

#include <compacter/compact_file_sink.h>
#include <chrono>
#include <boost/dynamic_bitset.hpp>

namespace gr {
  namespace compacter {

    class compact_file_sink_impl : public compact_file_sink
    {
     private:
      double d_time_stamp = 13;    
      uint64_t d_sample_rate = 2000000;
      uint32_t d_fft_size = 1024;
      uint64_t d_center_freq = 868300000;
	 float d_compact_threshold = -63;
	 uint64_t d_vector_no = 0;
	 uint32_t d_bin_no_bit_size = 0;

      boost::shared_ptr<char[]> compact(const float *inbuf, int *compact_size, int noutput_items);
	 void injectLoopDyn( boost::dynamic_bitset<uint8_t>& bs1,const boost::dynamic_bitset<>& bs2,int start);
	 void injectLoopDyn( boost::dynamic_bitset<>& bs1,const boost::dynamic_bitset<>& bs2,int start);

     public:
      compact_file_sink_impl(const char *filename, bool append=false);
      ~compact_file_sink_impl();

	 // Setters and getters
	 void set_sample_rate(uint64_t sample_rate);
	 uint64_t sample_rate();
      void set_fft_size(uint32_t fft_size);
	 uint32_t fft_size();
      void set_center_freq(uint64_t center_freq);
	 uint64_t center_freq();
	 void set_compact_threshold(float compact_threshold);
	 float compact_threshold();
	 void set_unbuffered(bool unbuffered);

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace compacter
} // namespace gr

#endif /* INCLUDED_COMPACTER_COMPACT_FILE_SINK_IMPL_H */

