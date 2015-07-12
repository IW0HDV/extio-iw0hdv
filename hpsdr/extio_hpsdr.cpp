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
 * @file extio_hpsdr.cpp
 * @brief extio_hpsdr.cpp : Defines the exported functions for the DLL application.
 * @author Andrea Montefusco IW0HDV
 * @version 0.0
 * @date 2014-12-06
 */


#include "stdio.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <algorithm>    // for std::min std::max macros

#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment (lib, "ws2_32.lib")
// Link with Iphlpapi.lib
#pragma comment(lib, "IPHLPAPI.lib")

#if defined _MSC_VER
#include <strsafe.h>
#endif

#include "util.h"
#include "log.h"

#include "hpsdr.h"
#include "extio.h"
#include "extio_config.h"
#include "extio_hpsdr.h"

#include "guiutil.h"
#include "gui.h"
#include "gui_control.h"
#include "gui_splashscreen.h"



template < typename ST >
GuiHpsdr * ExtioMercuryRadio<ST>::CreateGui(int sr)
{
	return new MercuryGui(sr);
}

template < typename ST >
GuiHpsdr * ExtioHermesRadio<ST>::CreateGui(int sr)
{
	return new HermesGui(sr);
}


void ExtioEthernet::FatalError(const char *pMsg) 
{ 
	LOGT("%s\n", "**** FATAL ERROR *****");

	if (peio_->pGui) {
		peio_->pGui->setHw(pMsg);
	} else {
		LOGT("%s\n", "NO GUI");
	}
};

void ExtioEthernet::TransmissionTmo(const char *pMsg) 
{ 
	LOGT("%s: [%s]\n", "TIMEOUT", pMsg);

	if (peio_) {
		if (peio_->pGui) {
			peio_->pGui->setHw(pMsg);
			peio_->pGui->appendMessage(pMsg);
			peio_->pGui->appendMessage("\r\n");
		}
		//peio_->signalStartReleased ();
	}
};

template <typename ST>
void ExtioHpsdrRadio<ST> :: setSampleRateHW(int new_sr)
{
	pR_->setSampleRate(new_sr);

	/* 100
		This status value indicates that a sampling frequency change has taken place,
		either by a hardware action, or by an interaction of the user with the DLL GUI.
		When the main program receives this status, it calls immediately after
		the GetHWSR() API to know the new sampling rate.
		We are calling the callback only for the first instance (otherwise Studio 1 is looping on Start/Stop cycle - TBI).
	*/
	if (*pExtioCallback && (Dll::GetInstanceNumber()==1)) {
		LOGT("new sample rate: %d\n", new_sr);
		(*pExtioCallback) (-1, 100, 0., 0);
		pCr_->SendOtherInstancesNewSampleRate (new_sr);
	}
}


//
// Globals
//

ExtIODll singleton;

ExtIODll::ExtIODll(): Extio(0), pR(0), pExr(0), pGui(0), pExtioEth(0), pCmdRec(0), rxIQ(0) 
{
	fprintf (stderr, "%s\n", "ExtioDll HPSDR DEFAULT ctor");
    Dll::Register (this);	
}

#pragma data_seg (".SS_EXTIO_HERMES")

// !!!! have to be initialized vars, due to shared segments rules constraints
unsigned char bufHR[ MAX(sizeof(ExtioMercuryRadio < EXTIO_SAMPLE_TYPE >), sizeof(ExtioHermesRadio < EXTIO_SAMPLE_TYPE >)) ] SHARED = { 0 };
char bufHE [sizeof(ExtioEthernet)]	SHARED = { 0 };

#pragma data_seg ()


