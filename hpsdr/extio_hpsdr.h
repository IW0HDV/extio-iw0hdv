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

#if !defined	__EXTIO_HPSDR_H__
#define			__EXTIO_HPSDR_H__

#include "extio.h"
#include "hpsdr.h"
#include "Extio_config.h"



#define EXTIO_API __declspec(dllexport)

// type for Extio callback, used for signallingof data and events to main program
typedef  void (* EXTIO_RX_CALLBACK) (int, int, float, int *) ;

extern EXTIO_RX_CALLBACK ExtioCallback ;


#include "intradllcomm.h"
#include "log.h"
#include "dllmain.h"  // for GetInstanceNumber

template < typename ST, int NS >
class IdllComm: public IntraComm 
{
public:
	IdllComm (int channel, EXTIO_RX_CALLBACK *pc):IntraComm(channel),cback(pc)
	#if !defined NDEBUG
	,ni(0) 
	#endif
	{}

	int receive (unsigned channel, unsigned char *buf, int len) {
		#if !defined NDEBUG
		if ( (ni % 4096) == 0 ) { 
			LOGT("---------- receive: RX: %d  buflen: %d\n", channel, len); 
			ni++;
		}
		ni++;
		#endif
		if (len != (NS*2*sizeof(typename ST::sample_type))) {
			LOGX("ERROR Callback on channel %d: length: %d (%p): expected:%d received: %d\n", channel, len, *cback, (NS*2*sizeof(typename ST::sample_type)), len);
		} else
			(*cback) (NS, 0, 0., (int *) buf );
		return 0;
	}
private:
	EXTIO_RX_CALLBACK *cback;
	#if !defined NDEBUG
	int ni;
	#endif
};


/*
 * class ExtioDataConversion
 *
 * helper class for buffering and conversion of HPSDR samples to Extio formats 
*/
template < typename ST >
class ExtioDataConversion {
public:
	ExtioDataConversion (int _ns):ns(_ns) { pb = new unsigned char[2 * ns * sizeof(typename ST::sample_type)]; }
	~ExtioDataConversion() { delete [] pb;  }
	int getNs() { return ns; }

	unsigned char *pb;
	int ns;

	int convert_iq_from_rx (HpsdrRxIQSample *i, HpsdrRxIQSample *q, ExtIO_int24)
	{
		int24 *p = (int24 *)pb;
		for (int n = 0; n < ns; ++n) {
			p->v1 = i->s3, p->v2 = i->s2, p->v3 = i->s1; ++p;
			p->v1 = q->s3, p->v2 = q->s2, p->v3 = q->s1; ++p;
			++i, ++q;
		}
		return 0;
	}

	int convert_iq_from_rx (HpsdrRxIQSample *i, HpsdrRxIQSample *q, ExtIO_int32)
	{
		int *p = (int *)pb;
		for (int n = 0; n < ns; ++n)
			*p++ = i->int_32(),
			*p++ = q->int_32(),
			++i,
			++q;
		return 0;
	}

	int convert_iq_from_rx (HpsdrRxIQSample *i, HpsdrRxIQSample *q, ExtIO_float32)
	{
		float *p = (float *)pb;
		for (int n = 0; n < ns; ++n)
			*p++ = i->float_32(),
			*p++ = q->float_32(),
			++i,
			++q;
		return 0;
	}

	int convert_iq_from_rx(HpsdrRxIQSample *i, HpsdrRxIQSample *q, ExtIO_int_hpsdr)
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

class Radio;
class GuiHpsdr;
class CommandReceiver;

template <typename ST>
class ExtioHpsdrRadio : public ExtioDataConversion<ST>
{
public:
	ExtioHpsdrRadio(int ns, Radio *p, EXTIO_RX_CALLBACK *pCb, CommandReceiver *pCmdRec): 
		ExtioDataConversion<ST>(ns), 
		cnt(0), 
		pidc(0), 
		pR_(p), 
		pCr_(pCmdRec),
		pExtioCallback(pCb)
	{  }

	virtual ~ExtioHpsdrRadio() {}


	void setIdllComm(IntraComm *pIdc)
	{ 
		pidc = pIdc; 
	}

	// called when the rx buffer is full
	int send_iq_from_rx_to_dsp (int nrx, HpsdrRxIQSample *i, HpsdrRxIQSample *q, int ns)
	{
		// convert and copy raw I/Q data from native HPSDR format to ExtIO format
		this->convert_iq_from_rx(i, q, ST()); // Extio type selection is done at compile type !

		// internal buffers counter
		//if ((cnt % 1024) == 0) { LOGT("---------- RX: %d  buflen: %d\n", nrx, ns); cnt++; }

		// setup a buffer for ExtIO
		if (pidc) pidc->send(nrx, this->pb, 2 * this->getNs() * sizeof(typename ST::sample_type));
		cnt++;
		return 0;
	}

	void setSampleRateHW(int new_sr);
	// send only the event to parent SDR sw
//	void changeSamplerate();

	
	Radio * getRadio() { return pR_; }

	virtual GuiHpsdr *CreateGui(int sr) = 0;

protected:
	int cnt;
	IntraComm *pidc;
	Radio *pR_;
	CommandReceiver *pCr_;
	EXTIO_RX_CALLBACK *pExtioCallback;
};

#include "hpsdr.h"

template < typename ST >
class ExtioMercuryRadio : public Mercury, public ExtioHpsdrRadio<ST>
{
public:
	ExtioMercuryRadio(int ns, EXTIO_RX_CALLBACK *pCb, CommandReceiver *pCr): 
		Mercury(), ExtioHpsdrRadio<ST>(ns, this, pCb, pCr)
	{  }

