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
     * \brief The compactor is a module created for the purpose of logging power spectrum measurement to a file
	* in a way that minimizes storage usage by exluding certain values
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
	 
	/*!
       * \brief Set the sample rate
       *
       * The sample rate is saved in the file for future use in processing. The sample rate 
 	  * is written to the file before the first measurement is recorded. Therefore, the
	  * changing the sample rate will only have effect, when it is done before the file is created.
	  * The sample rate is not directly utilized by the module, but for processing purposes, the 
	  * sample rate should reflect the actual sampling rate used in the system
       *
       * \param sample_rate the sample rate in Hz
       */
	 virtual void set_sample_rate(uint64_t sample_rate) = 0;//2000000;
	 /*!
       * \brief Returns the sample rate
       *
       * The sample rate 
       */
	 virtual uint64_t sample_rate() = 0;
	/*!
       * \brief Set the fft size
       *
       * The fft size is saved in the file for future use in processing and used for compacting.
	  * The format of the compacted file is dependent on the fft size, thus the value should only
	  * be changed before the file has been created.
       *
       * \param fft_size the fft size
       */
      virtual void set_fft_size(uint32_t fft_size) = 0;//1024;
	 virtual uint32_t fft_size() = 0;
	/*!
       * \brief Set the center frequency
       *
       * The center frequency is saved in the file for future use in processing. The center frequency 
 	  * is written to the file before the first measurement is recorded. Therefore, 
	  * changing the center frequency will only have effect, when it is done before the file is created.
	  * The center frequency is not directly utilized by the module, but for processing purposes, the 
	  * center frequency should reflect the actual center frequency used in the system
       *
       * \param center_freq the center frequency in Hz
       */
      virtual void set_center_freq(uint64_t center_freq) = 0;//868300000;
	 virtual uint64_t center_freq() = 0;
	/*!
       * \brief Set the filtering threshold
       *
       * The filtering threshold determines which power measurements should be stored. 
	  * Any values below the threshold are discarded. This is intended to allow disregarding
	  * power measurements that fall below the noise floor, and therefore bears no 
	  * relevant information. The value of the power being filtered is in dB, and dBm if the 
	  * device is calibrated.
       *
       * \param compact_threshold the filtering threshold in dB(m)
       */
	 virtual void set_compact_threshold(float compact_threshold) = 0;//-63;
	 virtual float compact_threshold() = 0;
	/*!
       * \brief Disallow buffereing of the value to file
       *
       * Since writing to a file has interaction the the parmanent storage on the device.
	  * The operating system will by default buffer data for effecient I/O operations
	  * Setting unbuffered, enforces the flushing of the file write buffer, after each 
	  * input cycle.
	  *
       * \param center_freq the center frequency in Hz
       */
	 virtual void set_unbuffered(bool unbuffered) = 0;
    };

  } // namespace compacter
} // namespace gr

#endif /* INCLUDED_COMPACTER_COMPACT_FILE_SINK_H */

