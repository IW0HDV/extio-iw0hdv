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
 * @file hpsdr.h
 * @brief HPSDR Hermes modeling classes
 * @author Andrea Montefusco IW0HDV
 * @version 0.0
 * @date 2013-09-23
 */

#if ! defined	__HPSDR_H__
#define			__HPSDR_H__

#include <list>
#include <iostream>
#include <assert.h>
#include <pthread.h>
#include "log.h"
#include "util.h"

#include <winsock2.h>
#include <iphlpapi.h>
#pragma  comment (lib, "ws2_32.lib")
// Link with Iphlpapi.lib
#pragma comment(lib, "IPHLPAPI.lib")
#pragma warning( disable : 4995 )

#define HPSDR_DEFAULT_SAMPLE_RATE 192000
//
// the HPSDR native samples are 24 bits signed integer, big endian 
//

struct HpsdrRxIQSample {
	unsigned char s1;  // MSB
	unsigned char s2;
	unsigned char s3;  // LSB

	int int_32 ()
	{	
		int rv = 0;
		rv = (s1 << 16) | (s2 << 8) | (s3);
		// sign extension
		if (rv & 0x00800000) rv = rv | 0xff000000;
		return rv;
	}
	float float_32 ()
	{	
		int rv;
		// samples from hardware are 24 bit signed integer
		// put them in a regular int
		rv  = (int)((signed char)   s1) << 16;
		rv += (int)((unsigned char) s2) << 8 ;
		rv += (int)((unsigned char) s3)      ;
		// next, rescale to +1.0/-1.0
		return ((float)rv)/8388607.0f;
	}
};


struct HpsdrMicSample {
	unsigned char s1;  // MSB
	unsigned char s2;  // LSB

	int int_32 ()
	{	
		int rv;
		// samples from hardware are 24 bit signed integer
		// put them in a regular int
		rv  = (int)((signed char)   s1) << 8;
		rv += (int)((unsigned char) s2)     ;
		// next, rescale to 32 bit
		rv <<= 16;
		return rv;
	}
	float float_32 ()
	{	
		int rv;
		// samples from hardware are 24 bit signed integer
		// put them in a regular float
		rv  = (int)((unsigned char) s1) << 8 ;
		rv += (int)((unsigned char) s2)      ;
		// next, rescale to +1.0/-1.0
		return ((float)rv)/32767.0f;
	}
};


class Link;  // forward declaration
class Radio;

class Flow
{
public:
	Flow (Radio *pR): pr(pR), nrxp(0), send_status(0) {};

	void initialization (Link *pL) ;
	int processFromRadio (unsigned char *);
	void processToRadio (unsigned char *);

	enum { O_BUF_SIZE = 512 } ;  // buffer size for HPSDR data packet

private:
	enum { SC = 0x7f} ;          // synchronization character for HPSDR data packet
	Radio *pr;
	Link  *pl;
	unsigned char ob[O_BUF_SIZE];
	int nrxp; // number of rx packet processed
	static const int usableBufLen [9];
	int send_status;
};



class Link {

public:
	Link (Flow *pF): pFlow(pF) {}
	virtual ~Link () {}

	static bool scan_devices ();

	virtual int	write (unsigned char ep, unsigned char* buffer,int length) { assert(0); return 0;} ; //= 0;



protected:
	Flow *pFlow;

private:
	
};

class ScanWatcher;


class Ethernet: public Link {

public:

struct NetInterface {
	char			ip_address[16];
	unsigned long	b_ip_address;
	unsigned char	hw_address[6];
	char			name [256];
	int				d_socket;
	int				status;
	} ;

struct Device {
	char	ip_address[16];
	char	mac_address[18];
	char	board_id[64];
	int     board_code;
	int		code_version;
	unsigned long b_card_ip_address;
};


	Ethernet (Flow *pF): Link(pF), data_socket(-1), sequence(-1),  watchdog_timeout_in_ms(250), send_sequence(-1), offset(8) {}
	virtual ~Ethernet () {}

	std::list < struct NetInterface >	getInterfaceList ();
	std::list < struct Device >			getDeviceList ();

	static int  scan_interface (int x, char *ifName);
	static bool scan_devices   (ScanWatcher *);   // was discover()
	
	static struct Device *found (int n = 0);

	void	startReceive (struct Device *p);
	void	stopReceive  ();
	int		write (unsigned char ep, unsigned char* buffer,int length);

protected:
	virtual void FatalError (const char *) = 0;
	virtual void TransmissionTmo (const char *) = 0;

private:
	
