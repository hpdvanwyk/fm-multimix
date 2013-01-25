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
 * This is the header file for a sine wave generator. The sine wave is used 
 * to perform mixing of the input signal. 
 */
#ifndef SINEGEN_H
#define SINEGEN_H

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <complex.h>

typedef struct sine_g
{
	float y_2; //y[n-2]
	float y_1; //y[n-1]
	float k;
}
sine_gen;

sine_gen* sine_set_up(float frac , float phase,  float amplitude);
static inline float sine_next_val(sine_gen* obj)
{
	float yn = (obj->k * obj->y_1 - obj->y_2);
	obj->y_2 = obj->y_1;
	obj->y_1 = yn;
	return yn;
}

#endif
