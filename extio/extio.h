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
 
#if !defined __EXTIO_H__
#define		 __EXTIO_H__

//
// Extio types definitions
// (that doesn't have equivalent in C++)
// 
struct int24 {
	unsigned char v1;
	unsigned char v2;
	unsigned char v3;
};
struct int24_hpsdr {
	unsigned char v1;
	unsigned char v2;
	unsigned char v3;
};

//
// Integer constants to Extio types mapper class
// (See Modern C++ Design, A.Alexandrescu, 2.4)
//
template <int v, typename T>
struct ExtIOtype
{
	enum { value = v };
	typedef T sample_type;
	char dummy;
};



#include <stdint.h>

/*
	type, this is an index code that Winrad uses to identify the hardware type supported by the DLL.
	Please use one the following values :
	3	the hardware does its own digitization and the audio data are returned 
		to Winrad via the callback device. Data must be in 16 bit (short) format, little endian.
	4	The audio data are returned via the sound card managed by Winrad. 
		The external hardware just controls the LO, and possibly a preselector, under DLL control.
	5	the hardware does its own digitization and the audio data are returned 
		to Winrad via the callback device. Data are in 24 bit integer format, little endian.
	6	the hardware does its own digitization and the audio data are returned
		to Winrad via the callback device. Data are in 32 bit integer format, little endian.
	7	the hardware does its own digitization and the audio data are returned
		to Winrad via the callback device. Data are in 32 bit float format, little endian.
	8	special format for HPSDR hardware; the hardware does its own digitization and the audio data are returned 
		to Winrad via the callback device. Data are in 24 bit integer format, little endian.

*/

typedef ExtIOtype < 3, int16_t>		ExtIO_int16;
typedef ExtIOtype < 5, int24>		ExtIO_int24;
typedef ExtIOtype < 6, int32_t>		ExtIO_int32;
typedef ExtIOtype < 7, float>		ExtIO_float32;
typedef ExtIOtype < 8, int24_hpsdr>	ExtIO_int_hpsdr;


#include "dllmain.h"

#define EXTIO_API __declspec(dllexport)

// type for Extio callback, used in order to send data and signalling events to the main program
typedef  void (* EXTIO_RX_CALLBACK) (int, int, float, int *) ;

#include <log.h>

/**
 * Class Extio
 * base class for Extio dll
 * derive one for each family of radios
 */
class Extio: public Dll {
protected:
	Extio (HMODULE h);
public:
	virtual ~Extio() { }
	
	// the four methods below may be implemented
	void ProcessAttach () {}
	void ProcessDetach () {}
	void ThreadAttach () {}
	void ThreadDetach () {}

	// start of methods to be implemented in derived classes
	virtual bool InitHW(char *name, char *model, int& index) { index = -1; return false; }
	virtual bool OpenHW(void) { return false; }
	virtual void CloseHW(void) {}
	
	virtual int  StartHW(long freq) { return 0; }
	virtual void StopHW(void) {}
	
	virtual int SetHWLO(long LOfreq) { return 0; }
	virtual long GetHWLO(void) { return 0; }
	
	virtual long GetHWSR(void) { return 0; }
	virtual int GetStatus(void) { return 0; }
	
	virtual void ShowGUI(void) {}
	virtual void HideGUI(void) {}
	// end of methods to be implemented in derived classes

