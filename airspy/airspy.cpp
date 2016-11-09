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
#include <algorithm>
#include <airspy.h>
#include "log.h"
#include "airspy.hpp"


void AirSpyRadio::get_lib_version (int &major, int &minor, int &revision)
{
	static airspy_lib_version_t lib_version;
	airspy_lib_version(&lib_version);
	major = lib_version.major_version, minor = lib_version.minor_version, revision = lib_version.revision; 
}


AirSpyRadio::AirSpyRadio ():
	sr_(-1),
	vga_gain_(5), mixer_gain_(10), lna_gain_(5),
	device(0),
	serial_number_val(0),
	bs_(0),
	buffer(0),
	bl_(0),
	n_sr_(0),
	srs_(0)
{
	strcpy (serial,"");
	int result = airspy_init();
    if ( result != AIRSPY_SUCCESS ) {
        LOGT("airspy_init() failed: %s (%d)\n", airspy_error_name((airspy_error)result), result);
        return;
    }
	{
		int major, minor, revision;
        AirSpyRadio::get_lib_version (major, minor, revision);
	    LOGT("airspy_lib_version: %d.%d.%d\n", major, minor, revision); 
	}
}


AirSpyRadio::~AirSpyRadio ()
{
    if (device) {
        // call to airspy_stop_rx() is not mandatory because it is recalled later into airspy_close()

        // turn off preamplifier's power supply
		set_rf_bias(0);
    
        int result = airspy_close(device);
        if( result != AIRSPY_SUCCESS ) {
           LOGT("airspy_close() failed: %s (%d)\n", airspy_error_name((airspy_error)result), result);
        }
    }
	delete [] srs_;
    airspy_exit();
	LOGT("%s\n", "~AirSpyRadio" );
}


bool AirSpyRadio::status ()
{
    return true;
}

const char *AirSpyRadio::last_error()
{
    return "TBI";
}

const char *AirSpyRadio::get_serial ()
{
    airspy_read_partid_serialno_t read_partid_serialno;
	
    int result = ::airspy_board_partid_serialno_read(device, &read_partid_serialno);
    if (result != AIRSPY_SUCCESS) {
        LOGT("failed: %s (%d)\n", airspy_error_name((airspy_error)result), result);
        return "UNKNOWN";
	} else {
	    snprintf (serial, sizeof(serial), "%08X%08X", 
		          read_partid_serialno.serial_no[2],
				  read_partid_serialno.serial_no[3]);
	}
	return serial;
}

int AirSpyRadio::open ()
{
	int result = airspy_open(&device);
    if( result != AIRSPY_SUCCESS ) {
       LOGT("airspy_open() failed: %s (%d)\n", airspy_error_name((airspy_error)result), result);
	   return -1;
    } else {
		::airspy_get_samplerates(device, &n_sr_, 0);
		srs_ = new uint32_t [n_sr_];
		::airspy_get_samplerates(device, srs_, n_sr_);
		return 0;
	}
}


int AirSpyRadio::start (int bufsize)
{
    int result = ::airspy_set_sample_type(device, AIRSPY_SAMPLE_FLOAT32_IQ);
    if( result != AIRSPY_SUCCESS ) {
        LOGT("airspy_set_sample_type() failed: %s (%d)\n", airspy_error_name((airspy_error)result), result);
		return -1;
    }
	
	set_sample_rate(sr_);
	set_vga_gain(vga_gain_);
	set_mixer_gain(mixer_gain_);
	set_lna_gain(lna_gain_);
	
	result = ::airspy_start_rx(device, callback, this);
    if( result != AIRSPY_SUCCESS ) {
        LOGT("airspy_start_rx() failed: %s (%d)\n", airspy_error_name((airspy_error)result), result);
        return -1;
    } else {
        buffer = new uint8_t [bufsize];
	    bs_ = bufsize;
	    bl_ = 0;
	    return 0;
	}
}

int AirSpyRadio::stop ()
{
	int result = airspy_stop_rx(device);
    if( result != AIRSPY_SUCCESS ) {
        LOGT("airspy_stop_rx() failed: %s (%d)\n", airspy_error_name((airspy_error)result), result);
        return -1;
    } else {
	    delete [] buffer;
		bs_ = bl_ = 0 ;
	    return 0;
	}
}

int AirSpyRadio::set_sample_rate (int nsr)
{
	sr_ = nsr;
	int result = airspy_set_samplerate(device, sr_);
    if( result != AIRSPY_SUCCESS ) {
        LOGT("airspy_set_samplerate(%d) failed: %s (%d)\n", sr_, airspy_error_name((airspy_error)result), result);
		return -1;
    } else
	    return 0;
}

int AirSpyRadio::get_sample_rate ()
{
	LOGT("airspy_get_samplerate: (%d)\n", sr_);
	return sr_;
}


int AirSpyRadio::set_frequency (int nf)
{
	int result = airspy_set_freq(device, f_ = nf);
    if( result != AIRSPY_SUCCESS ) {
        LOGT("airspy_set_freq() failed: %s (%d)\n", airspy_error_name((airspy_error)result), result);
        return -1;
    } else 
	    return 0;
}

