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
 * This is a process spawner that creates rtl-fm processes if something worth 
 * demodulating is found. This requires a modified version of rtl-fm that can be
 * found at <https://github.com/TonberryKing/rtlsdr>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "fft.h"
#include "demod_proc.h"

demodproc* bin_list[FFT_LEN];
demodproc* short_proc_list[FFT_LEN];
int process_count=0;

int create_process(int bin, long long int totalread)
{
	int i;
	int in[2], out[2], pid;	
	demodproc* newproc = malloc(sizeof(demodproc));
	if (pipe(in) < 0) return -1;
	if (pipe(out) < 0) return -1;

	if((pid=fork()) == 0)
	{
		char cmdstring[255];
		//child process
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		//close(stderr);

		dup2(out[0],STDIN_FILENO);
		dup2(in[1],STDOUT_FILENO);
		fcntl(in[1], F_SETFL, fcntl(in[0], F_GETFL) &~ O_NONBLOCK ); 
		fcntl(out[0], F_SETFL, fcntl(out[1], F_GETFL) &~ O_NONBLOCK ); 

		close(in[0]);
		close(out[1]);
		//snprintf(cmdstring, 255, "cat > output%d.raw", bin);
		snprintf(cmdstring, 255, "rtl_fm -s 22050 -P -C -i 46 -l 150 - |"
				"sox -t raw -r 22050 -e signed-integer -b 16 -c 1 -L - "
				"-r 8000 output%d_%lld.wav sinc 0-3000 -n 128 ", bin, totalread/SAMP_RATE/2);

		execl("/bin/sh", "sh", "-c", cmdstring , (char *)NULL);

		fprintf(stderr,"Failed to start child process\n");
		exit(1);
	}

	newproc->inpipe = in[0];
	newproc->outpipe = out[1];
	newproc->bin = bin;
	newproc->detection_misses = 0;
	newproc->serviced = 0;

	newproc->real = sine_set_up(-(float)(abs(bin-(FFT_LEN/2)))/FFT_LEN , 0, 65535);
	//amplitude is 2^16-1
	if(bin-(FFT_LEN/2) < 0)
	{
		newproc->imag = sine_set_up(-(float)(abs(bin-(FFT_LEN/2)))/FFT_LEN , -90,  65535);
	}
	else
	{
		newproc->imag = sine_set_up(-(float)(abs(bin-(FFT_LEN/2)))/FFT_LEN , 90,  65535);
	}
	

	fcntl(in[0], F_SETFL, fcntl(in[0], F_GETFL) | O_NONBLOCK ); 
	fcntl(out[1], F_SETFL, fcntl(out[1], F_GETFL) | O_NONBLOCK ); 

	close(in[1]);
	close(out[0]);

	bin_list[bin]=newproc;
	short_proc_list[0]=newproc;

	//rewrite the short process list
	process_count=0;
	for(i=0;i<FFT_LEN;i++)
	{
		if(bin_list[i]!=0)
		{
			short_proc_list[process_count] = bin_list[i];
			process_count++;
		}
	}

	return 0;
}
void end_process(demodproc* proc)
{
	int i;
	close(proc->outpipe);
	close(proc->inpipe);
	bin_list[proc->bin]=0;
	free(proc);
	
	//rewrite the short process list
	process_count=0;
	for(i=0;i<FFT_LEN;i++)
	{
		if(bin_list[i]!=0)
		{
			short_proc_list[process_count] = bin_list[i];
			process_count++;
		}
	}
}

demodproc** get_process_list()
{
	return short_proc_list;
}

int get_process_count()
{
	return process_count;
}

void check_processes(double* bins, int* freqs, int freqcount, 
		long long int total_read, int misses, int center_freq) 
{
	int i,j;
	int miss =1;
	for(i=0; i<freqcount; i++)
	{
		miss=1;
		for(j=-(BANDWIDTH_BINS/2); j<(BANDWIDTH_BINS/2); j++)
		{
			if(bins[freqs[i]+j] >0)
			{
				if(bin_list[freqs[i]]!=0)
				{
					//fprintf(stderr, "has %d, happy\n", freqs[i]);
				}
				else
				{
					fprintf(stderr, "Creating process to demodulate %d Hz\n", 
							(freqs[i]-(FFT_LEN/2))*SAMP_RATE/FFT_LEN+center_freq);
					create_process(freqs[i],total_read);
				}
				miss =0;
				bin_list[freqs[i]]->detection_misses=0;
				break;
			}
		}
		if(miss)
		{
			if(bin_list[freqs[i]]!=0)
			{
				bin_list[freqs[i]]->detection_misses++;
				if(bin_list[freqs[i]]->detection_misses>misses)
				{
					fprintf(stderr, "Ending demodulator for %d Hz\n", 
							(freqs[i]-(FFT_LEN/2))*SAMP_RATE/FFT_LEN+center_freq);
					end_process(bin_list[freqs[i]]);
				}
			}
		}

	}
}
