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

#if !defined	__EXTIO_PERSEUS_H__
#define			__EXTIO_PERSEUS_H__

#include "extio_config.h"

/*
 * class ExtioDataConversion
 *
 * helper class for buffering and conversion of Perseus samples to Extio formats 
 *
 */
template < typename ST >
class ExtioDataConversion {
public:
	ExtioDataConversion (int _ns):ns(_ns) { pb = new unsigned char[2 * ns * EXTIO_BASE_TYPE_SIZE ]; }
	~ExtioDataConversion() { delete [] pb;  }
	int getNs() { return ns; }

	unsigned char *pb;
	int ns;

	int convert_iq_from_rx (PerseusRxIQSample *i, PerseusRxIQSample *q, ExtIO_int24)
	{
		int24 *p = (int24 *)pb;
		for (int n = 0; n < ns; ++n) {
			p->v1 = i->s3, p->v2 = i->s2, p->v3 = i->s1; ++p;
			p->v1 = q->s3, p->v2 = q->s2, p->v3 = q->s1; ++p;
			++i, ++q;
		}
		return 0;
	}

	int convert_iq_from_rx (PerseusRxIQSample *i, PerseusRxIQSample *q, ExtIO_int32)
	{
		int *p = (int *)pb;
		for (int n = 0; n < ns; ++n)
			*p++ = i->int_32(),
			*p++ = q->int_32(),
			++i,
			++q;
		return 0;
	}

	int convert_iq_from_rx (PerseusRxIQSample *i, PerseusRxIQSample *q, ExtIO_float32)
	{
		float *p = (float *)pb;
		for (int n = 0; n < ns; ++n)
			*p++ = i->float_32(),
			*p++ = q->float_32(),
			++i,
			++q;
		return 0;
	}

	int convert_iq_from_rx(PerseusRxIQSample *i, PerseusRxIQSample *q, ExtIO_int_hpsdr)
	{
		int24_hpsdr *p = (int24_hpsdr *)pb;
		for (int n = 0; n < ns; ++n) {
			p->v1 = i->s3, p->v2 = i->s2, p->v3 = i->s1; ++p;
			p->v1 = q->s3, p->v2 = q->s2, p->v3 = q->s1; ++p;
			++i, ++q;
		}
		return 0;
	}

};


#include <stdint.h>
#include "log.h"
#include "perseus.h"

template <typename ST>
class ExtioPerseusRadio2 : public ExtioDataConversion<ST>, public PerseusRadio
{
public:
	ExtioPerseusRadio2(int ns, EXTIO_RX_CALLBACK *pCb): 
		ExtioDataConversion<ST>(ns), 
		PerseusRadio (),
		cnt(0), 
		pExtioCallback(pCb),
		ns_(ns)
	{  }

	virtual ~ExtioPerseusRadio2() {}


	// called from the receiving context when the buffer is full
	int send_iq_from_rx_to_dsp (PerseusRxIQSample *i, PerseusRxIQSample *q, int ns)
	{
		// convert and copy raw I/Q data from two buffers in native Perseus format to ExtIO format
		this->convert_iq_from_rx(i, q, ST()); // Extio type selection is done at compile type !

		// housekeeping: increase internal buffers counter
		if ((cnt % 1024) == 0) { LOGT("---------- buflen: %d\n", ns); cnt++; }

		// send data to client SDR program
		(*pExtioCallback) (ns, 0, 0., (int *) this->pb );

		cnt++; // housekeeping
		return 0;
	}
	
	//
	// called from libperseus-sdr data callback
	// this method is declared in PerseusRadio class
	// The buffer received from hardware contains 24-bit IQ samples (6 bytes per sample)
	// We save the received IQ samples as 24 bit (msb aligned) integer IQ samples.
	//
	int data_available (void *buf, int buf_size)
	{	
		uint8_t	*sbuf	= (uint8_t*)buf;
		int nSamples	= buf_size/(3*2); // 3*2 = 6 bytes for sample, as per Perseus USB data stream format

		// Transform data from linear interleaved native Perseus hardware USB format
		// to two buffers, one for I and the other one for Q
		PerseusRxIQSample i_buf [ns_]; // * sizeof(typename ST::sample_type)];
		PerseusRxIQSample q_buf [ns_]; // * sizeof(typename ST::sample_type)];
		
		for (int k=0; k < nSamples; k++) {
			i_buf[k].s3 = *sbuf++ , i_buf[k].s2 = *sbuf++ , i_buf[k].s1 = *sbuf++ ;
			q_buf[k].s3 = *sbuf++ , q_buf[k].s2 = *sbuf++ , q_buf[k].s1 = *sbuf++ ;
		}
		// send to client SDR program
		this->send_iq_from_rx_to_dsp (i_buf, q_buf, nSamples);
	
		return 0;
	}
	/*
	 * method used in order to force a sample rate change, usually as
	 * per user input on master program GUI
	 */
	void setSampleRateHW(int new_sr)
	{
		this->set_sample_rate(new_sr);

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
		}
	
	}

protected:
	int cnt;
	EXTIO_RX_CALLBACK *pExtioCallback;
	int ns_;
};

#include <memory>
template <class ST> using PEXTPRADIO = std::shared_ptr < ExtioPerseusRadio2<ST> > ;

#include "gui_splashscreen.h"
#include "gui_control.h"

class ExtIODll : public Extio {

public:

	void ProcessAttach() {}
	void ProcessDetach() 
	{ 
		CloseHW();  // force radio hardware close 
	}
	void ThreadAttach() {}
	void ThreadDetach() {}
	
	virtual bool InitHW(char *name, char *model, int& extio_type);
	virtual bool OpenHW(void);
	virtual void CloseHW(void);
	
	virtual int  StartHW(long freq);
	virtual void StopHW(void);
	
	virtual int SetHWLO(long LOfreq);
	virtual long GetHWLO(void);
	
	virtual long GetHWSR(void);
	virtual int GetStatus(void);
	
	virtual void ShowGUI(void);
	virtual void HideGUI(void);
	
	virtual const char *name() { return "PERSEUS-I0HDV"; }

	// main radio object pointer
    PEXTPRADIO<EXTIO_BASE_TYPE> pExr;
	
	// GUI pointers
	PSPLASH  pSplash;
	PCTRLGUI pGui;
    
	// copy ctor and assignment op deleted: this is a singleton global object
    ExtIODll (const ExtIODll &) = delete;
    ExtIODll & operator=(const ExtIODll &) = delete ;

private:
    // static object that guarantees one (and one only) instance is generated at the global
	// level, so that C++ runtime builds it , calling the empty c'tor
	// the trick here is that the following static member is an object of this _same_ class
	// so it has access to c'tor even if it is kept private
	// see http://accu.org/index.php/journals/1328 by Alexander Nasonov
	//
    static ExtIODll singleton;

	ExtIODll ();
	~ExtIODll ();

};


#endif
