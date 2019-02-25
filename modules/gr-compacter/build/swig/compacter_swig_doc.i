
/*
 * This file was automatically generated using swig_doc.py.
 *
 * Any changes to it will be lost next time it is regenerated.
 */




%feature("docstring") gr::compacter::compact_file_sink "The compactor is a module created for the purpose of logging power spectrum measurement to a file in a way that minimizes storage usage by exluding certain values.

Constructor Specific Documentation:

Return a shared_ptr to a new instance of compacter::compact_file_sink.

To avoid accidental use of raw pointers, compacter::compact_file_sink's constructor is in a private implementation class. compacter::compact_file_sink::make is the public interface for creating new instances.

Args:
    filename : name of the file to open and write output to.
    append : if true, data is appended to the file instead of overwriting the initial content."

%feature("docstring") gr::compacter::compact_file_sink::set_sample_rate "Set the sample rate.

The sample rate is saved in the file for future use in processing. The sample rate is written to the file before the first measurement is recorded. Therefore, the changing the sample rate will only have effect, when it is done before the file is created. The sample rate is not directly utilized by the module, but for processing purposes, the sample rate should reflect the actual sampling rate used in the system"

%feature("docstring") gr::compacter::compact_file_sink::sample_rate "Returns the sample rate.

The sample rate"

%feature("docstring") gr::compacter::compact_file_sink::set_fft_size "Set the fft size.

The fft size is saved in the file for future use in processing and used for compacting. The format of the compacted file is dependent on the fft size, thus the value should only be changed before the file has been created."



%feature("docstring") gr::compacter::compact_file_sink::set_center_freq "Set the center frequency.

The center frequency is saved in the file for future use in processing. The center frequency is written to the file before the first measurement is recorded. Therefore, changing the center frequency will only have effect, when it is done before the file is created. The center frequency is not directly utilized by the module, but for processing purposes, the center frequency should reflect the actual center frequency used in the system"



%feature("docstring") gr::compacter::compact_file_sink::set_compact_threshold "Set the filtering threshold.

The filtering threshold determines which power measurements should be stored. Any values below the threshold are discarded. This is intended to allow disregarding power measurements that fall below the noise floor, and therefore bears no relevant information. The value of the power being filtered is in dB, and dBm if the device is calibrated."



%feature("docstring") gr::compacter::compact_file_sink::set_unbuffered "Disallow buffereing of the value to file.

Since writing to a file has interaction the the parmanent storage on the device. The operating system will by default buffer data for effecient I/O operations Setting unbuffered, enforces the flushing of the file write buffer, after each input cycle."

%feature("docstring") gr::compacter::compact_file_sink::make "The compactor is a module created for the purpose of logging power spectrum measurement to a file in a way that minimizes storage usage by exluding certain values.

Constructor Specific Documentation:

Return a shared_ptr to a new instance of compacter::compact_file_sink.

To avoid accidental use of raw pointers, compacter::compact_file_sink's constructor is in a private implementation class. compacter::compact_file_sink::make is the public interface for creating new instances.

Args:
    filename : name of the file to open and write output to.
    append : if true, data is appended to the file instead of overwriting the initial content."