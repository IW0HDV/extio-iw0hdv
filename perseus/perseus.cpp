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

#include <perseus-sdr.h>
#include "log.h"
#include "perseus.h"

struct PRimpl {
	
	PRimpl ():num_perseus(0),descr(0)
	{
		memset (&prodid, sizeof(prodid), 0);
	}
	int num_perseus;
	perseus_descr *descr;
	eeprom_prodid prodid;
};

PerseusRadio::PerseusRadio ():
	sr(PERSEUS_DEFAULT_SAMPLE_RATE),
	f(PERSEUS_DEFAULT_FREQUENCY),
	att(false),
	dither(false),
	preamp(false),
	presel(false),
	nsr_(0),
	srv(0)
{
	pi.reset ( new PRimpl );
	// Set debug info dumped to stderr to the maximum verbose level
	perseus_set_debug(dbg_lvl);
			
    //printf ("Revision: %s\n", svn_revision);
	//printf ("SAMPLE RATE: %d\n", sr);
	//printf ("NBUF: %d BUF SIZE: %d TOTAL BUFFER LENGTH: %d\n", nb, bs, nb*bs);

	// Check how many Perseus receivers are connected to the system
	pi->num_perseus = perseus_init();
	LOGT("%d Perseus receivers found\n", pi->num_perseus);

	if (pi->num_perseus==0) {
		LOGT("%s\n", "No Perseus receivers detected");
		perseus_exit();
	}

}

const char *PerseusRadio::get_serial ()
{
	return serial.c_str();
}

bool PerseusRadio::status () 
{
	return (pi->num_perseus > 0);
}

const char * PerseusRadio::last_error()
{
	return perseus_errorstr();
}

int PerseusRadio::open () {
	// Open the first one...
	if ((pi->descr = perseus_open(0)) == 0) {
		LOGT("error: %s\n", perseus_errorstr());
    	return 255;
	}
	
	// Download the standard firmware to the unit
	LOGT("%s\n", "Downloading firmware...");
	if (perseus_firmware_download (pi->descr,0)<0) {
		LOGT("firmware download error: %s", perseus_errorstr());
		return 255;
	}

	char buf[1024] = {0};
	// Dump some information about the receiver (S/N and HW rev)
	if (pi->descr->is_preserie == TRUE) {
		LOGT("%s\n", "The device is a preserie unit");
		snprintf (buf, sizeof(buf), "%s", "preserie unit");
	} else
		if (perseus_get_product_id(pi->descr,&pi->prodid)<0) {
			LOGT("get product id error: %s", perseus_errorstr());
			snprintf (buf, sizeof(buf), "%s", perseus_errorstr());
    	} else {
			LOGT("Receiver S/N: %05d-%02hX%02hX-%02hX%02hX-%02hX%02hX - HW Release:%hd.%hd\n",
				(uint16_t) pi->prodid.sn, 
				(uint16_t) pi->prodid.signature[5],
				(uint16_t) pi->prodid.signature[4],
				(uint16_t) pi->prodid.signature[3],
				(uint16_t) pi->prodid.signature[2],
				(uint16_t) pi->prodid.signature[1],
				(uint16_t) pi->prodid.signature[0],
				(uint16_t) pi->prodid.hwrel,
				(uint16_t) pi->prodid.hwver );
			snprintf (buf, sizeof(buf), "%05d-%02hX%02hX-%02hX%02hX-%02hX%02hX - HW:%hd.%hd",
				(uint16_t) pi->prodid.sn, 
				(uint16_t) pi->prodid.signature[5],
				(uint16_t) pi->prodid.signature[4],
				(uint16_t) pi->prodid.signature[3],
				(uint16_t) pi->prodid.signature[2],
				(uint16_t) pi->prodid.signature[1],
				(uint16_t) pi->prodid.signature[0],
				(uint16_t) pi->prodid.hwrel,
				(uint16_t) pi->prodid.hwver );
				
		}
	if (strlen(buf)) {
		snprintf (buf, sizeof(buf), "%05d-%02hX%02hX-%02hX%02hX-%02hX%02hX",
				(uint16_t) pi->prodid.sn, 
				(uint16_t) pi->prodid.signature[5],
				(uint16_t) pi->prodid.signature[4],
				(uint16_t) pi->prodid.signature[3],
				(uint16_t) pi->prodid.signature[2],
				(uint16_t) pi->prodid.signature[1],
				(uint16_t) pi->prodid.signature[0] );
		serial = buf;
	}
	// Printing all sampling rates available .....
	{
		int buf[BUFSIZ];
		
	    if (perseus_get_sampling_rates (pi->descr, buf, (sizeof(buf)/sizeof(buf[0])) ) < 0) {
			LOGT("get sampling rates error: %s\n", perseus_errorstr());
			return 254;
		} else {
			int nsr = 0;
			while (buf[nsr]) {
				LOGT("#%d: sample rate: %d\n", nsr, buf[nsr]);
				srates.push_back (buf[nsr]);
				nsr++;
			}
			srv = new int [nsr];
			if (srv) for (int i=0; i<nsr; ++i) srv[i]=buf[i];
			nsr_ = nsr;
			
			
		}
	}
	
	// Configure the receiver for default MS/s operations
	LOGT("%s\n", "Configuring FPGA...");
	if (perseus_set_sampling_rate(pi->descr, sr) < 0) {  // specify the sampling rate value in Samples/second
		LOGT("fpga configuration error: %s\n", perseus_errorstr());
	}	

	return 0;
}


