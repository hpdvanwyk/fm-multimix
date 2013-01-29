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
 * This is a simple circular buffer used for buffering input data. 
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "circ_buffer.h"

void circInit(struct circ_buf* buffer, int size)
{
	buffer->begin =0;
	buffer-> size = size;
	buffer-> count = 0;
	buffer->buffer = (uint8_t*) malloc(size);
}
void circFree(struct circ_buf* buffer, int size)
{
	free (buffer->buffer);
}

int circSpaceLeft(struct circ_buf* buffer)
{
	return buffer->size - buffer->count;
}

int circCount(struct circ_buf* buffer)
{
	return buffer->count;
}

void circWrite(struct circ_buf* buffer, uint8_t* elements, int len)
{
	int rem;
	int end = (buffer->begin + buffer->count) % buffer->size;
	if(buffer-> size - end < len) //wrap around
	{
		rem = len - (buffer->size - end);
		memcpy(buffer->buffer+end, elements, (buffer->size - end));
		memcpy(buffer->buffer, elements+(buffer->size - end), rem);
	}
	else
	{
		memcpy(buffer->buffer+end , elements, len);
	}

	if(buffer->count + len > buffer->size)
	{
		buffer->begin= (buffer->begin + len) % buffer->size;
		buffer->count = buffer->size;
	}
	else
	{
		buffer->count += len;
	}

}

int circRead(struct circ_buf* buffer, uint8_t* out, int len)
{
	int rem;
	if(len > buffer->count)
	{
		len = buffer->count;
	}
	if(buffer->size - buffer->begin < len) //wrap around
	{
		rem = len - (buffer->size - buffer->begin);
		memcpy(out, buffer->buffer + buffer->begin, (buffer->size - buffer->begin));
		memcpy(out + (buffer->size - buffer->begin), buffer->buffer, rem);
	}
	else
	{
		memcpy(out, buffer->buffer+buffer->begin, len);
	}
	buffer->begin= (buffer->begin + len) % buffer->size;
	buffer->count -= len;
	return len;
}

int circPeek(struct circ_buf* buffer, uint8_t* out, int len)
{
	int rem;
	if(len > buffer->count)
	{
		len = buffer->count;
	}
	if(buffer->size - buffer->begin < len) //wrap around
	{
		rem = len - (buffer->size - buffer->begin);
		memcpy(out, buffer->buffer + buffer->begin, (buffer->size - buffer->begin));
		memcpy(out + (buffer->size - buffer->begin), buffer->buffer, rem);
	}
	else
	{
		memcpy(out, buffer->buffer+buffer->begin, len);
	}

	return len;
}

void circReadAdvance(struct circ_buf* buffer, int len)
{
	buffer->begin= (buffer->begin + len) % buffer->size;
	buffer->count -= len;
}
