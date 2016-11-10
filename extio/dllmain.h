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

/** 
 * @file dllmain.h
 * @brief Header for generic DLL service functions
 * @author Andrea Montefusco IW0HDV
 * @version 0.0
 * @date 2013-09-23
 */

#if !defined	__DLLMAIN_H__
#define			__DLLMAIN_H__

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>


class Dll {
protected:
	Dll (HMODULE hm);

public:
	virtual ~Dll () {};

	static  void Register(Dll *);
	
	virtual void ProcessAttach () {} //= 0;
	virtual void ProcessDetach () {} //= 0;
	virtual void ThreadAttach () {} //= 0;
	virtual void ThreadDetach () {} //= 0;

	void inc () ;
	void dec () ;

	static Dll *getObj ();
	static HMODULE GetMyHandle() { return hMod; }
	void SetHModule(HMODULE hm) { hMod = hm; }
	static int GetInstanceNumber ();
	static int GetInstanceQuantity () ;
	virtual const char *name() { return ""; }

private:
	Dll() = delete;
	Dll(const Dll&) = delete;
	static HMODULE hMod;
	static int instance_;
	static int local_instance;
};

#if 0
Dll *createDllObject (HMODULE); // forward declaration, to be defined before using DLL_CLASS macro

//
// this macro generate code for the derived class factory
// can be instantiated one time only 
//
#define DLL_CLASS(T) \
Dll *createDllObject (HMODULE h) \
{ \
    if (Dll::getObj() == 0) \
	    return new T(h); \
	else \
		return Dll::getObj(); \
};
#endif

#endif