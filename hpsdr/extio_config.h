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

#include <extio.h>

/**
 *	Extio base parameters
 *
 */

typedef		ExtIO_int24		EXTIO_SAMPLE_TYPE;						// type of sample exchanged on each callback invocation

const int	EXTIO_DEFAULT_SAMPLE_RATE = HPSDR_DEFAULT_SAMPLE_RATE;	// starting sampling rate: 192000

const int	EXTIO_NS = 1024;										// samples exchanged on each callback invocation



#endif
