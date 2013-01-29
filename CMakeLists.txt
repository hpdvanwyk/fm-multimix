#    This file is part of fm-multimix, A multiple channel fm downmixer.
#    Copyright (C) 2013  Hendrik van Wyk
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>. 

cmake_minimum_required (VERSION 2.6)
set(CMAKE_C_FLAGS "-std=gnu99 -O3")
project (multi_downmix)

find_library(FFTW_LIBRARY
     NAMES fftw3 fftw)

add_library (fft_lib fft.c)
add_library (sinegen sinegen.c)
add_library (demod_proc demod_proc.c)
add_library (circ_buffer circ_buffer.c)

#add_executable (fft test_fft.c)
#add_executable (fft_conv fft_conv_gen.c)
#add_executable (test_sine test_sine.c)
add_executable (fm_multimix fm_multimix.c)

target_link_libraries(fft_lib fftw3)
target_link_libraries(sinegen m)

#target_link_libraries(fft fft_lib)
#target_link_libraries(fft_conv fftw3)
#target_link_libraries(test_sine sinegen)
target_link_libraries(demod_proc sinegen)
target_link_libraries(fm_multimix fft_lib sinegen demod_proc circ_buffer)