	int data_socket;
	struct sockaddr_in data_addr;
	int data_addr_length;

	unsigned char buffer[70];

	pthread_t receive_thread_id;
	pthread_t watchdog_thread_id;

	int ep;
	long sequence;
	int watchdog_timeout_in_ms;

	static void * receive_thread (void* arg) ;

	// write messages section
	unsigned char output_buffer[1032];
	unsigned char input_buffer[20480];
	long send_sequence;
	int offset;

	// internally used to compose a full packet to hardware
	void	send_buffer (int s, unsigned char* buffer, int length);

	static void * watchdog_thread (void* arg);

	enum { MAX_DEVICES = 10 };
	static struct Device devs [MAX_DEVICES];

	enum { MAX_INTERFACES = 256 };
	static struct NetInterface interfaces[MAX_INTERFACES];
	static int nif;
	static int dev_found;

	static struct Device * search_dev_by_ip (const char * ip);

};


class ScanWatcher
{
public:
	// called during scan_devices, to be overridden in derived classes
	virtual int ScanStarted() { return 0; }
	virtual int ScanStopped(int x = 0) { return 0; } // called with the number of devices found
	virtual int	InterfaceFound(Ethernet::NetInterface *)	{ return 0; }
	virtual int	DeviceFound(Ethernet::Device *)		{ return 0; }

	~ScanWatcher() {}
};


//
// class for control part of the HPSDR frame
//
struct CtrlBuf {
	unsigned char c[5] ;
};

class Radio; // forward declaration

template < int  N_SAMPLES = 1024 >
struct Receiver {

	Receiver (): pr(0), frequency_changed(false), frequency(7050000), ns(0) {}

	void setRadio (Radio *pR) { pr = pR; };
	void setN (int n) { ni = n; };

	Radio *pr;
	int    ni;   // istance number
	bool frequency_changed;
	long frequency;
	HpsdrRxIQSample input_buffer_i   [N_SAMPLES];  // I, Q, Mic
	HpsdrRxIQSample input_buffer_q   [N_SAMPLES];
	HpsdrMicSample  input_buffer_mic [N_SAMPLES];

	int output_buffer[N_SAMPLES*4]; // Left Audio, Right Audio, Tx I, Tx Q
	// output buffer toward the DLL CALLBACKS
	//T cb_buffer [2][N_SAMPLES*2*N_BLOCKS];
	int ns;
	//T nb;
	//T ni;

	void append_input_iq  ( HpsdrRxIQSample ls, HpsdrRxIQSample rs)  { input_buffer_i[ns] = ls, input_buffer_q[ns] = rs; };
	void append_input_mic ( HpsdrMicSample ms ) { input_buffer_mic[ns] = ms; };
	void next_sample () { ++ns; }
	
	bool is_buffer_full (bool process_tx = false);
};



class AlexFilter {
public:
	AlexFilter() : manual(false) /* disabled */, lp(_3020m), hp(_6_5M) {}
	virtual ~AlexFilter() {}
	enum LowPass {
		_3020m = B8(00000001),
		_6040m = B8(00000010),
		_80m   = B8(00000100),
		_160m  = B8(00001000),
		_6m    = B8(00010000),
		_1210m = B8(00100000),
		_1715m = B8(01000000)
	};
	enum HighPass {
		_13M    = B8(00000001),
		_20M    = B8(00000010),
		_9_5M   = B8(00000100),
		_6_5M   = B8(00001000),
		_1_5M   = B8(00010000),
		_bypass = B8(00100000),
		_6M     = B8(01000000),
	};
	
	void setTxAnt (int nta)
	{
		tx_ant = nta;
	}
	
	void setManual(bool nm) { 
		manual = nm;
	}

	void setLP(LowPass nlp) { lp = nlp; }
	LowPass getLP() { return lp; }

	void setHP(HighPass nhp) { hp = nhp; }
	HighPass getHP() { return hp; }

	void setCtrl_0 (CtrlBuf *cd) 
	{
		// Alex TX
		// Ant 1-2-3 control
		cd->c[4] &= B8(11111100);
		cd->c[4] |= (tx_ant & 0x03);
	}
	void setCtrl_9 (CtrlBuf *cd) 
	{
		if (manual) {
			cd->c[2] |= B8(01000000);
		} else {
			cd->c[2] &= B8(10111111);
		}

		// Alex RX
		// clear all HPF bits
		cd->c[3] &= B8(10000000);
		cd->c[3] |= hp;

		// Alex TX
		// clear all LPF bits
		cd->c[4] &= B8(10000000);
		cd->c[4] |= lp;
	}

private:
	bool	 manual; /* false == 0 - manual mode disabled    */
	LowPass  lp;
	HighPass hp;
	int		 tx_ant;
};

