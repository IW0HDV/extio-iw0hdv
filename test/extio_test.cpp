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
 * @file extio_test.cpp
 * @brief extio_test.cpp : Defines the exported functions for the DLL application.
 * @author Andrea Montefusco IW0HDV
 * @version 0.0
 * @date 2015-12-30
 */
 
#include "log.h"
#include "extio_test.h"


ExtIODll ExtIODll::singleton;


ExtIODll :: ExtIODll (): Extio(0) 
{
	// don't put any C runtime related operation here
	// as it is not going to be honored if and object of this class is
	// declared as static / singleton
	//fprintf (stderr, "%s\n", "ExtioDll Test module DEFAULT ctor"); fflush (stderr);
    Dll::Register (this);	
}
	
ExtIODll :: ~ExtIODll ()
{
	// don't put any C runtime related operation here
	// as it is not going to be honored if and object of this class is
	// declared as static / singleton
	//fprintf (stderr, "%s\n", "ExtioDll Test module dctor"); fflush (stderr);
}



bool ExtIODll::InitHW(char *name, char *model, int & extio_type)
{
	static bool first = true;
	EXTIO_BASE_TYPE extio_type_;

	extio_type =  extio_type_.value;

	extio_type_.dummy = true;
	
	if (first) {
		first = false;
	}

	strcpy(name, "TEST-IW0HDV");
	strcpy(model, "A" ); 
	return extio_type_.dummy;
}


bool ExtIODll::OpenHW(void)
{
	bool rc = true;
	
	return rc;	
}


void ExtIODll::CloseHW(void)
{
	// all the dynamically allocated objects are freed in smart pointers destructors
	
    LOGT("1Instance #%d\n", GetInstanceNumber());
	//LOG_CLOSE; is found in dllmain.cpp
    
    return;
}


int  ExtIODll::StartHW(long freq)
{
	LOGT("EXTIO_NS: %d  EXTIO_BASE_TYPE_SIZE: %d N:%d\n", EXTIO_NS , EXTIO_BASE_TYPE_SIZE , 2);
	return EXTIO_NS; // # of samples returned by callback
}


void ExtIODll::StopHW(void)
{
	if (GetInstanceNumber() == 1) {
		LOGT("%s\n", "Stopping async data acquisition...");
	}
	return;
}


int ExtIODll::SetHWLO(long freq)
{
	return 0;
}


long ExtIODll::GetHWLO(void)
{
	return 7070000;
}


long ExtIODll::GetHWSR(void)
{
	return 192000;
}

int ExtIODll::GetStatus(void)
{
	return 0;
}

void ExtIODll::ShowGUI(void)
{
	if ( GetInstanceNumber() == 1 ) {
		LOGT("%s\n", "ShowGUI");
	}
	return;
}

void ExtIODll::HideGUI(void)
{
	if ( GetInstanceNumber() == 1 ) {
		LOGT("%s\n", "HideGUI");
	}
	return;
}





