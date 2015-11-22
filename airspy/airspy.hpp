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

#if !defined __AIRSPY_HPP__
#define      __AIRSPY_HPP__

#include <airspy.h>

struct AirSpyRxIQSample {
	float f;
	float float_32 ()
	{	
	    return f;
	}
};

const int AIRSPY_DEFAULT_SAMPLE_RATE = 2500000;
const int AIRSPY_DEFAULT_FREQUENCY = 103500000;


#if defined CONSOLE
const int dbg_lvl = 5;
#else
const int dbg_lvl = -1;
#endif


class AirSpyRadio {
public:
	AirSpyRadio ();
	virtual ~AirSpyRadio ();
	const char *get_serial ();
	
	bool status ();
	const char *last_error();
	int open();
	int start (int bufsize);
	int stop();
	int set_sample_rate (int sr);
	int get_sample_rate ();

	int set_frequency (int);
	int get_frequency ();

	/* Parameter value shall be between 0 and 15 */
    int set_lna_gain(uint8_t value);
    int get_lna_gain() { return lna_gain_ ; }

    /* Parameter value shall be between 0 and 15 */
    int set_mixer_gain(uint8_t value);
    int get_mixer_gain() { return mixer_gain_ ; }

    /* Parameter value shall be between 0 and 15 */
    int set_vga_gain(uint8_t value);
    int get_vga_gain() { return vga_gain_ ; }

    /* Parameter value:
    	0=Disable LNA Automatic Gain Control
    	1=Enable LNA Automatic Gain Control
    */
    int set_lna_agc(uint8_t value);
    /* Parameter value:
    	0=Disable MIXER Automatic Gain Control
    	1=Enable MIXER Automatic Gain Control
    */
    int set_mixer_agc(uint8_t value);
    
    /* Parameter value shall be 0=Disable BiasT or 1=Enable BiasT */
    int set_rf_bias(uint8_t value);
    
    const char* board_id_name();
	
	const int get_samplerate_n (int n);
	int get_samplerates () { return n_sr_; }
	static void get_lib_version (int &major, int &minor, int &revision);
	const char* version_string ();
	
protected:	

	virtual int data_available (void *, int) = 0;

	int		sr_;
	int		f_;
	
	int     vga_gain_;
	int     mixer_gain_;
	int     lna_gain_;
	
private:
	struct airspy_device* device;
	uint64_t serial_number_val;
    char     serial[128];
    // callback buffer	
	int bs_;
	uint8_t *buffer;
	int bl_;
    static int callback(airspy_transfer_t *);
    char version[256];
	uint32_t   n_sr_;
	uint32_t  *srs_;
};

#endif