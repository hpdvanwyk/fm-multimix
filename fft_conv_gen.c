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
 * This program is used to generate the shape that is used  in fft.c to 
 * attemp to identify NBFM transmissions. It requires a transmitting NBFM
 * signal as input. This program is not used in the final executable but
 * is left here in case the shape needs to be recalculated.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <unistd.h>
#include <fftw3.h>

#define FFT_LEN 1024
#define FFT_AVG 4096
#define DETECTION_LEVEL 6
#define SAMP_RATE 1102500

#define BANDWIDTH 12500
#define BANDWIDTH_BINS (double)BANDWIDTH/((double)SAMP_RATE/(double)FFT_LEN)


int main(int argc, char *argv[])
{
	int readcount;
	int i,j;
	int skip=0;
	int16_t sample[2*FFT_LEN];
	double fftavg [FFT_LEN];
	int fftavg_counter=FFT_AVG;

	fftw_complex *in, *out;
	fftw_plan p;
	in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FFT_LEN);
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FFT_LEN);
	p = fftw_plan_dft_1d(FFT_LEN, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

	readcount = fread(sample, sizeof(int16_t), 2*FFT_LEN, stdin);	

	while (readcount > 0)
	{
		if(skip == 0)
		{
			if(readcount< FFT_LEN/2)
			{
				fprintf(stderr, "too little data read for fft\n");
			}
			for( i=0,j=0; i<readcount; i+=2,j++ )
			{
				in[j]= (float)sample[i] + I*(float)sample[i+1];
				in[j] *= pow(-1,j);
			}

			fftw_execute(p); 

			for ( i=0; i<FFT_LEN ; i++)
			{
				//avg_calc += cabs(out[i]);	
				fftavg [i]+= cabs(out[i]);
			}
			
			if(fftavg_counter!=0)
			{
				fftavg_counter--;
			}
			else
			{
				fftavg_counter=FFT_AVG;
			
				//fprintf(stderr, "avg: %f \n", avg);
				for ( i=0; i<FFT_LEN ; i++)
				{
					if(i>FFT_LEN/2-BANDWIDTH_BINS/2 && i<FFT_LEN/2+BANDWIDTH_BINS/2)
					{
						fprintf(stderr, "%f,", (fftavg[i]/FFT_AVG)/1200000);
					}
					fftavg[i]=0;
				}
				fprintf(stderr,"\n");
			}


			skip = 0;
		}
		else
		{
			skip--;
		}


		readcount = fread(sample, sizeof(int16_t), 2*FFT_LEN, stdin);	
	}

	fftw_destroy_plan(p);
	fftw_free(in); fftw_free(out);
	return 0;
}
