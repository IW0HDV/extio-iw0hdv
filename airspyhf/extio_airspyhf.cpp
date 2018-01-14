/*
 *  IW0HDV Extio
 *
 *  Copyright 2015 by Andrea Montefusco IW0HDV
 *
 *  Licensed under GNU General Public License 3.0 or later. 
 *  Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 */

 /** 
 * @file extio_airspyhf.cpp
 * @brief extio_airspyhf.cpp : Defines the exported functions for the DLL application.
 * @author Andrea Montefusco IW0HDV
 * @version 0.0
 * @date 2017-07-20
 */
 
#include <stdlib.h>     /* getenv */

#include "log.h"
#include "extio_airspyhf.h"
#include "gui_splashscreen.h"
#include "gui_control.h"


ExtIODll ExtIODll::singleton;


ExtIODll :: ExtIODll (): Extio(0) 
{
	//fprintf (stderr, "%s\n", "ExtioDll AirSpyHf DEFAULT ctor");
    Dll::Register (this);	
}
	
ExtIODll :: ~ExtIODll ()
{
	//fprintf (stderr, "%s\n", "ExtioDll AirspyHf dctor");
}



bool ExtIODll::InitHW(char *name, char *model, int & extio_type)
{
	static bool first = true;
	EXTIO_BASE_TYPE extio_type_;

	extio_type =  extio_type_.value;

	extio_type_.dummy = true;
	/*
	 * The serial number of hardware to be used can be
	 * specified on the command line
	 * using the environment variable AIRSPYHF.
	 * Please note that there are no space between
	 * the last char of serial number and the double ampersand
	 *
	 * set AIRSPYHF=3B528E80D5D23FEF&& HDSDR.exe
	 *
	 */
	char *sn = getenv("AIRSPYHF");
	if (sn) {
		LOGT("ENV: %s\n", sn);
	}
	
	if (first) {
		first = false;
		
		pSplash.reset ( new AirSpyHfSplash(0, 0) );

		pExr.reset ( new ExtioAirSpyHfRadio<EXTIO_BASE_TYPE> (EXTIO_NS, &extioCallback, sn ) );

		if  (pExr == 0 || !pExr->status()) {
			pSplash->SetStatus("Unable to find receiver ! [%s]", pExr ? pExr->last_error() : "");
			pSplash->Show();
			LOGT("%s%s\n", "Error creating instance: ", pExr ? pExr->last_error() : "");
			return false;
		}
	}

	strcpy(name, this->name());
	strcpy(model, "AirSpyHF+" );
	return extio_type_.dummy;
}


bool ExtIODll::OpenHW(void)
{
	bool rc = false;
	
	if  (pExr && pExr->status()) {
		if (pExr->open() == 0) {
			pSplash->Hide(); pSplash = nullptr;

			pGui.reset ( new AirSpyHfCtrlGui (pExr) );
			if (pGui) pGui->Show ();
			rc = true;
		} else {
			pSplash->SetStatus("Unable to open receiver ! [%s]", pExr->last_error() );
			pSplash->Show();
		}
	} else {
		pSplash->Show();
	}
	
	return rc;	
}


void ExtIODll::CloseHW(void)
{
	// all the dynamically allocated objects are freed in smart pointers destructors
	
    LOGT("1Instance #%d %p %p %p\n", GetInstanceNumber(), pExr.get(), pSplash.get(), pGui.get());
    pSplash = nullptr;
    
    LOGT("2Instance #%d %p %p %p\n", GetInstanceNumber(), pExr.get(), pSplash.get(), pGui.get());
    pGui = nullptr;
	
    LOGT("3Instance #%d %p %p %p\n", GetInstanceNumber(), pExr.get(), pSplash.get(), pGui.get());
    pExr = nullptr;

	//LOG_CLOSE; is found in dllmain.cpp
    
    return;
}


int  ExtIODll::StartHW(long freq)
{
	LOGT("EXTIO_NS: %d  EXTIO_BASE_TYPE_SIZE: %d N:%d\n", EXTIO_NS , EXTIO_BASE_TYPE_SIZE , 2);

	if (pExr) pExr->startHW(EXTIO_NS * EXTIO_BASE_TYPE_SIZE * 2);

	if (pGui) pGui->DisableControls ();

	return EXTIO_NS; // # of samples returned by callback
}


void ExtIODll::StopHW(void)
{
	if (GetInstanceNumber() == 1) {
		LOGT("%s\n", "Stopping async data acquisition...");
		if (pExr) pExr->stop();
	}
	if (pGui) pGui->EnableControls ();

	return;
}


int ExtIODll::SetHWLO(long freq)
{
/*	
		This entry point is used by Winrad to communicate and control the desired frequency 
		of the external HW via the DLL. The frequency is expressed in units of Hz. 
		The entry point is called at each change (done by any means) of the LO value 
		in the Winrad main screen.

		frequency limits as found in preliminary AirSpyHf 
		specifications (July 2017): http://airspy.com/airspy-hf-plus/
		
		HF coverage between DC .. 31 MHz
		VHF coverage between 60 .. 260 MHz
 */

	if (freq < 0) return -1; /* <0 (a negative number N) The specified frequency is lower  than the
	                            minimum that the  hardware is  capable to  generate.
															The  absolute value of N indicates what is the minimum supported
															by the HW. */
	if (freq > 31000000) {
		if (freq < 60000000) return 31000000;
		if (freq > 260000000) return 260000000;
	} 
	
	if (pExr) pExr->set_frequency (freq);

	return 0;
}


long ExtIODll::GetHWLO(void)
{
	if (pExr) return pExr->get_frequency();
	return 0;
}


long ExtIODll::GetHWSR(void)
{
	int sr;
	if (pExr) {
		sr = pExr->get_sample_rate();
		LOGT("returned sr: %d\n", sr);
		return sr;
	} else
		return 0;
}

int ExtIODll::GetStatus(void)
{
	return 0;
}

void ExtIODll::ShowGUI(void)
{
	if ( GetInstanceNumber() == 1 ) {
		if (pSplash) pSplash->Show();
		if (pGui) pGui->Show ();
	}
	return;
}

void ExtIODll::HideGUI(void)
{
	if ( GetInstanceNumber() == 1 ) {
		if (pGui) pGui->Hide ();
		if (pSplash) pSplash->Hide();
	}
	return;
}