//
// Radio Factory Helper
//
template < typename EXTIO_SAMPLE_TYPE >
ExtioHpsdrRadio<EXTIO_SAMPLE_TYPE> *
CreateExtioHpsdrRadio (const char *board_id, CommandReceiver *pCr, ExtIODll *pEio)
{
	// explicitly call CreateExtioHpsdrRadio<EXTIO_SAMPLE_TYPE> in order to force to generate code in case
	// the compiler optimizations are used; otherwise a link error is generated in modules that don't see the definition
	static ExtioHpsdrRadio<EXTIO_SAMPLE_TYPE> *p = CreateExtioHpsdrRadio<EXTIO_SAMPLE_TYPE> ((const char *)0,(CommandReceiver *)0,(ExtIODll *)0);

	if (board_id == 0) {
		p=0;return 0;
	}
	RadioFactory<EXTIO_SAMPLE_TYPE> rf;

	return rf.Create(board_id, bufHR, /* &ExtioCallback */ &(pEio->extioCallback), pCr);
}

//
// discovery thread helper
//
pthread_t scan_thread_id;

void *scan_dev_thread(void *p)
{
	HpsdrSplash *pSplash = (HpsdrSplash *)p;

	LOGT("%s\n", "Starting radio scan");

	Ethernet::scan_devices (pSplash);

	return 0;
}

////////////////////////////////////////////////////////////////
//
// virtual methods redefined
//

// when the process attach the DLL, initialize the Windows Sockets API (WSA)
void ExtIODll::ProcessAttach()
{
	static struct {
		int err;
		const char *err_str;
	} wsa_err [] = {
		{ 0, "WSA successfully started"},
		{ WSASYSNOTREADY, "ERROR: The underlying network subsystem is not ready for network communication."},
		{ WSAVERNOTSUPPORTED, "ERROR: The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation."},
		{ WSAEINPROGRESS, "ERROR: A blocking Windows Sockets 1.1 operation is in progress."},
		{ WSAEPROCLIM, "ERROR: A limit on the number of tasks supported by the Windows Sockets implementation has been reached."},
		{ WSAEFAULT, "ERROR: The lpWSAData parameter is not a valid pointer."},
	};

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	LOG_OPEN("hpsdr", GetInstanceNumber());

	// initialize Windows sockets
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
	for (unsigned i=0; i< ARRAY_SIZE(wsa_err); ++i)
		if (err == wsa_err[i].err) {
			LOGT("WSAStartup: %d (%s)\n", err, wsa_err[i].err_str);
			fprintf (stderr, "WSAStartup: %d (%s)\n", err, wsa_err[i].err_str);
		}
};

// on detaching, close the radio hardware (it should be done form the client DSP program)
// and close (cleanup) the Windows Sockets API (WSA)
void ExtIODll::ProcessDetach() 
{
	CloseHW(); 
	WSACleanup();
};

bool ExtIODll::InitHW(char *name, char *model, int & extio_type)
{
	LOGT ("Instance #%d\n", GetInstanceNumber() ) ;

	static bool first = true;
	EXTIO_SAMPLE_TYPE extio_type_;

	extio_type = extio_type_.value;
	extio_type_.dummy = true;
	
	if (first) {
		first = false;
		if (GetInstanceNumber() == 1) {
			// on the first instance, opens the splash screen and start the 
			// scan thread in background
			// so the parent DSP program can go ahead, whilst the search 
			// of active HPSDR hardware is done
			
			if (pCmdRec == 0) pCmdRec = new CommandReceiver(this);

			std::shared_ptr<HpsdrSplash> p(new HpsdrSplash(&pGui, &pCmdRec, this) );
			pSplash = p;

			int rc = ::pthread_create(&scan_thread_id, NULL, scan_dev_thread, (void *)pSplash.get());
			if (rc != 0) {
				LOGT("pthread_create failed on scan device thread: rc=%d\n", rc);
				return 0;
			}
			else {
				LOGT("%s\n", "scan device thread: thread succcessfully started");
			}
		}
	}
	strcpy(name, "HPSDR");
	strcpy(model, "unknown" );   // at this point scan is undergoing, so no answers available
	return extio_type_.dummy;
}

bool ExtIODll::OpenHW()
{
	LOGT("Instance #%d\n", GetInstanceNumber());
	//
	// really we don't open any hardware here
	// only the Splash panel is shown to the user so she/he has a chance 
	// to select the right hardware to be used
	//
	if (GetInstanceNumber() == 1) {
		if (pSplash) pSplash->Show();
		if (pGui) pGui->Show();
	} else {
		// create command receiver for instances > 1
		if (pCmdRec == 0) pCmdRec = new CommandReceiver(this);
	}
	return true;
}