	/*
		The callback function in Winrad that the DLL is expected to call, is defined as follows :
		
		void extIOCallback(int cnt, int status, float IQoffs, short IQdata[])
		
		Parameters :
		
		cnt
		
		is the number of samples returned. As the data is complex (I/Q pairs), 
		then there are two 16 bit values per sample.
		If negative, then the callback was called 
		just to indicate a status change, no data returned. 
		Presently Winrad does not use this value, 
		but rather the return value of the StartHW() API, to allocate the 
		buffers and process the audio data returned by the DLL. 
		The cnt value is checked only for negative value, meaning a status change.
		
		status
		
		is a status indicator (see the call GetStatus). 
		When the DLL detects a HW change, e.g. a power On or a power Off, 
		it calls the callback function with a cnt parameter negative, 
		indicating that no data is returned, but that the call 
		is meant just to indicate a status change.
		Currently the status parameter has just two implemented values 
		(apart from those used by the SDR 14/SDR IQ hardware):
		
		100
		This status value indicates that a sampling frequency change 
		has taken place, either by a hardware action, or by an 
		interaction of the user with the DLL GUI.
		When Winrad receives this status, it calls immediately 
		after the GetHWSR() API to know the new sampling rate.
		
		101
		This status value indicates that a change of the LO frequency 
		has taken place, either by a hardware action, or by 
		an interaction of the user with the DLL GUI.
		When Winrad receives this status, it calls immediately 
		after the GetHWLO() API to know the new LO frequency.
		
		102
		This status value indicates that the DLL has temporarily 
		blocked any change to the LO frequency. 
		This may happen, e.g., when the DLL has started recording 
		on a WAV file the incoming raw data. 
		As the center frequency has been written into the WAV file header, 
		changing it during the recording would be an error.
		
		103
		This status value indicates that changes to the 
		LO frequency are again accepted by the DLL
		
		104 
		****** CURRENTLY NOT IMPLEMENTED *******
		This status value indicates that a change of the LO frequency 
		has taken place, and that Winrad should act so to keep 
		the Tune frequency unchanged. 
		When Winrad receives this status, it calls immediately 
		after the GetHWLO() API to know the new LO frequency
		
		105
		This status value indicates that a change of the Tune frequency 
		has taken place, either by a hardware action, or by an interaction 
		of the user with the DLL GUI.
		When Winrad receives this status, it calls immediately after 
		the GetTune() API to know the new Tune frequency. 
		The TuneChanged() API is not called when setting the new Tune frequency
		
		106
		This status value indicates that a change of the demodulation mode 
		has taken place, either by a hardware action, or by an interaction 
		of the user with the DLL GUI.
		When Winrad receives this status, it calls immediately after the GetMode() 
		API to know the new demodulation mode. 
		The ModeChanged() API is not called when setting the new mode.
		
		107
		This status value indicates that the DLL is asking Winrad to behave 
		as if the user had pressed the Start button. 
		If Winrad is already started, this is equivalent to a no-op.
		
		108
		This status value indicates that the DLL is asking Winrad to behave 
		as if the user had pressed the Stop button. 
		If Winrad is already stopped, this is equivalent to a no-op.
		
		109
		This status value indicates that the DLL is asking Winrad to change 
		the passband limits and/or the CW pitch. 
		When Winrad receives this status, it calls immediately the GetFilters API.
		
		Upon request from the DLL writer, the status flag could be managed 
		also for other kinds of external hardware events.
		
		IQoffs
		If the external HW has the capability of determining and providing 
		an offset value which would cancel or minimize the DC offsets of the two outputs, 
		then the DLL should set this parameter to the specified value. 
		Otherwise set it to zero.
		
		IQdata
		This is a pointer to an array of samples where the DLL is expected 
		to place the digitized audio data in interleaved format (I-Q-I-Q-I-Q etc.) 
		in little endian ordering. 
		The number of bytes returned must be equal to IQpairs * 2 * N,
		where IQpairs is the return value of the StartHW() API, and N is the sizeof()
		of the type of data returned, as specified by the ‘type’ parameter of the InitHW() API.
		
	*/
	
	// signals methods to parent client (DSP program)
	void signalSamplerateChanged () { if (extioCallback) (*extioCallback) (-1, 100, 0., 0); }
	void signalLocalOscillatorChanged () { if (extioCallback) (*extioCallback) (-1, 101, 0., 0); }
	void signalLocalOscillatorHold () { if (extioCallback) (*extioCallback) (-1, 102, 0., 0); }
	void signalLocalOscillatorRelease () { if (extioCallback) (*extioCallback) (-1, 103, 0., 0); }
	// 104 not implemented
	void signalTuneFrequencyChanged () { if (extioCallback) (*extioCallback) (-1, 105, 0., 0); }
	void signalDemodulatorChanged () { if (extioCallback) (*extioCallback) (-1, 106, 0., 0); }
	void signalStartPressed () { if (extioCallback) (*extioCallback) (-1, 107, 0., 0); }
	void signalStartReleased () { if (extioCallback) (*extioCallback) (-1, 108, 0., 0); }
	void signalFiltersChanged () { if (extioCallback) (*extioCallback) (-1, 109, 0., 0); }

	// the static pointer to ourself is used in DLL static functions 
	// to invoke virtual members in order to be sure that 
	//
	// 1) the global object has been constructed in a derived class 'ctor
	// 2) the virtual methods in derived classes are polymorphically called
	//
	static Extio * get() ;

	EXTIO_RX_CALLBACK extioCallback;

};



#endif