class Radio
{
public:
	Radio (): n_rx(1), mox(0), duplex(1)
	{
		pAlex = new AlexFilter;
		//rl.push_back ((Radio *)this);
		for ( int i = 0; i < 8; ++i ) rx[i].setRadio (this), rx[i].setN (i);
		setSampleRate (HPSDR_DEFAULT_SAMPLE_RATE);
	}

	virtual ~Radio() { delete pAlex;  }

	virtual void setFrequency (long nf, int nrec = 0) { rx[nrec].frequency = nf; }
	virtual void getFrequency (long &f, int nrec = 0) { f = rx[nrec].frequency; }

	int		getNumberOfRx () ;
	void	setNumberOfRx(int n) ;

	virtual void setSampleRate (int sr) 
	{
		switch (sr) {
		case 48000:
			sample_rate = sr;
			speed       = 0;
			output_decimation_rate = 1;
			break;
		case 96000:
			sample_rate = sr;
			speed       = 1;
			output_decimation_rate = 2;
			break;
		case 192000:
			sample_rate = sr;
			speed       = 2;
			output_decimation_rate = 4;
			break;
		case 384000:
			sample_rate = sr;
			speed       = 3;
			output_decimation_rate = 8;
			break;
		default:
			LOGT("Invalid sample rate, should be 48000,96000,192000,384000) was %d !\n", sr);
			break;
		}
	}
	void getSampleRate (int &sr) { sr = sample_rate; }

	virtual void setAttenuator (int newAtt) = 0;

	bool setPreamp (bool p)
	{
		preamp = p;
		return true;
	}


	bool setDither (bool d)
	{
		dither = d;
		return true;
	}

	bool setRandomizer (bool r)
	{
		randomizer = r;
		return true;
	}


	//unsigned char getSwVersion() { return sw_ver; }

	//
	// process_iq_from_rx
	// called when the rx buffer is full, consumes I/Q from RX
	// pure virtual function to be implemented in derived radio classes
	//
	virtual int process_iq_from_rx (int nrx, HpsdrRxIQSample *i, HpsdrRxIQSample *q, int ns) = 0;

	virtual int process_iq_audio_to_radio (unsigned char *, unsigned char *, unsigned char *, unsigned char *) { return 0; }



	virtual void getControlData (CtrlBuf *cd) 
	{
		// extract PTT, DOT and DASH, they are present in all control buffers
		ptt  = (cd->c[0] & 0x01) == 0x01;
		dash = (cd->c[0] & 0x02) == 0x02;
		dot  = (cd->c[0] & 0x04) == 0x04;

		switch( (cd->c[0] >> 3) & 0x1F ) {
		// C1 
		// 0 0 0 0 0 0 0 0
		//   | | | | | | |
		//   | | | | | | +---------- LT2208 Overflow (1 = active, 0 = inactive)
        //   | | | | | +------------ Hermes I01 (0 = active, 1 = inactive)
        //   | | | | +-------------- Hermes I02 (0 = active, 1 = inactive)
        //   | | | +---------------- Hermes I03 (0 = active, 1 = inactive)
        //   | | +------------------ Hermes I04 (0 = active, 1 = inactive)
        //   | +-------------------- Cyclops PLL locked (0 = unlocked, 1 = locked)
        //   +---------------------- Cyclops - Mercury frequency changed, bit toggles   
        //                  
        case 0:
            lt2208ADCOverflow = cd->c[1] & 0x01;
            break;
		}
	}

