/**
 *  IW0HDV Extio
 *
 *  Copyright 2017 by Andrea Montefusco IW0HDV
 *
 *  Licensed under GNU General Public License 3.0 or later. 
 *  Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 */

#if !defined __AIRSPYHF_HPP__
#define      __AIRSPYHF_HPP__

#include <airspyhf.h>

struct AirSpyHfRxIQSample {
	float f;
	float float_32 ()
	{	
	    return f;
	}
};

const int AIRSPYHF_DEFAULT_FREQUENCY = 7050000;


#if defined CONSOLE
const int dbg_lvl = 5;
#else
const int dbg_lvl = -1;
#endif


class AirSpyHfRadio {
public:
	AirSpyHfRadio (const char *sn);
	virtual ~AirSpyHfRadio ();

	static int scan_devices (const char ***dev_list);
	const char *get_serial ();
	const char *get_serial_from_hw ();

	bool status ();
	const char *last_error();
	int open();
	int start (int bufsize);
	int stop();
	int set_sample_rate (int sr);
	int get_sample_rate ();

	int set_frequency (int);
	int get_frequency ();

	int get_calibration (int32_t *ppb = 0);
	int set_calibration (int32_t);
	int flash_calibration ();

	int set_user_output(airspyhf_user_output_t pin, airspyhf_user_output_state_t value);

	/* utilities */
	const char* board_id_name();

	const int get_samplerate_n (unsigned int n);
	int get_samplerates () { return n_sr_; }
	static void get_lib_version (int &major, int &minor, int &revision);
	const char* version_string ();
	
protected:	

	virtual int data_available (void *, int) = 0;

	int		sr_;
	int		f_;
	
//private:
public:
	struct airspyhf_device* device;
	uint64_t serial_number_val;
  char     serial[128];
  // callback buffer	
	int bs_;
	uint8_t *buffer;
	int bl_;
  static int callback(airspyhf_transfer_t *);
  char version[256];
	uint32_t   n_sr_;
	uint32_t  *srs_;
	int32_t    ppb_cal_;

	static unsigned int ndev_;
	static uint64_t *serials_;
	static char **ser_strings_;
};

#endif