std::vector<int> PerseusRadio::get_sample_rate_v ()
{
	return srates;
}

int PerseusRadio::start (int bufsize)
{
	int rc = -255;
	if (pi->descr) {
		
		// Configure the receiver for default MS/s operations
		LOGT("%s @%d buffer length: %d\n", "Configuring FPGA...", sr, bufsize);
		if (perseus_set_sampling_rate(pi->descr, sr) < 0) {  // specify the sampling rate value in Samples/second
			LOGT("fpga configuration error: %s\n", perseus_errorstr());
		}	

		if ( (rc = perseus_start_async_input (pi->descr, bufsize, callback, this)) < 0) {
			LOGT("start async input error: %s\n", perseus_errorstr());
		}
	}
	return rc;
}

int PerseusRadio::stop ()
{
	if (pi->descr) {
		LOGT("%s\n", "Stopping async data acquisition...");
		return perseus_stop_async_input(pi->descr);
	} else
		return -1;
}

int PerseusRadio::set_sample_rate (int nsr)
{
	int rc = 0;
	
	// Configure the receiver for default MS/s operations
	LOGT("%s\n", "Configuring FPGA...");
	if ((rc = perseus_set_sampling_rate(pi->descr, nsr)) < 0) {  // specify the sampling rate value in Samples/second
		LOGT("fpga configuration error: %s\n", perseus_errorstr());
	} else 
		sr = nsr;
	return rc;

}

int PerseusRadio::get_sample_rate ()
{
	return sr;
}

int PerseusRadio::data_available (void *, int)
{
	return 0;
}

int PerseusRadio::set_frequency (int freq)
{
	return perseus_set_ddc_center_freq(pi->descr, f=freq, presel == true ? 1 : 0);
}

int PerseusRadio::get_frequency ()
{
	return f;
}


int PerseusRadio::set_attenuator(int att)
{
	return perseus_set_attenuator(pi->descr, att);
}

int PerseusRadio::set_preamp (int new_preamp)
{
	preamp = new_preamp;
	return perseus_set_adc(pi->descr, dither, preamp);
}

int PerseusRadio::set_dither (int new_dither)
{
	dither = new_dither;
	return perseus_set_adc(pi->descr, dither, preamp);
}

int PerseusRadio::set_preselector (int new_presel)
{
	presel = new_presel;
	return perseus_set_ddc_center_freq(pi->descr, f, presel == true ? 1 : 0);
}




PerseusRadio::~PerseusRadio ()
{
	LOGT("%s\n", "Destroying 1...");	
	if (pi) {
		if (status() && pi->descr) {
			LOGT("%s\n", "Destroying 2...");	
			perseus_close(pi->descr);
		}
		// crashes in case hardware is non connected - TBVerified
    	//perseus_exit();
	    LOGT("%s\n", "Destroying 3...");		
	}
}


int PerseusRadio :: callback(void *buf, int buf_size, void *obj)
{

	PerseusRadio *pobj = static_cast<PerseusRadio *> (obj);
	
	if (pobj) pobj->data_available (buf, buf_size);
	
	return 0;
}

	