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
    compact_file_sink::make(const char *filename, bool append)
    {
      return gnuradio::get_initial_sptr
        (new compact_file_sink_impl(filename, append));
    }

    /*
     * The private constructor
     */
    compact_file_sink_impl::compact_file_sink_impl(const char *filename, bool append)
      : gr::sync_block("compact_file_sink",
              gr::io_signature::make(1, 1, sizeof(float)),
              gr::io_signature::make(0, 0, 0)),
        file_sink_base(filename, true, append)
    {
      // Set variables
      d_time_stamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();   
      set_sample_rate(2000000);
      set_fft_size(1024);
      set_center_freq(868300000);
	 set_compact_threshold(-63);

	 
      do_update();

      // Write meta data to file
      
      int count = fwrite(&d_time_stamp, sizeof(d_time_stamp), 1, d_fp) * sizeof(d_time_stamp) ;      
      count += fwrite(&d_sample_rate, sizeof(d_sample_rate), 1, d_fp) * sizeof(d_sample_rate);
      count += fwrite(&d_fft_size, sizeof(d_fft_size), 1, d_fp) * sizeof(d_fft_size);
      count += fwrite(&d_center_freq, sizeof(d_center_freq), 1, d_fp) * sizeof(d_center_freq);
//	 std::cout << "wrote " << count << " bytes of meta data" << std::endl;
//std::bitset<32> x(d_fft_size);
//std::cout << "Encoding of fft_size: " << x << std::endl;

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

    /*
     * Actual work carried out here
     */
    int
    compact_file_sink_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const float *inbuf = (const float *) input_items[0];
      int  nwritten = 0;

	 // Write actual data to file
      // Create one long byte array, and write it
 	 int compact_size = 0;
      boost::shared_ptr<char[]> compact_buf = compact(inbuf, &compact_size, noutput_items);

	std::cout << "Writing following buffer to file: ";
	for (int i = 0; i < compact_size; i++){
	  std::bitset<8> x(compact_buf[i]);
	  std::cout << "> " << x ;
	}
	std::cout << std::endl;
	do_update();
	
	 char test[23];
for (int i = 0; i < 23; i++){
	  test[i] = compact_buf[i];
	}
	 int count = fwrite(test, compact_size, 1, d_fp);
      //int count = fwrite(&compact_buf, compact_size, 1, d_fp);
//      std::cout << "wrote " << count * compact_size << " bytes of compact data" << std::endl;


      // Tell runtime system how many output items we produced.
      return noutput_items;
    }


    boost::shared_ptr<char[]>
    compact_file_sink_impl::compact(const float *inbuf, int *compact_size, int noutput_items)
    {
      int compact_items = 0;
	 for(int i = 0; i < noutput_items; i++)
      {
        if(inbuf[i] > d_compact_threshold)
        {
          compact_items++;
        }
      }
//	 std::cout << compact_items << " out of " << noutput_items << " values were kept" << std::endl;

	 int format = 0;
	 if (compact_items > 50)
      {
	   *compact_size = d_fft_size * 4;
	   format = 0;
	 } else {
        int bit_length = 64 + 32 + compact_items * (32 + log2(d_fft_size));
        int padding = 8 - (bit_length % 8);
	   *compact_size = (bit_length + padding) / 8 ;
//	   std::cout << "Byte length: " << *compact_size << std::endl;
	   format = 1;
	 }

	 size_t bit_length = *compact_size * 8;
      boost::dynamic_bitset<uint8_t> compact_bitset(bit_length);
	 int offset = 0;
	 
	 // vector number
	 boost::dynamic_bitset<> vector_no(sizeof(d_vector_no)*8, d_vector_no);
	 injectLoopDyn(compact_bitset, vector_no, offset); 
	 offset += vector_no.size();
	 d_vector_no++;
	 

	 /*  debug prints  */
	 boost::dynamic_bitset<> ones(8, 255);
 	 injectLoopDyn(vector_no, ones, 0);
	 std::cout << "vector_no:  ";
 	 //vector_no[0] = 1;
      for (boost::dynamic_bitset<>::size_type i = 0; i < compact_bitset.size(); ++i)
        std::cout << vector_no[i];
	 std::cout << std::endl;

	 // length and format
	 boost::dynamic_bitset<> length_format(32, *compact_size << 1);
	 length_format[length_format.size()-1] = format;
std::cout << "length_format[]: " <<  length_format[length_format.size()-1] << std::endl;

std::cout << "length_format: ";
for(int i = 0; i <32;i++)
      {
	   std::cout <<  length_format[i];
      }
std::cout << std::endl;

      injectLoopDyn(compact_bitset, length_format, offset); 
	 offset += length_format.size();

	 const int pack_size = log2(d_fft_size) + 32;
	 const int unused = 8 - (pack_size % 8);

	 
      boost::shared_ptr<char[]> compact_buf( new char[*compact_size] );
      int j = 0;
      for(int i = 0; i < noutput_items; i++)
      {
        if(inbuf[i] > d_compact_threshold)
        {
 		//std::cout << "bin_no " << i << " value " << inbuf[i] << std::endl;
		boost::dynamic_bitset<> bin_no(d_bin_no_bit_size, (unsigned long) i);
		boost::dynamic_bitset<> value(sizeof(float)*CHAR_BIT, *const_cast<unsigned long*>(reinterpret_cast<const unsigned long*> (&inbuf[i]) ));
          
		injectLoopDyn(compact_bitset, bin_no, offset); 
		offset += bin_no.size();
		injectLoopDyn(compact_bitset, value, offset);
	   	offset += bin_no.size();

		
          j++;
        }
      }

	 boost::dynamic_bitset<> padding(unused);
	 injectLoopDyn(compact_bitset, padding, offset);

std::cout << "bitset before converting to char buffer: ";
	 for (boost::dynamic_bitset<>::size_type i = 0; i < compact_bitset.size(); ++i){
        std::cout << compact_bitset[i];}
	 std::cout << std::endl;

      // Now we just need to convert the bitset into a nice char buffer
 	 
	 //const int bytes_size = *compact_size;
	 typedef uint8_t Block;
	 std::vector<Block> bytes;
	 //boost::dynamic_bitset<uint8_t> transfer(compact_bitset.size());
	 //transfer = compact_bitset;
	 //bytes.resize(bytes_size);
	 std::cout << "bytes.size() " << bytes.size() << "): " << std::endl;
	 std::cout << "compact_size " << *compact_size << "): " << std::endl;
      boost::to_block_range(compact_bitset, std::back_inserter(bytes));
      std::cout << "char buffer (size = " << bytes.size() << "): " ;
	 for(int i = 0; i <*compact_size;i++)
      {
	   compact_buf[i] = bytes[i];
      }

std::cout << "Transferring following buffer: ";
	for (int i = 0; i < *compact_size; i++){
	  std::bitset<8> x(compact_buf[i]);
	  std::cout << "> " << x ;
	}
	std::cout << std::endl;

	 std::cout << std::endl;
      return compact_buf;
    }

    void 
    compact_file_sink_impl::injectLoopDyn( boost::dynamic_bitset<uint8_t>& bs1,const boost::dynamic_bitset<>& bs2,int start)
    {
      for(size_t i=0;i<bs2.size();i++)
        bs1[i+start]=bs2[i];
    }


    void 
    compact_file_sink_impl::injectLoopDyn( boost::dynamic_bitset<>& bs1,const boost::dynamic_bitset<>& bs2,int start)
    {
      for(size_t i=0;i<bs2.size();i++)
        bs1[i+start]=bs2[i];
    }

    // Should not be used
    boost::dynamic_bitset<> 
    compact_file_sink_impl::concatLoopDyn( const boost::dynamic_bitset<>& bs1,const boost::dynamic_bitset<>& bs2)
    {
	boost::dynamic_bitset<> res(bs1);
	res.resize(bs1.size()+bs2.size());
	    size_t bs1Size=bs1.size();
	    size_t bs2Size=bs2.size();

	    for(size_t i=0;i<bs2.size();i++)
		   res[i+bs1Size]=bs2[i];
	    return res;
    }

  } /* namespace compacter */
} /* namespace gr */

