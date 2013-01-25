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
 * This is a testing program for fft.c
 * This is not part of the final program
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "fft.h"

int main(int argc, char *argv[])
{
	int i;
	int readcount;
	int skip=0;
	uint8_t sample[2*FFT_LEN];
	fft_obj* chanfinder_obj = fft_init();
	int chans_found;
	double* results;

	readcount = fread(sample, sizeof(uint8_t), 2*FFT_LEN, stdin);	

	while (readcount > 0)
	{
		if(skip == 0)
		{
			chans_found = do_fft(chanfinder_obj, sample, readcount);

			if(chans_found)
			{
				fprintf(stderr, "found %d possible transmissions \n", chans_found);
				results= get_fft_results(chanfinder_obj);
				for(i=0; i<FFT_LEN; i++)
				{
					if(results[i] > 0)
					{
						fprintf(stderr,"something at %d with %f\n",
								(i-FFT_LEN/2)*(SAMP_RATE/FFT_LEN), results[i]);
					}
				}
			}
			skip = 50;
		}
		else
		{
			skip--;
		}
		readcount = fread(sample, sizeof(uint8_t), 2*FFT_LEN, stdin);	
	}

	fft_free (chanfinder_obj);
	return 0;
}
