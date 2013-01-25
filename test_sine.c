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
 * This is a testing program for sinegen.c
 * This is not part of the final program
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <unistd.h>
#include "sinegen.h"


int main(int argc, char *argv[])
{
	sine_gen* real;
	sine_gen* imag;
	float out [2048];
	int i;
	
	int bin =512;

	real = sine_set_up((double)(abs(bin-(1024/2)))/1024 , 0,  1);
	if(bin-(1024/2) < 0)
	{
		imag = sine_set_up((double)(abs(bin-(1024/2)))/1024 , -90,  1);
	}
	else
	{
		imag = sine_set_up((double)(abs(bin-(1024/2)))/1024 , 90,  1);
	}
	while(1)
	{
		for( i=0; i<2048; i+=2)
		{
			out[i]=sine_next_val(real);
			out[i+1]=sine_next_val(imag);
		}
			fwrite(out, sizeof(float), 2048, stdout);
	}
	return 0;
}