	virtual void setControlData (CtrlBuf *cd) // the first byte has to be preloaded with the control block requested to be filled in
	{
		if (mox) {
			cd->c[0] |= 0x01;
		} else {
			cd->c[0] &= 0xFE;
		}

		switch (cd->c[0] >> 1) {
			case 0:
				cd->c[1] &= 0xfc;
				cd->c[1] |= speed;

				// Preamplifier
				(preamp     == true) ? SET_BIT(cd->c[3], 2) : CLR_BIT(cd->c[1], 2);
				// ADC controls
				(dither     == true) ? SET_BIT(cd->c[3], 3) : CLR_BIT(cd->c[1], 3);
				(randomizer == true) ? SET_BIT(cd->c[3], 4) : CLR_BIT(cd->c[1], 4);

				// duplex ( 0 = off  1 = on )
				cd->c[4] &= 0xfb;
				cd->c[4] |= (duplex << 2); 

				// number or receivers (000 = 1 001 = 2 ...... 111 = 8
				cd->c[4] &= 0xc7 ;
				cd->c[4] |= (n_rx - 1) << 3;
				
				if (pAlex) pAlex->setCtrl_0(cd);
			break;

			case 1: // transmitter frequency TBD
					// temporary use the rx #0 frequency
			{
				int nrec = 0;
				cd->c[1] =  rx[nrec].frequency  >> 24        ;
				cd->c[2] = (rx[nrec].frequency  >> 16) & 0xFF;
				cd->c[3] = (rx[nrec].frequency  >> 8 ) & 0xFF;
				cd->c[4] = (rx[nrec].frequency       ) & 0xFF;
			}
			break;
			
			case 2:  // receiver #1
			case 3:  // receiver #2
			case 4:  // receiver #3
			case 5:  // receiver #4
			case 6:  // receiver #5
			case 7:  // receiver #6
			case 8:  // receiver #7
			{
				int nrec = (cd->c[0] >> 1) - 2;

				cd->c[1] = rx[nrec].frequency >> 24;
				cd->c[2] = (rx[nrec].frequency >> 16) & 0xFF;
				cd->c[3] = (rx[nrec].frequency >> 8) & 0xFF;
				cd->c[4] = (rx[nrec].frequency) & 0xFF;
			}
			break;

			case 9: // 
				pAlex->setCtrl_9(cd);
			break;

		}

	}

	//void printList ()
	//{
	//	std::list<Radio*>::iterator it;
	//	for(it = rl.begin(); it != rl.end(); it++) {
	//		std::cout << *it << std::endl; // compile error
	//	}
	//}

	Receiver <1024> rx[8] ;

	//
	// helpers for AlexFilter
	//
	void setManual(bool nm) { pAlex->setManual(nm); }
	void setLP (AlexFilter::LowPass nlp) {	pAlex->setLP(nlp); 	}
	void setHP(AlexFilter::HighPass nhp) { pAlex->setHP(nhp); }
	void setTxAnt(int na) { pAlex->setTxAnt(na); }
	
private:
	AlexFilter *pAlex;

	int sample_rate;
	int speed;
	int output_decimation_rate;
	int n_rx;
	int   mox;
	int duplex;
	bool preamp;
	bool dither;
	bool randomizer;


	bool  ptt;
	bool  dash;
	bool  dot;
	int   lt2208ADCOverflow;


protected:
	//std::list <Radio *> rl;

	unsigned char sw_ver;
	//virtual void setSwVer (unsigned char) = 0;
};

class Hermes: public Radio
{
public:
	Hermes (): Radio(), oco(0), attenuator(0) 
	{
		//rl.push_back (this);
	}

	void setAttenuator (int att) { attenuator = att; }
	void setOpenCollectorOutputs (int o) { oco = 0 ; }
	void setPowerOut (int p) { power_out = p; }
	int	 getFirmwareversion() { return sw_ver; }
	