int AirSpyRadio::get_frequency ()
{
    return f_;
}


int AirSpyRadio::callback(airspy_transfer_t* transfer)
{
    AirSpyRadio *p;
    if (transfer) p = static_cast<AirSpyRadio *> (transfer->ctx);

    // AIRSPY_SAMPLE_FLOAT32_IQ:
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
	return 0;
}

/* Parameter value shall be between 0 and 15 */
int AirSpyRadio::set_lna_gain(uint8_t value)
{
	int result = ::airspy_set_lna_gain(device, lna_gain_ = value);
    if( result != AIRSPY_SUCCESS ) {
        LOGT("airspy_set_lna_gain() failed: %s (%d)\n", airspy_error_name((airspy_error)result), result);
    }
	return (result == AIRSPY_SUCCESS ? 0 : -1);
}

/* Parameter value shall be between 0 and 15 */
int AirSpyRadio::set_mixer_gain(uint8_t value)
{
	int result = ::airspy_set_mixer_gain(device, mixer_gain_ = value);
    if( result != AIRSPY_SUCCESS ) {
        LOGT("airspy_set_mixer_gain() failed: %s (%d)\n", airspy_error_name((airspy_error)result), result);
    }
	return (result == AIRSPY_SUCCESS ? 0 : -1);
}


/* Parameter value shall be between 0 and 15 */
int AirSpyRadio::set_vga_gain(uint8_t value)
{
	int result = ::airspy_set_vga_gain(device, vga_gain_ = value);
    if( result != AIRSPY_SUCCESS ) {
         LOGT("airspy_set_vga_gain() failed: %s (%d)\n", airspy_error_name((airspy_error)result), result);
    }
	return (result == AIRSPY_SUCCESS ? 0 : -1);
}


/* Parameter value:
	0=Disable LNA Automatic Gain Control
	1=Enable LNA Automatic Gain Control
*/
int AirSpyRadio::set_lna_agc(uint8_t value)
{
	int result = ::airspy_set_lna_agc(device, value);
    if( result != AIRSPY_SUCCESS ) {
         LOGT("airspy_set_lna_agc() failed: %s (%d)\n", airspy_error_name((airspy_error)result), result);
    }
	return (result == AIRSPY_SUCCESS ? 0 : -1);
}


/* Parameter value:
	0=Disable MIXER Automatic Gain Control
	1=Enable MIXER Automatic Gain Control
*/
int AirSpyRadio::set_mixer_agc (uint8_t value)
{
	int result = ::airspy_set_mixer_agc(device, value);
    if( result != AIRSPY_SUCCESS ) {
         LOGT("airspy_set_mixer_agc() failed: %s (%d)\n", airspy_error_name((airspy_error)result), result);
    }
	return (result == AIRSPY_SUCCESS ? 0 : -1);
}


/* Parameter value shall be 0=Disable BiasT or 1=Enable BiasT */
int AirSpyRadio::set_rf_bias (uint8_t value)
{
	int result = ::airspy_set_rf_bias(device, value);
    if( result != AIRSPY_SUCCESS ) {
         LOGT("airspy_set_rf_bias() failed: %s (%d)\n", airspy_error_name((airspy_error)result), result);
    }    return (result == AIRSPY_SUCCESS ? 0 : -1);
}


/* Parameter value: 0..21 */
int AirSpyRadio::set_linearity_gain (uint8_t value)
{
	int result = ::airspy_set_linearity_gain(device, value);
    if( result != AIRSPY_SUCCESS ) {
         LOGT("airspy_set_linearity_gain() failed: %s (%d)\n", airspy_error_name((airspy_error)result), result);
    }    return (result == AIRSPY_SUCCESS ? 0 : -1);
}


/* Parameter value: 0..21 */
int AirSpyRadio::set_sensitivity_gain (uint8_t value)
{
	int result = ::airspy_set_sensitivity_gain(device, value);
    if( result != AIRSPY_SUCCESS ) {
         LOGT("airspy_set_sensitivity_gain() failed: %s (%d)\n", airspy_error_name((airspy_error)result), result);
    }    return (result == AIRSPY_SUCCESS ? 0 : -1);
}


const char* AirSpyRadio::board_id_name ()
{
    uint8_t bid;
    if (::airspy_board_id_read(device, &bid) == AIRSPY_SUCCESS)
        return ::airspy_board_id_name((airspy_board_id)bid);
	else
	    return "UNKNOWN";
}

const char* AirSpyRadio::version_string ()
{
	::memset (version, 0, sizeof(version));
	if (AIRSPY_SUCCESS == ::airspy_version_string_read(device, version, sizeof(version)-1)) {
		return version;
	} else
		return "UNKNOWN";
}

const int AirSpyRadio::get_samplerate_n (unsigned int n)
{
	if (n_sr_ && n >= 0 && n < n_sr_)
		return srs_[n];
	else
		return -1;
}

