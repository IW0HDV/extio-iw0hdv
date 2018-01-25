#
# Extio for Airspy
#
# Copyright 2015 by Andrea Montefusco IW0HDV
#
# Licensed under GNU General Public License 3.0 or later. 
# Some rights reserved. See COPYING, AUTHORS.
#
# @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
#

SET(LIBAIRSPYHF_SRCS
	iqbalancer.c
	airspyhf.c
)

# force C++ compile in order to avoid link problems with pthread library
SET_SOURCE_FILES_PROPERTIES( ${LIBAIRSPYHF_SRCS} PROPERTIES LANGUAGE CXX )

add_library(libairspyhf-0.0.0 STATIC ${LIBAIRSPYHF_SRCS} )


