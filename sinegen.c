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
 * This is a sine wave generator. The sine wave is used 
 * to perform mixing of the input signal. 
 */

#include <stdio.h>
#include "sinegen.h"

sine_gen* sine_set_up(float frac , float phase,  float amplitude)
{
	sine_gen* obj = malloc(sizeof(sine_gen));
	obj->k =2*cos(2*M_PI*frac);
	
	obj->y_2 =-amplitude * sin((phase*M_PI/180 + 2*M_PI*frac)); //y[-1]
	obj->y_1 =-amplitude * sin (phase*M_PI/180); //y[0]
	return obj;
}