int ExtIODll::StartHW(long LOfrequency)
{
	LOGT("Instance #%d LOfreq: %d\n", GetInstanceNumber(), LOfrequency);
	Ethernet::Device *pDev = 0;

	if (GetInstanceNumber() == 1) {

		if (!Ethernet::found()) {
			// signals to user that no proper hardware has been found
			GuiError x("No hardware found, unable to start receiver !");
			if (pSplash) pSplash->Hide();
			x.show();
			if (pSplash) pSplash->SetStatus("%s", (const char*)x), pSplash->Show();
			// return 0 is an error to the main program, hence the DSP processing is not started at all
			return 0;
		} else {
			pSplash->Hide();
		}

		if ((pDev = Ethernet::found (pSplash->GetSel()) )) {

			if (pR == 0) {

				if (pGui) {
					// Gui and Radio already created in the Splash screen
					// only globals to be initialized
					pExr = pGui->getRadio();
					pR = pExr->getRadio();
				} else {
					// Create radio according to type discovered
					pExr = CreateExtioHpsdrRadio<EXTIO_SAMPLE_TYPE>(pDev->board_id, pCmdRec, this);

					if (!pExr) {
						GuiError("Hardware unsupported, unable to start receiver !").show();
						return 0;
					} else {
						pR = pExr->getRadio(); // global pointer to Radio

						// create Gui and setup it
						pGui = pExr->CreateGui(EXTIO_DEFAULT_SAMPLE_RATE);
						pGui->setRadio(pExr);

						// explicitly call setSampleRateHW in order to force to generate code in case
						// the compiler optimizations are used; otherwise a link error is generated
						// in modules that don't see the definition
						pExr->setSampleRateHW (0);
					}
				}

				// The following call is really needed, in order to setup the samplerate
				{
					int new_sr;
					pR->getSampleRate( new_sr );
					LOGT("Instance #%d radio sample rate: %d\n", GetInstanceNumber(), new_sr);
					pExr->setSampleRateHW ( new_sr );
				}
				// create an Hpsdr flow object and assign to the Ethernet object
				Flow *pFlow = new Flow(pR);
				pExtioEth = new ExtioEthernet(pFlow, this);
			}

			pGui->setHwAddressGUI(pDev); // set the device pointer to gui
			pGui->Show();				 // and shows it
		}
		// here we have the receiver object created (pR != 0) 
		int act_sr;
		pR->getSampleRate(act_sr);
		LOGT("StartHW() before starting receive: SAMPLE RATE; %d #rx: %d\n", act_sr, pGui->getRecNumber());

		pR->setNumberOfRx(pGui->getRecNumber());// before the real receivers are started, 
												// we have to select how many receivers are to be used

		pExtioEth->startReceive(pExtioEth->found()); // finally, start the receiver(s)

		pR->setFrequency(LOfrequency); // establish the frequency

	} else { // for instances > 1
		pExtioEth = (ExtioEthernet *)bufHE;
		pDev = pExtioEth->found();
		
		// point local hermes radio object pointer to shared buffer
		RadioFactory<EXTIO_SAMPLE_TYPE> rf;
		pExr = rf.Pointer (pDev->board_id, bufHR, pCmdRec);
		if (pExr) pR = pExr->getRadio();
	}

	//
	// all instances, even the first one, have to prepare to receive data from the main one
	//
	rxIQ = new IdllComm < EXTIO_SAMPLE_TYPE, EXTIO_NS >(GetInstanceNumber() - 1, &extioCallback);

	if (rxIQ) {
		rxIQ->startReceive();
		LOGT("%s\n", "OK: rxIQ created !");
	} else {
		LOGT("%s\n", "FATAL: rxIQ not created !");
	}

	if (GetInstanceNumber() == 1) {
		// in first instance setup the internal data sender
		pExr->setIdllComm(new IntraComm());
		pGui->EnableControls();
	} else {

		// sanity checks
		//
		// first instance is:  #1
		// second instance is: #2
		// third instance is:  #3
		//....
		if (pR) {
			if (GetInstanceNumber() > pR->getNumberOfRx()) {
				GuiError("Too many instances started, unable to start receiver !").show();
				return 0;
			}
		} else {
			GuiError("Fatal error, no radio object instantiated !").show();
			return 0;
		}
	}
	if (pCmdRec && (GetInstanceNumber() == 1)) {
		LOGT("Sending start to other instances......%s\n", "-");
		pCmdRec->SendOtherInstancesStart();
	}

	return EXTIO_NS; // # of samples returned by callback
}

