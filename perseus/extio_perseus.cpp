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

 /** 
 * @file extio_perseus.cpp
 * @brief extio_perseus.cpp : Defines the exported functions for the DLL application.
 * @author Andrea Montefusco IW0HDV
 * @version 0.0
 * @date 2013-09-23
 */

#include "log.h"
#include "extio_perseus.h"
#include "gui_splashscreen.h"
#include "gui_control.h"


//
// the only object of ExtIODll class
//
ExtIODll ExtIODll::singleton;


ExtIODll::ExtIODll () : Extio(0)
{
	fprintf (stderr, "%s\n", "ExtioDll Perseus DEFAULT ctor");
	// register this instance to the Dll base class
    Dll::Register (this);	
}

ExtIODll :: ~ExtIODll ()
{
	fprintf (stderr, "%s\n", "ExtioDll Perseus dctor");
}


bool ExtIODll::InitHW(char *name, char *model, int & extio_type)
{
	static bool first = true;
	EXTIO_BASE_TYPE extio_type_;

	extio_type =  extio_type_.value;

	extio_type_.dummy = true;
	
	if (first) {
		first = false;

		pSplash.reset( new PerseusSplash(0, 0) );

		pExr.reset ( new ExtioPerseusRadio2<EXTIO_BASE_TYPE> (EXTIO_NS, &extioCallback ) );
		
		if  (pExr == 0 || !pExr->status()) {
			pSplash->SetStatus("Unable to find receiver ! [%s]", pExr ? pExr->last_error() : "");
			pSplash->Show();
			return false;
		}
	}

	strcpy(name, "Perseus");
	strcpy(model, "The_only_done_ever" ); 
	return extio_type_.dummy;
}

bool ExtIODll::OpenHW(void)
{
	bool rc = false;
	
	if  (pExr && pExr->status()) {
		if (pExr->open() == 0) {
			// Create the GUI 
			pGui.reset ( new PerseusCtrlGui (pExr) );
			if (pGui) {
				// hide and destroy splash screen
			    pSplash->Hide();
			    pSplash = nullptr;
				// show the GUI
				pGui->Show ();
			    rc = true;
			} else {
				pSplash->SetStatus("%s", "Unable to create GUI !");
				pSplash->Show();
			}
		} else {
			pSplash->SetStatus("Unable to open receiver ! [%s]", pExr->last_error() );
			pSplash->Show();
		}
	} else {
		pSplash->Show();
	}
	
	return rc;
}



//static volatile int closing = 0;


void ExtIODll::CloseHW(void)
{
	//LOG_CLOSE; is found in dllmain.cpp
	// all the dynamically allocated objects are freed in smart pointers destructors
    return;
}

int  ExtIODll::StartHW(long freq)
{
	LOGT("EXTIO_NS: %d  EXTIO_BASE_TYPE_SIZE: %d N:%d\n", EXTIO_NS , EXTIO_BASE_TYPE_SIZE , 2);

    // take care: this is the libperseus-sdr buffer length
	if (pExr) pExr->start(EXTIO_NS * sizeof(PerseusRxIQSample) * 2);

	return EXTIO_NS; // # of samples returned by callback
}


void ExtIODll::StopHW(void)
{
	if (GetInstanceNumber() == 1) {
		LOGT("%s\n", "Stopping async data acquisition...");
		if (pExr) pExr->stop();
	}
	return;
}


int ExtIODll::SetHWLO(long freq)
{
	if (freq < 10000) return -10000;
	if (freq > 60000000) return 60000000;
	
	if (pExr) pExr->set_frequency (freq);

	return 0;
}


long ExtIODll::GetHWLO(void)
{
	if (pExr) return pExr->get_frequency();
		else  return 0;
}


long ExtIODll::GetHWSR(void)
{
	if (pExr) {
		LOGT("returned sr: %d\n", pExr->get_sample_rate() );
		return pExr->get_sample_rate();
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



