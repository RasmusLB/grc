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


#ifndef INCLUDED_COMPACTER_COMPACT_FILE_SINK_H
#define INCLUDED_COMPACTER_COMPACT_FILE_SINK_H

#include <compacter/api.h>
#include <gnuradio/blocks/api.h>
#include <gnuradio/blocks/file_sink_base.h>
#include <gnuradio/sync_block.h>


namespace gr {
  namespace compacter {
    

    /*!
     * \brief <+description of block+>
     * \ingroup compacter
     *
     */
    class COMPACTER_API compact_file_sink : virtual public sync_block,
                                            virtual public blocks::file_sink_base
    {
     public:
      typedef boost::shared_ptr<compact_file_sink> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of compacter::compact_file_sink.
       *
       * To avoid accidental use of raw pointers, compacter::compact_file_sink's
       * constructor is in a private implementation
       * class. compacter::compact_file_sink::make is the public interface for
       * creating new instances.
       *
       * \param filename name of the file to open and write output to.
       * \param append if true, data is appended to the file instead of
       *        overwriting the initial content.
       */
      static sptr make(size_t itemsize,const char *filename, bool append=false);
	 
	 virtual void set_sample_rate(uint64_t sample_rate) = 0;//2000000;
	 virtual uint64_t sample_rate() = 0;
      virtual void set_fft_size(uint32_t fft_size) = 0;//1024;
	 virtual uint32_t fft_size() = 0;
      virtual void set_center_freq(uint64_t center_freq) = 0;//868300000;
	 virtual uint64_t center_freq() = 0;
	 virtual void set_compact_threshold(float compact_threshold) = 0;//-63;
	 virtual float compact_threshold() = 0;
	 virtual void set_unbuffered(bool unbuffered) = 0;
    };

  } // namespace compacter
} // namespace gr

#endif /* INCLUDED_COMPACTER_COMPACT_FILE_SINK_H */