int ExtIODll::GetStatus()
{
	LOGT("Instance #%d\n", GetInstanceNumber());

	return 0;
}

void ExtIODll::StopHW()
{
	LOGT("Instance #%d\n", GetInstanceNumber());

	if (GetInstanceNumber() == 1) {
		pGui->appendMessage ("Hardware stopped by user request.\n");
		pExtioEth->stopReceive ();
		pGui->setHwAddressGUI(Ethernet::found(pSplash->GetSel()));
		if (pCmdRec && (GetInstanceNumber() == 1)) pCmdRec->SendOtherInstancesStop();
	}
	return;
}

void ExtIODll::CloseHW()
{
	LOGT("Instance #%d\n", GetInstanceNumber());
	if (pCmdRec && (GetInstanceNumber() == 1)) pCmdRec->SendOtherInstancesClose();
	if ( GetInstanceNumber() == 1 ) delete pGui, pGui=0 ; //, delete pSplash, pSplash=0;
	if (pCmdRec) delete pCmdRec, pCmdRec = 0;
	//LOG_CLOSE; moved in dllmain.cpp
	return;
}

int ExtIODll::SetHWLO(long freq)
{
	LOGT("Instance #%d freq: %d (Radio *: 0x%p)\n", GetInstanceNumber(), freq, pR);
	
	if (freq < 10000) return -10000;
	if (freq > 60000000) return 60000000;
	// check the radio pointer is valid
	// Studio1 calls this entry before the StartHW is sent
	if (pR) pR->setFrequency ( freq, GetInstanceNumber() - 1 ) ;
	//if (pCmdRec && (GetInstanceNumber() == 1)) pCmdRec->SendOtherInstancesHWLO(freq);
	return 0;
}

long ExtIODll::GetHWLO()
{
	LOGT("Instance #%d\n", GetInstanceNumber());
	long LOfreq;
	if (pR) pR->getFrequency(LOfreq, GetInstanceNumber() - 1);
	LOGT("   return LOfreq: %d\n", LOfreq);
	return LOfreq;
}

long ExtIODll::GetHWSR()
{
	LOGT("Instance #%d\n", GetInstanceNumber());
	int sr = EXTIO_DEFAULT_SAMPLE_RATE;
	if (pR) pR->getSampleRate(sr);
	LOGT("   return: %d\n", sr);
	return sr;
}

void ExtIODll::ShowGUI()
{
	LOGT("Instance #%d\n", GetInstanceNumber());

	if ( GetInstanceNumber() == 1 ) {
		if (pSplash) pSplash->Show();
		if (pGui) pGui->Show ();
	}
	return;
}

void ExtIODll::HideGUI()
{
	LOGT("Instance #%d\n", GetInstanceNumber());

	if ( GetInstanceNumber() == 1 ) {
		if (pGui) pGui->Hide ();
		if (pSplash) pSplash->Hide();
	}
	return;
}

#if 0 // not currently used not needed with HPSDR hardware
extern "C"
EXTIO_HERMES_API void __stdcall IFLimitsChanged (long low, long high)
{
	LOG (("IFLimitsChanged() called\n"));
	
	return;
}

extern "C"
EXTIO_HERMES_API void __stdcall TuneChanged (long freq)
{
	LOG (("TuneChanged() called with freq: %d\n", freq));
	
    return;
}


extern "C"
EXTIO_HERMES_API void __stdcall RawDataReady(long samprate, int *Ldata, int *Rdata, int numsamples)
{
	return;
}

#endif


