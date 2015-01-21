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

SET(LIBAIRSPY_SRCS
		airspy.c
		iqconverter_int16.c
		iqconverter_float.c
)

# force C++ compile in order to avoid link problems with pthread library
SET_SOURCE_FILES_PROPERTIES( ${LIBAIRSPY_SRCS} PROPERTIES LANGUAGE CXX )

add_library(libairspy STATIC ${LIBAIRSPY_SRCS} )
