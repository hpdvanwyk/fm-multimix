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
 * This is the header file for a simple circular buffer. 
 */

struct circ_buf{
	int begin;
	int size;
	int count;
	uint8_t* buffer;
};

void circInit(struct circ_buf* buffer, int size);
void circFree(struct circ_buf* buffer, int size);
int circSpaceLeft(struct circ_buf* buffer);
int circCount(struct circ_buf* buffer);
void circWrite(struct circ_buf* buffer, uint8_t* elements, int len);
int circRead(struct circ_buf* buffer, uint8_t* out, int len);
int circPeek(struct circ_buf* buffer, uint8_t* out, int len);
void circReadAdvance(struct circ_buf* buffer, int len);
