/*
    fm-multimix  A multiple channel fm downmixer.
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
 *This is the main program. A loop with a call to select() reads data 
 *from stdin, compares possible transmissions to a list of frequencies 
 *that should be recorded and spawns rtl-fm if required.
 *Mixing is also done here.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include "fft.h"
#include "sinegen.h"
#include "demod_proc.h"

fd_set readdesc;
fd_set writedesc;
uint8_t inbuf [FFT_LEN*2];
uint8_t outbuf [FFT_LEN*2];
int hasbuf =0;
fft_obj* chanfinder_obj;
int chans_found;
double* results;
int skip=0;
int i;
int readcount;
int highdesc = 0;
int descready;
demodproc** demod_processes;
int center_freq=0;
long long int samples_read=0;
int freqs[FFT_LEN];
int freq_count=0;
int verbosity=0;

void mix_signal(demodproc* proc, uint8_t* inbuf, uint8_t* outbuf)
{
	int j;
	int32_t tmpreal;
	int32_t tmpimag;
	int32_t tmpinreal;
	int32_t tmpinimag;
	
	for(j=0;j<FFT_LEN*2;j+=2)
	{
		tmpreal = sine_next_val(proc->real);
		tmpimag = sine_next_val(proc->imag);
		tmpinreal = inbuf[j]-128;
		tmpinimag = inbuf[j+1]-128;
		//fprintf(stderr, "%f %f \n", tmpreal, tmpimag);
		
		outbuf[j]=(uint8_t)((((tmpinreal*tmpreal) - (tmpinimag*tmpimag))>>16)+ 128);
		outbuf[j+1]=(uint8_t)((((tmpinimag*tmpreal) + (tmpinreal*tmpimag))>>16)+ 128);
	}
}

void set_file_descriptors()
{
	FD_ZERO(&writedesc);
	FD_ZERO(&readdesc);

	if(!hasbuf)
	{
		FD_SET(STDIN_FILENO, &readdesc);
		if(STDIN_FILENO > highdesc)
		{
			highdesc = STDIN_FILENO;
		}
		for(i=0; i<get_process_count(); i++)
		{
			demod_processes[i]->serviced=0;
		}
	}
	if(hasbuf)
	{
		for(i=0; i<get_process_count(); i++)
		{
			if(demod_processes[i]->serviced == 0)
			{
				FD_SET(demod_processes[i]->outpipe, &writedesc);
				if(demod_processes[i]->outpipe > highdesc)
				{
					highdesc = demod_processes[i]->outpipe;
				}
			}
		}
	}
}

int read_data()
{
	if(FD_ISSET(STDIN_FILENO, &readdesc))
	{
		readcount = fread(inbuf, sizeof(uint8_t), FFT_LEN*2, stdin);  
		//	fprintf(stderr,"reading\n");
		if(readcount == 0)
		{
			fprintf(stderr,"mixer end of file\n");
			return 0;
		}
		if(readcount == -1)
		{
			if(errno == EAGAIN || errno == EWOULDBLOCK)
			{
				fprintf(stderr,"again or wouldblock stdin read\n");
			}
			else
			{
				fprintf(stderr,"error reading from stdin, exiting \n");
				return 0;
			}
		}
		hasbuf=1;
		samples_read+=FFT_LEN*2;

		
		if(skip == 0)
		{
			chans_found = do_fft(chanfinder_obj, inbuf, readcount);

			if(chans_found)
			{
				results= get_fft_results(chanfinder_obj);
				int i;
				for(i=0; i<FFT_LEN; i++)
				{
					if(results[i] > 0)
					{
						fprintf(stderr,"something at %d (%dHz) with %f\n",
								i,(i-(FFT_LEN/2))*SAMP_RATE/FFT_LEN+center_freq, results[i]);
					}
				}
				check_processes(results, freqs, freq_count, samples_read); 
			}
			skip = 50;
		}
		else
		{
			skip--;
		}
	}
	return 1;
}

int write_to_demod()
{
	for(i=0; i<get_process_count(); i++)
	{
		if(FD_ISSET(demod_processes[i]->outpipe, &writedesc))
		{
			mix_signal(demod_processes[i],inbuf,outbuf);
			int writecount = write(demod_processes[i]->outpipe, outbuf, FFT_LEN*2);
			if(writecount == 0)
			{
				close(demod_processes[i]->outpipe);
			}
			if(writecount == -1)
			{
				if(errno == EAGAIN || errno == EWOULDBLOCK)
				{
					fprintf(stderr,"again or wouldblock pipe write\n");
				}
				else
				{
					fprintf(stderr,"error writing to pipe, exiting \n");
					return 0;
				}
			}
			//	hasbuf=0;
			demod_processes[i]->serviced=1;

		}
	}

	//check if all processes have been serviced

	int serviced=1;
	for(i=0; i<get_process_count(); i++)
	{
		if(!demod_processes[i]->serviced)
		{
			serviced = 0;
			break;
		}
	}
	if(serviced)
	{
		hasbuf = 0;
	}
	return 1;
}

void arg_msg()
{
	fprintf(stderr, "put something descriptive here\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	struct timeval timeout;
	int running =1;
	int opt;

	while ((opt = getopt(argc, argv, "f:v")) != -1) 
	{
		switch (opt) 
		{
			case 'f':
				center_freq = (int)atof(optarg);
				break;
			case 'v':
				verbosity++;
				break;
			default:
				arg_msg();
				break;
		}
	}
	while(optind < argc)
	{
			freqs[freq_count]=(int)atoi(argv[optind]);
			freq_count++;
			optind++;
	}
	if(freq_count == 0)
	{
		arg_msg();
	}

	int min,max;
	int i;
	min=freqs[0];
	max=freqs[0];
	for(i=1; i<freq_count; i++)
	{
		if(min>freqs[i])
		{
			min = freqs[i];
		}	
		if(max<freqs[i])
		{
			max=freqs[i];
		}
	}
	if(max-min > 1000000)//slightly less than actual sampled bandwidth to avoid issues with sampling at the center frequency.
	{
		fprintf(stderr, "Difference between maximum and minimum freqencies exceed sampled spectrum\n");
		exit(1);
	}
	if(!center_freq)
	{
		center_freq= (min+max)/2;

		//do not use frequencies around center_freq.
		//combination of dc from ADC and artifacts from splitting spectrum.
	//	for(i=0;i<freq_count;i++)
	//	{
	//		if(freqs[i]< center_freq+BANDWIDTH && freqs[i]> center_freq-BANDWIDTH)
	//		{
	//			center_freq+=BANDWIDTH;
	//			break;
	//		}
	//	}
	//	for now this has to be handled externally
	}

	for(i=0;i<freq_count;i++)
	{
		freqs[i]=((freqs[i]-center_freq) * FFT_LEN / SAMP_RATE)+(FFT_LEN/2);
	}

	demod_processes	= get_process_list();
	chanfinder_obj = fft_init();

	fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK ); 
	fcntl(STDOUT_FILENO, F_SETFL, fcntl(STDOUT_FILENO, F_GETFL) | O_NONBLOCK);
	while (running)
	{
		set_file_descriptors();

		timeout.tv_sec=1;
		timeout.tv_usec=0;

		descready = select( highdesc+1, &readdesc, &writedesc, NULL, &timeout);

		if(descready != -1)
		{
			if(!read_data())
			{
				running =0;
				continue;
			}

			if(!write_to_demod())
			{
				running = 0;
				continue;
			}
		}
	}
	return 0;
}
