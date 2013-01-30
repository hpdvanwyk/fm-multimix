/*
    This file is part of fm-multimix, A multiple channel fm downmixer.
    Copyright (C) 2013  Hendrik van Wyk

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 */

/*
 * This is the header file for a wrapper for FFTW that attempts to identify
 * transmitting channels.
 */
#ifndef FFT_CHANFINDER_H
#define FFT_CHANFINDER_H

#include <fftw3.h>
#include <stdint.h>

#define FFT_LEN 2048
#define FFT_AVG 2
#define DETECTION_LEVEL 2
#define SAMP_RATE 1014300
#define BANDWIDTH 12500
#define BANDWIDTH_BINS (int)((double)BANDWIDTH/((double)SAMP_RATE/(double)FFT_LEN))


typedef struct fft_s
{
	double avg;
	double avg_calc;
	double fftavg [FFT_LEN];
	double band_power [FFT_LEN];
	double result [FFT_LEN];
	double window [FFT_LEN];
	int fftavg_counter;
	float threshold;

	fftw_complex *in;
	fftw_complex *out;
	fftw_plan p;
}
fft_obj;

fft_obj* fft_init(float threshold);

void fft_free(fft_obj* obj);

int do_fft(fft_obj* obj, uint8_t* buff, int len);

double* get_fft_results(fft_obj* obj);

#endif
