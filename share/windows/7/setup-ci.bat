@ECHO OFF

:: %HOMEDRIVE% = C:
:: %HOMEPATH% = \Users\steinbac
:: %system32% ??
:: No spaces in paths
:: Program Files > ProgramFiles
:: cls = clear screen
:: CMD reads the system environment variables when it starts. To re-read those variables you need to restart CMD
:: Use console 2 http://sourceforge.net/projects/console/

SET "SW_ROOT=%HOMEDRIVE%%HOMEPATH%\software"
:: SETX SW_ROOT "%HOMEDRIVE%%HOMEPATH%\software"

:: utilities
SET "BENCH_ROOT=%SW_ROOT%\benchmark\1.4.1"
SET "ZIP_ROOT=%SW_ROOT%\7-Zip\17.01beta"
SET "CMAKE_ROOT=%SW_ROOT%\cmake\3.11.3"
SET "GRADLE_ROOT=%SW_ROOT%\gradle\4.1"
SET "CURL_ROOT=%SW_ROOT%\curl\7.50.1"
SET "TIFF_ROOT=%SW_ROOT%\tiff\4.0.6"
SET "HDF5_ROOT=%SW_ROOT%\hdf5\1.8.17"
:: SET "FFMPEG_ROOT=%SW_ROOT%\ffmpeg\3.0.2-static"

SET "LZ4_ROOT=%SW_ROOT%\lz4\1.8.2"

SET "BOOST_ROOT=%SW_ROOT%\boost\1_67_0"

SET "FFMPEG_ROOT=%SW_ROOT%\vcpkg\repo\packages\ffmpeg4sqy_x64-windows-static"
SET "X264_ROOT=%SW_ROOT%\x264\master-static"
SET "X265_ROOT=%SW_ROOT%\x265\2.0-static-crt"

SET "CMAKE_bin=%CMAKE_ROOT%\bin"
SET "GRADLE_bin=%GRADLE_ROOT%\bin"
SET "CURL_bin=%CURL_ROOT%"

SET "ZIP_bin=%ZIP_ROOT%"

SET "LLVM_BIN=%HOMEDRIVE%\Program Files\LLVM\bin"
SET "LLVM_LIB=%HOMEDRIVE%\Program Files\LLVM\lib"
SET "LLVM_INCLUDE=%HOMEDRIVE%\Program Files\LLVM\include"

:: libraries
SET "BENCH_INCLUDE=%BENCH_ROOT%\include"
SET "BENCH_LIB=%BENCH_ROOT%\lib"

SET "LZ4_BIN=%LZ4_ROOT%\bin"
SET "LZ4_LIB=%LZ4_ROOT%\lib"
SET "LZ4_INCLUDE=%LZ4_ROOT%\include"

SET "BOOST_LIB=%BOOST_ROOT%\lib64-msvc-14.1"
SET "BOOST_INCLUDE=%BOOST_ROOT%\include"

SET "TIFF_BIN=%TIFF_ROOT%\bin"
SET "TIFF_LIB=%TIFF_ROOT%\lib"
SET "TIFF_INCLUDE=%TIFF_ROOT%\include"

SET "HDF5_BIN=%HDF5_ROOT%\bin"
SET "HDF5_LIB=%HDF5_ROOT%\lib"
SET "HDF5_INCLUDE=%HDF5_ROOT%\include"

SET "FFMPEG_BIN=%FFMPEG_ROOT%\bin"
SET "FFMPEG_LIB=%FFMPEG_ROOT%\lib"
SET "FFMPEG_INCLUDE=%FFMPEG_ROOT%\include"

SET "X264_BIN=%X264_ROOT%\bin"
SET "X264_LIB=%X264_ROOT%\lib"
SET "X264_INCLUDE=%X264_ROOT%\include"

SET "X265_BIN=%X265_ROOT%\bin"
SET "X265_LIB=%X265_ROOT%\lib"
SET "X265_INCLUDE=%X265_ROOT%\include"

:: with FFMPEG
:: SET "Path variable
:: set "PATH=%ZIP_BIN%;%CMAKE_BIN%;%GRADLE_BIN%;%GIT_BIN%;%CURL_BIN%;%TIFF_BIN%;%HDF5_BIN%;%LZ4_BIN%;%FFMPEG_BIN%;%X264_BIN%;%X265_BIN%;%PATH%"
:: 
:: :: SET "BIN variable
:: set "LIB=%BOOST_LIB%;%TIFF_LIB%;%HDF5_LIB%;%FFMPEG_LIB%;%X264_LIB%;%X265_LIB%;%LZ4_LIB%;%LIB%"
:: 
:: :: SET "INCLUDE variable
:: set "INCLUDE=%BOOST_INCLUDE%;%TIFF_INCLUDE%;%HDF5_INCLUDE%;%FFMPEG_INCLUDE%;%X264_INCLUDE%;%X265_INCLUDE%;%LZ4_INCLUDE%;%INLUDE%"
:: 
:: :: SET "CMAKE_PREFIX_PATH variable to make find_package calls happy
:: set "CMAKE_PREFIX_PATH=%BOOST_ROOT%;%TIFF_ROOT%;%HDF5_ROOT%;%FFMPEG_ROOT%;%X264_ROOT%;%X265_ROOT%;%LZ4_ROOT%"

:: without FFMEPG
:: SET "Path variable
set "PATH=%ZIP_BIN%;%CMAKE_BIN%;%GRADLE_BIN%;%CURL_BIN%;%TIFF_BIN%;%HDF5_BIN%;%LZ4_BIN%;%BOOST_LIB%;%PATH%"

:: SET "BIN variable
set "LIB=%BOOST_LIB%;%TIFF_LIB%;%HDF5_LIB%;%LZ4_LIB%;%BENCH_LIB%;%LIB%"

:: SET "INCLUDE variable
set "INCLUDE=%BOOST_INCLUDE%;%TIFF_INCLUDE%;%HDF5_INCLUDE%;%LZ4_INCLUDE%;%BENCH_INCLUDE%;%INLUDE%"

:: SET "CMAKE_PREFIX_PATH variable to make find_package calls happy
set "CMAKE_PREFIX_PATH=%BOOST_ROOT%;%TIFF_ROOT%;%HDF5_ROOT%;%LZ4_ROOT%;%BENCH_ROOT%"


:: SET "Java variable, C:\Program Files\Java\jdk1.8.0_73
set "JAVA_HOME=%HOMEDRIVE%\Program Files\Java\jdk1.8.0_73"


:: PAUSE