	virtual void getControlData (CtrlBuf *cd) 
	{
		Radio::getControlData (cd);
		//std::cout << "Hermes getControlData" << std::endl; // compile error

        switch((cd->c[0] >> 3) & 0x1F) {
        // C1 
        // 0 0 0 0 0 0 0 0
        //   | | | | | | |
        //   | | | | | | +---------- LT2208 Overflow (1 = active, 0 = inactive)
        //   | | | | | +------------ Hermes I01 (0 = active, 1 = inactive)
        //   | | | | +-------------- Hermes I02 (0 = active, 1 = inactive)
        //   | | | +---------------- Hermes I03 (0 = active, 1 = inactive)
        //   | | +------------------ Hermes I04 (0 = active, 1 = inactive)
        //   | +-------------------- Cyclops PLL locked (0 = unlocked, 1 = locked)
        //   +---------------------- Cyclops - Mercury frequency changed, bit toggles   
        //                  
        case 0:
            io1 = (cd->c[1] & 0x02) ? 0: 1;
            io2 = (cd->c[1] & 0x04) ? 0: 1;
            io3 = (cd->c[1] & 0x08) ? 0: 1;
            io4 = (cd->c[1] & 0x10) ? 0: 1;
            sw_ver = cd->c[4] ;
            break;
/*
 *          C0
 *          0 0 0 0 1 x x x    
 *
 *          C1 - Bits 15-8 of Forward Power from Penelope or Hermes* (AIN5)
 *          C2 - Bits 7-0  of Forward Power from Penelope or Hermes* (AIN5)
 *          C3 - Bits 15-8 of Forward Power from Alex or Apollo*(AIN1)
 *          C4 - Bits 7-0  of Forward Power from Alex or Apollo*(AIN1)
 *
 *          Note: All analog levels are 12 bits.
 */
        case 1:
            forwardPower = ( cd->c[1] << 8) + cd->c[2];
            break;
/*
 *          C0
 *          0 0 0 1 0 x x x    
 *
 *          C1 - Bits 15-8 of Reverse Power from Alex or Apollo*(AIN2)
 *          C2 - Bits 7-0  of Reverse Power from Alex or Apollo*(AIN2)
 *          C3 - Bits 15-8 of AIN3 from Penny or Hermes*
 *          C4 - Bits 7-0  of AIN3 from Penny or Hermes*
 *
 *          Note: All analog levels are 12 bits.
 */
        case 2:
            analog_input3 = (cd->c[3] << 8) + cd->c[4];
            break;
/*
 *          C0
 *          0 0 0 1 1 x x x    
 *
 *          C1 - Bits 15-8 of AIN4 from Penny or Hermes*
 *          C2 - Bits 7-0  of AIN4 from Penny or Hermes*
 *          C3 - Bits 15-8 of AIN6,13.8v supply on Hermes*
 *          C4 - Bits 7-0  of AIN6,13.8v supply on Hermes*
 *
 *          *Note: All analog levels are 12 bits.
 */
        case 3:
			analog_input4 = (cd->c[1] << 8) + cd->c[2];
			analog_input6 = (cd->c[3] << 8) + cd->c[4];
			break;

		};
}

	virtual void setControlData (CtrlBuf *cd) 
	{
		Radio::setControlData (cd);

		switch (cd->c[0] >> 1) {
		case 0:
			cd->c[2] &= 0x01;
			cd->c[2] |= (oco << 1);
			break;
		case 9:
			cd->c[1] = power_out; 
			break;
		case 10:
			// Hermes attenuator management
			// set to 
			cd->c[4] |= 0x20;   // enable attenuator
			cd->c[4] &= 0xe0;   // zeroes four LSBs
			cd->c[4] |= (attenuator); // logical OR of attenuator value
			break;
		}
	}

protected:
	//virtual void setSwVer (unsigned char sv) { sw_ver = sv; };
	int io1;
	int io2;
	int io3;
	int io4;
	int forwardPower;
	int analog_input3;
	int analog_input4;
	int analog_input6;
	int oco; // Open Collector Ouputs
	int attenuator;
	int power_out; // 0 - 255
};

class Mercury: public Radio
{
public:
	Mercury (): Radio()
	{
		//rl.push_back (this);
	}

	void setAttenuator (int att) { attenuator = att; }
//	void setOpenCollectorOutputs (int o) { oco = 0 ; }
//	void setPowerOut (int p) { power_out = p; }

	virtual void getControlData (CtrlBuf *cd) 
	{
		Radio::getControlData (cd);
		// specific to Mercury, software release
		switch((cd->c[0] >> 3) & 0x1F) {
		case 0:
			sw_ver = cd->c[2] ;
			break;

		}
	}
	virtual void setControlData (CtrlBuf *cd) 
	{
		Radio::setControlData (cd);

		switch (cd->c[0] >> 1) {
		case 0:
			cd->c[1] |= B8(00001000); // 10 MHz Ref:   10  Mercury
			assert(B8(00001000) == (0x02 << 2));
			cd->c[1] |= (0x01 << 4);  // 122.88 MHz:   1   Mercury
			cd->c[1] |= (0x02 << 5);  // Config:       10  Mercury
			cd->c[1] |= (0x01 << 5);  // Mic Source:   1   Penelope
			// Alex attenuator
			cd->c[3] &= B8(11111100);
			cd->c[3] |= ((attenuator / 10) & B8(00000011));
			break;

		}
	}

protected:
	//virtual void setSwVer (unsigned char sv) { sw_ver = sv; };

	int attenuator;

};

void DumpHpsdrBuffer (const char* rem, int np, const unsigned char* b);

void DumpHpsdrHeader (const char* rem, int np, const unsigned char* b);

#endif
