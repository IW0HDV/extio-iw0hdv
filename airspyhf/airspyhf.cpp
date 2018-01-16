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

#include <memory.h>
#include <inttypes.h>
#include <algorithm>
#include <airspyhf.h>
#include "log.h"
#include "airspyhf.hpp"


void AirSpyHfRadio::get_lib_version (int &major, int &minor, int &revision)
{
	static airspyhf_lib_version_t lib_version;
	airspyhf_lib_version(&lib_version);
	major = lib_version.major_version, minor = lib_version.minor_version, revision = lib_version.revision; 
}

unsigned int AirSpyHfRadio::ndev_ = 0;
uint64_t *AirSpyHfRadio::serials_ = 0;
char **AirSpyHfRadio::ser_strings_ = 0;

AirSpyHfRadio::AirSpyHfRadio (const char *sn):
	sr_(-1),
	device(0),
	serial_number_val(0),
	bs_(0),
	buffer(0),
	bl_(0),
	n_sr_(0),
	srs_(0),
	ppb_cal_(0)
{
	if (sn)
		strcpy (serial, sn), strupr(serial);
	else
		strcpy (serial,"");
	LOGT("device name: [%s]\n", serial);
	{
		int major, minor, revision;
    AirSpyHfRadio::get_lib_version (major, minor, revision);
	  LOGT("airspyhf_lib_version: %d.%d.%d\n", major, minor, revision); 
	}
  scan_devices (0);
}

int AirSpyHfRadio::scan_devices (const char ***dev_list)
{
	if (ndev_ == 0) {
		// get device list length
		unsigned int nd = airspyhf_list_devices(0, 0);

	  if (nd > 0) {
			ndev_ = nd;
			LOGT("# of device(s): %d\n", nd);
			serials_ = new uint64_t [nd];
			ser_strings_ = new char * [nd];
			airspyhf_list_devices(serials_, nd);
			for (unsigned int i=0; i < nd; ++i) {
				char b[256];
				snprintf (b, sizeof(b), "%llX", serials_[i]);
				ser_strings_[i] = new char [strlen(b)+1];
				strcpy (ser_strings_[i], b);
				LOGT("device #%d: %s\n", i, ser_strings_[i]);
			}
		} else {
			LOGT("%s\n", "airspyhf_list_devices: no devices found !");
			return 0;
		}
	}

	if (ndev_  && dev_list) *dev_list = (const char **)ser_strings_;

	return ndev_;
}

AirSpyHfRadio::~AirSpyHfRadio ()
{
	if (device) {
		// call to airspy_stop_rx() is not mandatory because it is recalled later into airspyhf_close()
		int result = airspyhf_close(device);
		if( result != AIRSPYHF_SUCCESS ) {
			//LOGT("airspyhf_close() failed: %s (%d)\n", airspyhf_error_name((airspyhf_error)result), result);
    }
	}
	delete [] srs_;
	delete [] serials_;
	for (unsigned int i=0; i<ndev_; ++i) delete [] ser_strings_[i];
	delete [] ser_strings_;
	ndev_ = 0;
	srs_ = 0;
	serials_ = 0;
  //airspyhf_exit() deprecated
	LOGT("%s\n", "~AirSpyHfRadio" );
}


bool AirSpyHfRadio::status ()
{
	return true;
}

const char *AirSpyHfRadio::last_error()
{
	return "last error not available";
}

const char *AirSpyHfRadio::get_serial ()
{
	return serial;
}

const char *AirSpyHfRadio::get_serial_from_hw ()
{
  airspyhf_read_partid_serialno_t read_partid_serialno;
	
	strcpy(serial, "");
  int result = ::airspyhf_board_partid_serialno_read(device, &read_partid_serialno);
  if (result != AIRSPYHF_SUCCESS) {
        LOGT("failed: (%d)\n", result);
	} else {
		snprintf (serial, sizeof(serial), "%08X%08X",
							read_partid_serialno.serial_no[0],
							read_partid_serialno.serial_no[1]
							);
	}
	return serial;
}

int AirSpyHfRadio::open ()
{
	int result;

	LOGT("airspyhf_open(): serial len: %d\n", strlen(serial));
	if (strlen (serial) > 0) {
		uint64_t s = 0;
		for (unsigned i=0; i < strlen(serial); ++i) {
			char ch = serial[i];
			uint8_t x = 0;
			if (ch >= '0' && ch <= '9')
				x = ch - '0';
			else
				x = (ch - 'A') + 10;
			s = (s << 4) | x;
		}
		result = airspyhf_open_sn(&device, s);
		LOGT("airspyhf_open(): [%s] (%llX)\n", serial, s);
	} else {
		result = airspyhf_open(&device);
		LOGT("airspyhf_open(): generic open: %d\n", result);
		if( result == AIRSPYHF_SUCCESS )
			result = strlen(get_serial_from_hw ()) == 0 ? AIRSPYHF_ERROR: AIRSPYHF_SUCCESS;
	}
	if( result != AIRSPYHF_SUCCESS ) {
		LOGT("airspyhf_open() failed: (%d)\n", result);
		return -1;
	} else {
		// get calibration value from radio
		get_calibration ();
		// querying with '0' returns the number of sample rates available
		::airspyhf_get_samplerates(device, &n_sr_, 0);
		if (n_sr_ > 0) {
			LOGT("sample rates available: (%d)\n", n_sr_);
			srs_ = new uint32_t [n_sr_]; // set up a properly sized vector
			result = ::airspyhf_get_samplerates(device, srs_, n_sr_); // read the list
			if( result != AIRSPYHF_SUCCESS ) {
				LOGT("::airspyhf_get_samplerates() failed: (%d)\n", result);
			} else {
				for (unsigned i=0; i < n_sr_; ++i) {
					LOGT("samplerate(%d): %d\n", i, srs_[i]);
				}
			}
			return 0;
		} else
			return -1;
	}
}


