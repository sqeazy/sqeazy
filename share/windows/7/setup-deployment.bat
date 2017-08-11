@ECHO OFF

:: %HOMEDRIVE% = C:
:: %HOMEPATH% = \Users\steinbac
:: %system32% ??
:: No spaces in paths
:: Program Files > ProgramFiles
:: cls = clear screen
:: CMD reads the system environment variables when it starts. To re-read those variables you need to restart CMD
:: Use console 2 http://sourceforge.net/projects/console/

SET SW_ROOT="%HOMEDRIVE%%HOMEPATH%\software"

:: utilities
SET CMAKE_BIN="%SW_ROOT%\cmake\3.9.1\bin"
SET GRADLE_BIN="%SW_ROOT%\gradle\3.0\bin"
SET GIT_BIN="%SW_ROOT%\gradle\3.0\bin"
SET CURL_BIN="%SW_ROOT%\curl\7.50.1"
SET TIFF_BIN="%SW_ROOT%\tiff\4.0.6\bin"
SET HDF5_BIN="%SW_ROOT%\hdf5\1.8.17-static\bin"
SET FFMPEG_BIN="%SW_ROOT%\ffmpeg\3.0.2-static\bin"

SET LLVM_BIN="%HOMEDRIVE%\Program Files\LLVM\bin"
SET LLVM_LIB="%HOMEDRIVE%\Program Files\LLVM\lib"
SET LLVM_INCLUDE="%HOMEDRIVE%\Program Files\LLVM\include"

:: libraries
SET BOOST_LIB="%SW_ROOT%\boost\1_59_0_static\lib"
SET BOOST_INCLUDE="%SW_ROOT%\boost\1_59_0_static\include"

SET TIFF_LIB="%SW_ROOT%\tiff\4.0.6\lib"
SET TIFF_INCLUDE="%SW_ROOT%\tiff\4.0.6\include"

SET HDF5_LIB="%SW_ROOT%\hdf5\1.8.17-static\lib"
SET HDF5_INCLUDE="%SW_ROOT%\hdf5\1.8.17-static\include"

SET FFMPEG_LIB="%SW_ROOT%\ffmpeg\3.0.2-static\lib"
SET FFMPEG_INCLUDE="%SW_ROOT%\ffmpeg\3.0.2-static\include"

SET X264_LIB="%SW_ROOT%\x264\master-static\lib"
SET X264_INCLUDE="%SW_ROOT%\x264\master-static\include"

SET X265_LIB="%SW_ROOT%\x265\2.0-static-crt\lib"
SET X265_INCLUDE="%SW_ROOT%\x265\2.0-static-crt\include"

:: Set Path variable
setx PATH "%CMAKE_BIN%;%GRADLE_BIN%;%GIT_BIN%;%CURL_BIN%;%LLVM_BIN%;%TIFF_BIN%;%HDF5_BIN%;%FFMPEG_BIN%"

:: Set LIB variable
setx LIB "%BOOST_LIB%;%TIFF_LIB%;%HDF5_LIB%;%FFMPEG_LIB%;%X264_LIB%;%X265_LIB%;%LLVM_LIB%"

:: Set INCLUDE variable
setx INCLUDE "%BOOST_INCLUDE%;%TIFF_INCLUDE%;%HDF5_INCLUDE%;%FFMPEG_INCLUDE%;%X264_INCLUDE%;%X265_INCLUDE%;%LLVM_INCLUDE%"

:: Set Java variable, C:\Program Files\Java\jdk1.8.0_73
setx JAVA_HOME "%HOMEDRIVE%\Program Files\Java\jdk1.8.0_73"

:: PAUSE