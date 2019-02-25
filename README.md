# grc
Custom modules to gnuradio and gnuradio companion and flow graphs

The project is focused on creating a spectrumanalyzer from the RTL-SDR dongle, which can be deployed and collect data on the present interference on a specified frequency

To build and install the module go to

cd modules/gr-compacter
mkdir build
cd build
cmake ../ -DINSTALl_UDEV_RULES=ON 
make
sudo make install
