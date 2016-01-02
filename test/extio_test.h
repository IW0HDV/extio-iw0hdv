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

 #if !defined	__EXTIO_AIRSPY_H__
#define			__EXTIO_AIRSPY_H__

#include "extio_config.h"

/*
 * class ExtioDataConversion
 *
 * helper class for buffering and conversion of AIRSPY samples to Extio formats 
 *
 */
 
 
template < typename ST >
class ExtioDataConversion {
public:
	ExtioDataConversion (int _ns):ns(_ns) { pb = new unsigned char[2 * ns * sizeof(typename ST::sample_type)]; }
	~ExtioDataConversion() { delete [] pb;  }
	int getNs() { return ns; }

	unsigned char *pb;
	int ns;

	#if 0
	int convert_iq_from_rx (AirSpyRxIQSample *i, AirSpyRxIQSample *q, ExtIO_float32)
	{
		float *p = (float *)pb;
		for (int n = 0; n < ns; ++n)
			*p++ = i->float_32(),
			*p++ = q->float_32(),
			++i,
			++q;
		return 0;
	}
	#endif
};



class ExtIODll : public Extio {
public:
	
	void ProcessAttach() 
	{ 
		LOG_OPEN("test", GetInstanceNumber());
	}
	void ProcessDetach() 
	{ 
		LOGT("Process_Detach: instance: %d\n", GetInstanceNumber());
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
	
	virtual const char *name() { return "TEST-I0HDV"; }

private:


	// copy ctor and assignment op deleted: this is a singleton global object
    ExtIODll (const ExtIODll &) = delete;
    ExtIODll & operator=(const ExtIODll &) = delete ;

    // static object that guarantees one (and one only) instance is generated at the global
	// level, so that C++ runtime builds it , calling the empty c'tor
	// the trick here is that the following static member is an object of this _same_ class
	// so it has access to c'tor even if it is kept private
	// see http://accu.org/index.php/journals/1328 by Alexander Nasonov
	//
    static ExtIODll singleton;

	ExtIODll ();
	virtual ~ExtIODll ();
};

#endif
