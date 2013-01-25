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
 * This is a wrapper of FFTW that attempts to identify NBFM transmission by matching
 * a fft of the spectrum to the shape of a NBFM transmission that has been calculated
 * previously. fft_conv_gen.c can be used to recalculate these values.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <unistd.h>
#include "fft.h"


double convol [11]= {0.302196,0.495343,0.662907,0.818410,0.949743,1.003225,0.943759,0.817320,0.664393,0.491787,0.295309};

fft_obj* fft_init()
{
	int i;

	fft_obj* obj = malloc(sizeof(fft_obj));

	obj->in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FFT_LEN);
	obj->out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * FFT_LEN);
	obj->p = fftw_plan_dft_1d(FFT_LEN, obj->in, obj->out, FFTW_FORWARD, FFTW_ESTIMATE);

	for ( i=0; i<FFT_LEN ; i++)
	{
		obj->fftavg[i]=0;
		obj->band_power[i]=0;
		obj->result[i]=0;
		obj->window[i]=0.35875-(0.48829*cos((2*M_PI*i)/(FFT_LEN-1)))
			+(0.14128*cos((4*M_PI*i)/(FFT_LEN-1)))
			+(0.01168*cos((6*M_PI*i)/(FFT_LEN-1)))
		;
	}

	obj->avg=0;
	obj->avg_calc=0;
	obj->fftavg_counter=FFT_AVG;

	return obj;
}

void fft_free(fft_obj* obj)
{
	fftw_destroy_plan(obj->p);
	fftw_free(obj->in); fftw_free(obj->out);
	free (obj);
}

double calc_band_power(double* fft)
{
	int i;
	double tot=0;
	for(i=0; i<BANDWIDTH_BINS; i++)
	{
		tot +=( *(fft+(i-(BANDWIDTH_BINS-1)/2)) /FFT_AVG)*convol[i];
	}
	return tot/7;
}

int do_fft(fft_obj* obj, uint8_t* buff, int len)
{
	int i,j;
	int retval =0;
	if(len< FFT_LEN/2)
	{
		fprintf(stderr, "warning: for now fft needs to be called with 2*FFT_LEN samples\n");
	}
	for( i=0,j=0; i<len; i+=2,j++ )
	{
		obj->in[j]= (((float)buff[i])-127.5)*obj->window[j]+ I*(((float)buff[i+1])-127.5)*obj->window[j];
		obj->in[j] *= pow(-1,j);
	}

	fftw_execute(obj->p);

	for ( i=0; i<FFT_LEN ; i++)
	{
		obj->fftavg [i]+= cabs(obj->out[i]);
	}

	if(obj->fftavg_counter!=0)
	{
		obj->fftavg_counter--;
	}
	else
	{
		obj->fftavg_counter=FFT_AVG;

		for ( i=FFT_LEN/100; i<FFT_LEN*0.99 ; i++)
		{
			{
				obj->band_power[i] = calc_band_power(&(obj->fftavg[i]));
			}
		}

		obj->avg_calc=0;
		for ( i=0; i<FFT_LEN ; i++)
		{
			obj->avg_calc += obj->band_power[i];	
		}
		obj->avg = obj->avg_calc / FFT_LEN;

		for ( i=0; i<FFT_LEN ; i++)
		{
			if(obj->band_power[i] > obj->avg*DETECTION_LEVEL)
			{
				int largest =1;
				for(j=0; j<BANDWIDTH_BINS ; j++)
				{
					if(!( obj->band_power[i] >= obj->band_power[i+(j-(int)(BANDWIDTH_BINS/2))]) )
					{
						largest=0;
						break;
					}
				}
				if(largest)
				{
					obj->result[i]=obj->band_power[i]/obj->avg;
					retval++;
				}
				else
				{
					obj->result[i]=0;
				}
			}
			else
			{
					obj->result[i]=0;
			}
		}

		for ( i=0; i<FFT_LEN ; i++)
		{
			obj->fftavg[i]=0;
			obj->band_power[i]=0;
		}
	}

	return retval;
}

double* get_fft_results(fft_obj* obj)
{
	return obj->result;
}

