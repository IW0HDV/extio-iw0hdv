/**
 *  IW0HDV Extio
 *
 *  Copyright 2015 by Andrea Montefusco IW0HDV
 *
 *  Licensed under GNU General Public License 3.0 or later. 
 *  Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 */

#if !defined __PERSEUS_H__
#define __PERSEUS_H__

struct PerseusRxIQSample {
	unsigned char s1;  // MSB
	unsigned char s2;
	unsigned char s3;  // LSB

	int int_32 ()
	{	
		int rv = 0;
		rv = (s1 << 16) | (s2 << 8) | (s3);
		// sign extension
		if (rv & 0x00800000) rv = rv | 0xff000000;
		return rv;
	}
	float float_32 ()
	{	
		int rv;
		// samples from hardware are 24 bit signed integer
		// put them in a regular int
		rv  = (int)((signed char)   s1) << 16;
		rv += (int)((unsigned char) s2) << 8 ;
		rv += (int)((unsigned char) s3)      ;
		// next, rescale to +1.0/-1.0
		return ((float)rv)/8388607.0f;
	}
};


const int PERSEUS_DEFAULT_SAMPLE_RATE = 192000;
const int PERSEUS_DEFAULT_FREQUENCY = 7070000;


#if defined CONSOLE
const int dbg_lvl = 5;
#else
const int dbg_lvl = -1;
#endif

class PRimpl;

class PerseusRadio {
public:
	PerseusRadio ();
	virtual ~PerseusRadio ();
	const char *get_serial ();
	
	bool status ();
	const char *last_error();
	int open();
	int start (int bufsize);
	int stop();
	int set_sample_rate (int sr);
	int get_sample_rate ();
	const int *get_sample_rate_v (int &);
	int set_frequency (int);
	int get_frequency ();
	int set_attenuator (int);
	int set_preselector (int);
	int set_preamp (int);
	int set_dither (int);
	
protected:	

	virtual int data_available (void *, int);

	int		sr;
	int		f;
	bool	att;
	int		dither;
	int		preamp;
	bool	presel;
	
private:
	PRimpl *pi;
	int	nsr_;
	int	*srv;
	char *serial;
	
static int callback(void *buf, int buf_size, void *obj);
	
};

#endif