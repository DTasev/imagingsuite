cp  ~/git/lib/libkipl.1.0.0.dylib build/ImagingAlgorithms/libkipl.1.0.0.dylib
cp ~/git/imagingsuite/external/mac/lib/libNeXus.1.0.0.dylib build/ImagingAlgorithms/libNexus.1.0.0.dylib
cp ~/git/imagingsuite/external/mac/lib/libNeXusCPP.1.0.0.dylib build/ImagingAlgorithms/libNexusCPP.1.0.0.dylib
cp ~/git/imagingsuite/external/mac/lib/libhdf5_cpp.11.dylib build/ImagingAlgorithms/libhdf5_cpp.11.dylib
cp ~/git/imagingsuite/external/mac/lib/libhdf5.10.dylib build/ImagingAlgorithms/libhdf5.10.dylib
cp ~/git/imagingsuite/external/mac/lib/libsz.2.dylib build/ImagingAlgorithms/libsz.2.dylib
cp ~/git/imagingsuite/external/mac/lib/libhdf5_hl.10.dylib build/ImagingAlgorithms/libhdf5_hl.10.dylib

cd build/ImagingAlgorithms
ln -s libkipl.1.0.0.dylib libkipl.1.dylib
ln -s libNexus.1.0.0.dylib libNexus.1.dylib
ln -s libNexusCPP.1.0.0.dylib libNexusCPP.1.dylib
rm test*.py
ln -s ../../UnitTests/python/test*.py .
#nexus_related

# install_name_tool -change libkipl.1.dylib libkipl.1.dylib imagalg.cpython-37m-darwin.so
# install_name_tool -change libNeXus.1.dylib libNeXus.1.dylib libkipl.1.0.0.dylib
# install_name_tool -change libNeXusCPP.1.dylib libNeXusCPP.1.dylib libkipl.1.0.0.dylib
# install_name_tool -change /usr/lib/libz.1.dylib libz.1.dylib libNexus.1.dylib
# install_name_tool -change /usr/lib/libz.1.dylib libz.1.dylib libNexusCPP.1.dylib
# install_name_tool -change libNeXus.1.dylib libNeXus.1.dylib libNexusCPP.1.dylib

install_name_tool -change /usr/local/Cellar/hdf5/1.8.16_1/lib/libhdf5.10.dylib libhdf5.10.dylib libhdf5_cpp.11.dylib
install_name_tool -change /usr/local/Cellar/hdf5/1.8.16_1/lib/libhdf5.10.dylib libhdf5.10.dylib libhdf5_hl.10.dylib

install_name_tool -change  /usr/local/opt/hdf5/lib/libhdf5_cpp.11.dylib libhdf5_cpp.11.dylib libNeXus.1.0.0.dylib
install_name_tool -change  /usr/local/opt/hdf5/lib/libhdf5.10.dylib libhdf5.10.dylib libNeXus.1.0.0.dylib
install_name_tool -change  /usr/local/opt/hdf5/lib/libhdf5_hl.10.dylib libhdf5_hl.10.dylib libNeXus.1.0.0.dylib
install_name_tool -change  /usr/local/opt/szip/lib/libsz.2.dylib libsz.2.dylib libNeXus.1.0.0.dylib

install_name_tool -change  /usr/local/opt/hdf5/lib/libhdf5_cpp.11.dylib libhdf5_cpp.11.dylib libNeXusCPP.1.0.0.dylib
install_name_tool -change  /usr/local/opt/hdf5/lib/libhdf5.10.dylib libhdf5.10.dylib libNeXusCPP.1.0.0.dylib
install_name_tool -change  /usr/local/opt/hdf5/lib/libhdf5_hl.10.dylib libhdf5_hl.10.dylib libNeXusCPP.1.0.0.dylib
install_name_tool -change  /usr/local/opt/szip/lib/libsz.2.dylib libsz.2.dylib libNeXusCPP.1.0.0.dylib

install_name_tool -change /usr/local/opt/hdf5/lib/libhdf5.10.dylib libhdf5.10.dylib libhdf5.10.dylib
install_name_tool -change /usr/local/opt/szip/lib/libsz.2.dylib libsz.2.dylib libhdf5.10.dylib
install_name_tool -change /usr/lib/libz.1.dylib libz.1.dylib libhdf5.10.dylib

install_name_tool -change /usr/local/opt/hdf5/lib/libhdf5_cpp.11.dylib libhdf5_cpp.11.dylib libhdf5_cpp.11.dylib
install_name_tool -change /usr/local/opt/szip/lib/libsz.2.dylib libsz.2.dylib libhdf5_cpp.11.dylib
install_name_tool -change /usr/lib/libz.1.dylib libz.1.dylib libhdf5_cpp.11.dylib

install_name_tool -change /usr/local/opt/hdf5/lib/libhdf5_hl.10.dylib  libhdf5_hl.10.dylib libhdf5_hl.10.dylib
install_name_tool -change /usr/local/opt/szip/lib/libsz.2.dylib libsz.2.dylib libhdf5_hl.10.dylib
install_name_tool -change /usr/lib/libz.1.dylib libz.1.dylib libhdf5_hl.10.dylib

cd ../..