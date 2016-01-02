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

#if !defined __EXTIO_CONFIG_H__
#define		 __EXTIO_CONFIG_H__

#include "extio.h"

/*
 *	Extio base parameters specialization
 */

// type of samples exchanged on each ExtIO callback invocation
typedef ExtIO_float32 EXTIO_BASE_TYPE; 

// # I/Q samples exchanged on each callback invocation
const int EXTIO_NS = 8192;	

// size of ExtIO sample type
enum { EXTIO_BASE_TYPE_SIZE = sizeof(typename EXTIO_BASE_TYPE::sample_type) }; 

// starting sampling rate
const int EXTIO_DEFAULT_SAMPLE_RATE = 192000;	



#endif