	GuiHpsdr *CreateGui(int sr);

	//
	// this is a virtual method inherited from Radio base class 
	// it is called when the rx buffer is full
	//
	int process_iq_from_rx(int nrx, HpsdrRxIQSample *i, HpsdrRxIQSample *q, int ns)
	{
		return this->send_iq_from_rx_to_dsp(nrx, i, q, ns);
			//ExtioHpsdrRadio<ST>::send_iq_from_rx_to_dsp(nrx, i, q, ns);
	}
};


template < typename ST >
class ExtioHermesRadio : public Hermes, public ExtioHpsdrRadio<ST>
{
public:
	ExtioHermesRadio(int ns, EXTIO_RX_CALLBACK *pCb, CommandReceiver *pCr):
		Hermes(), ExtioHpsdrRadio<ST>(ns, this, pCb, pCr)
	{  }

	GuiHpsdr *CreateGui(int sr);

	//
	// this is a virtual method inherited from Radio base class
	// it is called when the rx buffer is full
	//
	int process_iq_from_rx(int nrx, HpsdrRxIQSample *i, HpsdrRxIQSample *q, int ns)
	{
		return this->send_iq_from_rx_to_dsp(nrx, i, q, ns);
	}
};

class Gui;
class ExtIODll;
//
// redirects to the proper GUI elements all the events coming from HPSDR Flow
//
class ExtioEthernet : public Ethernet {
public:
	ExtioEthernet(Flow *pF, ExtIODll *peio) : Ethernet(pF), peio_(peio)
	{}
	~ExtioEthernet() {}

	void FatalError(const char *pMsg);
	void TransmissionTmo(const char *pMsg);

private:
	ExtIODll *peio_;
};

#include "ExtIO_config.h"

template <
	typename EXTIO_SAMPLE_TYPE
>
struct RadioFactory {

	ExtioHpsdrRadio<EXTIO_SAMPLE_TYPE> *Create(const char *board_id, unsigned char *buf, EXTIO_RX_CALLBACK *pCb, CommandReceiver *pCr)
	{
		ExtioHpsdrRadio<EXTIO_SAMPLE_TYPE> *pExr = 0;
		// decides at run time which HW we have 
		// placement new used, in order to share it among all instances
		if (strcmp(board_id, "Mercury") == 0 || strcmp(board_id, "Metis") == 0) {
			pExr = new (buf)ExtioMercuryRadio < EXTIO_SAMPLE_TYPE >(EXTIO_NS, pCb, pCr);
		} else
		if (strcmp(board_id, "Hermes") == 0) {
			pExr = new (buf)ExtioHermesRadio < EXTIO_SAMPLE_TYPE >(EXTIO_NS, pCb, pCr);
		} else
		if ((strcmp(board_id, "Orion") == 0) || (strcmp(board_id, "Angelia") == 0)) {
			pExr = new (buf)ExtioHermesRadio < EXTIO_SAMPLE_TYPE >(EXTIO_NS, pCb, pCr);
		}
		return pExr;
	}
	ExtioHpsdrRadio<EXTIO_SAMPLE_TYPE> *Pointer(const char *board_id, unsigned char *buf, CommandReceiver *pCr)
	{
		ExtioHpsdrRadio<EXTIO_SAMPLE_TYPE> *pExr = 0;
		// decides at run time which HW we have 
		// placement new used, in order to share it among all instances
		if (strcmp(board_id, "Mercury") == 0 || strcmp(board_id, "Metis") == 0) {
			pExr = (ExtioMercuryRadio < EXTIO_SAMPLE_TYPE > *) buf;
		} else
		if (strcmp(board_id, "Hermes") == 0) {
			pExr = (ExtioHermesRadio < EXTIO_SAMPLE_TYPE > *) buf;
		} else
		if ((strcmp(board_id, "Orion") == 0) || (strcmp(board_id, "Angelia") == 0)) {
			pExr = (ExtioHermesRadio < EXTIO_SAMPLE_TYPE > *) buf;
		}
		return pExr;
	}

};

//typedef ExtioHpsdrRadio <EXTIO_SAMPLE_TYPE> ExtioHpsdrRadioT;

class Radio;
class GuiHpsdr;
class HpsdrSplash;
class ExtioEthernet;
class CommandReceiver;

#include "dllmain.h"
#include <memory>

class ExtIODll : public Extio {
public:
//	ExtIODll(HMODULE h): Extio(h), pR(0), pExr(0), pGui(0), pExtioEth(0), pCmdRec(0), rxIQ(0) {}
   	ExtIODll();
	void ProcessAttach();
	void ProcessDetach();
	void ThreadAttach() {};
	void ThreadDetach() {};
	
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
	
	virtual const char *name() { return "HPSDR-I0HDV"; }

	// pointers to abstract types
	Radio  *pR;
	ExtioHpsdrRadio < EXTIO_SAMPLE_TYPE > *pExr;
	GuiHpsdr *pGui;

	ExtioEthernet *pExtioEth;
	CommandReceiver *pCmdRec;
	IdllComm < EXTIO_SAMPLE_TYPE, EXTIO_NS > *rxIQ;
	std::shared_ptr<HpsdrSplash> pSplash;

private:
};

template <
	typename EXTIO_SAMPLE_TYPE
>
ExtioHpsdrRadio<EXTIO_SAMPLE_TYPE> * CreateExtioHpsdrRadio(const char *board_id, CommandReceiver *,  ExtIODll *pEio);

#endif