int AirSpyHfRadio::start (int bufsize)
{
	// sample type is always float 32
	#if 0
    int result = ::airspyhf_set_sample_type(device, AIRSPY_SAMPLE_FLOAT32_IQ);
    if( result != AIRSPYHF_SUCCESS ) {
        //LOGT("airspy_set_sample_type() failed: %s (%d)\n", airspy_error_name((airspy_error)result), result);
		return -1;
    }
	#endif
	
	set_sample_rate(sr_);
	
	//set_vga_gain(vga_gain_);
	//set_mixer_gain(mixer_gain_);
	//set_lna_gain(lna_gain_);
	
	int result = ::airspyhf_start(device, callback, this);
	if( result != AIRSPYHF_SUCCESS ) {
		LOGT("airspy_start_rx() failed: (%d)\n", result);
		return -1;
	} else {
		buffer = new uint8_t [bufsize];
		bs_ = bufsize;
		bl_ = 0;
		return 0;
	}
}

int AirSpyHfRadio::stop ()
{
	int result = airspyhf_stop(device);
	if( result != AIRSPYHF_SUCCESS ) {
		LOGT("airspyhf_stop() failed: (%d)\n", result);
    return -1;
  } else {
	  delete [] buffer;
		bs_ = bl_ = 0 ;
	  return 0;
	}
}

int AirSpyHfRadio::set_sample_rate (int nsr)
{
	sr_ = nsr;
	int result = airspyhf_set_samplerate(device, sr_);
	if( result != AIRSPYHF_SUCCESS ) {
		LOGT("airspyhf_set_samplerate(%d) failed: (%d)\n", sr_, result);
		return -1;
	} else
		return 0;
}

int AirSpyHfRadio::get_sample_rate ()
{
	LOGT("airspyhf_get_samplerate: (%d)\n", sr_);
	return sr_;
}


int AirSpyHfRadio::set_frequency (int nf)
{
	int result = airspyhf_set_freq(device, f_ = nf);
    if( result != AIRSPYHF_SUCCESS ) {
        LOGT("airspyhf_set_freq() failed: (%d)\n", result);
        return -1;
    } else 
	    return 0;
}

int AirSpyHfRadio::get_frequency ()
{
    return f_;
}


int AirSpyHfRadio::callback(airspyhf_transfer_t* transfer)
{
	AirSpyHfRadio *p = 0;
	if (transfer) p = static_cast<AirSpyHfRadio *> (transfer->ctx);

	if (p) {
		// assume AIRSPYHF_SAMPLE_FLOAT32_IQ:
		uint32_t bytes_to_write = transfer->sample_count * 4 * 2; 
		uint8_t *pt_rx_buffer   = (uint8_t *)transfer->samples;
		
		for (;bytes_to_write;) {
			int spaceleft = p->bs_ - p->bl_ ;
			int to_copy = std::min((int)spaceleft, (int)bytes_to_write);
			
			::memcpy (p->buffer + p->bl_, pt_rx_buffer, to_copy);
		
			bytes_to_write -= to_copy;
			pt_rx_buffer   += to_copy;
			
			if (p->bl_ == p->bs_) {
				p->data_available ((void *)p->buffer, p->bl_);
				p->bl_ = 0;
			}
			p->bl_ += to_copy;
		}
	}
	return 0;
}

const char* AirSpyHfRadio::board_id_name ()
{
	#if 0
	uint8_t bid;
	if (::airspyhf_board_id_read(device, &bid) == AIRSPYHF_SUCCESS)
			return ::airspyhf_board_id_name((airspyhf_board_id)bid);
	else
	#endif
	return "AirSpyHf board";
}

const char* AirSpyHfRadio::version_string ()
{
	::memset (version, 0, sizeof(version));
	#if 1
	if (AIRSPYHF_SUCCESS == ::airspyhf_version_string_read(device, version, sizeof(version)-1)) {
		return version;
	} else
	#endif
	return "VERSION NOT READABLE";
}

const int AirSpyHfRadio::get_samplerate_n (unsigned int n)
{
	if (n_sr_ && n >= 0 && n < n_sr_)
		return srs_[n];
	else
		return -1;
}

int AirSpyHfRadio::get_calibration (int32_t *ppb)
{
	if (device && ::airspyhf_get_calibration(device, &ppb_cal_) == AIRSPYHF_SUCCESS) {
		LOGT("airspyhf_get_calibration: (%d)\n", ppb_cal_);
		if (ppb) *ppb = ppb_cal_;
		return 0;
	} else {
		return -1;
	}
}


int AirSpyHfRadio::set_calibration (int32_t ppb)
{
	if (device && ::airspyhf_set_calibration(device, ppb_cal_ = ppb) == AIRSPYHF_SUCCESS) {
		LOGT("airspyhf_set_calibration: (%d)\n", ppb_cal_);
		return 0;
	} else {
		return -1;
	}
}

int AirSpyHfRadio::set_user_output(airspyhf_user_output_t pin, airspyhf_user_output_state_t value)
{
	if (device && ::airspyhf_set_user_output(device, pin, value) == AIRSPYHF_SUCCESS) {
		LOGT("airspyhf_set_user_output: pin: %d, value: %d\n", pin, value);
		return 0;
	} else {
		return -1;
	}
}
