/*
 * Extio for Airspy
 *
 * Copyright 2015 by Andrea Montefusco IW0HDV
 *
 * Licensed under GNU General Public License 3.0 or later. 
 * Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 *
 */

#include <typeinfo>
#include <iostream>
#include "dllmain.h"
#include "extio.h"
#include "log.h"

#ifdef _WIN32

	#define EXTIO_EXPORTS

	/* You should define EXTIO_EXPORTS *only* when building the DLL. */
	#ifdef EXTIO_EXPORTS
		#define EXTIO_API __declspec(dllexport)
	#else
		#define EXTIO_API __declspec(dllimport)
	#endif
	
	/* Define calling convention in one place, for convenience. */
	#define EXTIO_CALL __stdcall
	
#else /* _WIN32 not defined. */
	
	/* Define with no value on non-Windows OSes. */
	#define EXTIO_API
	#define EXTIO_CALL
	
#endif

Extio::Extio (HMODULE h): Dll(h), extioCallback(0) 
{
	//LOGT("%s\n", "ctor");
}

// static member of Extio base class
Extio * Extio::get()  
{ 
	try {
		return dynamic_cast<Extio *> (Dll::getObj()); 
	}
	catch(const std::bad_cast & e) {
        LOGT("INTERNAL ERROR: %s\n", e.what());
    }
	return 0;
}

extern "C"
EXTIO_API bool EXTIO_CALL InitHW(char *name, char *model, int & extio_type)
{
	LOGT ("Instance #%d Dll: %p\n", Dll::GetInstanceNumber(), Extio::get() ) ;
	if (Extio::get()) return (Extio::get())->InitHW (name, model, extio_type);
		else return false;
}

extern "C"
EXTIO_API bool __stdcall OpenHW()
{
	LOGT("Instance #%d\n", Dll::GetInstanceNumber());
	if (Extio::get()) return (Extio::get())->OpenHW();
		else return false;
}

extern "C"
EXTIO_API int __stdcall StartHW(long LOfrequency)
{
	LOGT("Instance #%d LOfreq: %d\n", Dll::GetInstanceNumber(), LOfrequency);
	if (Extio::get()) return (Extio::get())->StartHW(LOfrequency); // # of samples returned by callback
	return 0;
}

extern "C"
EXTIO_API int __stdcall GetStatus()
{
	LOGT("Instance #%d\n", Dll::GetInstanceNumber());
	if (Extio::get()) return (Extio::get())->GetStatus();
	return 0;
}

extern "C"
EXTIO_API void __stdcall StopHW()
{
	LOGT("Instance #%d\n", Dll::GetInstanceNumber());
	if (Extio::get()) return (Extio::get())->StopHW();
	return;
}

extern "C"
EXTIO_API void __stdcall CloseHW()
{
	if (Extio::get()) return (Extio::get())->CloseHW();
}

extern "C"
EXTIO_API int __stdcall SetHWLO(long freq)
{
	LOGT("Instance #%d freq: %d\n", Dll::GetInstanceNumber(), freq);
	if (Extio::get()) return (Extio::get())->SetHWLO(freq);
		else return -1;
}

extern "C"
EXTIO_API long __stdcall GetHWLO()
{
	LOGT("Instance #%d\n", Dll::GetInstanceNumber());
	if (Extio::get()) return (Extio::get())->GetHWLO();
		else return 0;
}

extern "C"
EXTIO_API long __stdcall GetHWSR()
{
	LOGT("Instance #%d\n", Dll::GetInstanceNumber());
	if (Extio::get()) return (Extio::get())->GetHWSR();
		else return 0;
}

extern "C"
EXTIO_API void __stdcall SetCallback (EXTIO_RX_CALLBACK parentCallBack)
{
	LOGT("Instance #%d\n", Dll::GetInstanceNumber());
	if (Extio::get()) (Extio::get())->extioCallback = parentCallBack;
}

extern "C"
EXTIO_API void __stdcall ShowGUI()
{
	LOGT("Instance #%d\n", Dll::GetInstanceNumber());
	if (Extio::get()) (Extio::get())->ShowGUI();
}

extern "C"
EXTIO_API void __stdcall HideGUI()
{
	LOGT("Instance #%d\n", Dll::GetInstanceNumber());
	if (Extio::get()) (Extio::get())->HideGUI();
	return;
}


#if 0 // not currently used not needed with HPSDR hardware
extern "C"
EXTIO_API void __stdcall IFLimitsChanged (long low, long high)
{
	LOG (("IFLimitsChanged() called\n"));
	
	return;
}

extern "C"
EXTIO_API void __stdcall TuneChanged (long freq)
{
	LOG (("TuneChanged() called with freq: %d\n", freq));
	
    return;
}


extern "C"
EXTIO_API void __stdcall RawDataReady(long samprate, int *Ldata, int *Rdata, int numsamples)
{
	return;
}

#endif
