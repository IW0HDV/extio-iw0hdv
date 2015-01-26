#
# IW0HDV Extio
#
# Copyright 2015 by Andrea Montefusco IW0HDV
#
# Licensed under GNU General Public License 3.0 or later. 
# Some rights reserved. See COPYING, AUTHORS.
#
# @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
#

SET(UTIL_SRCS
		log.cpp  
		message_allocator.cpp    
)

add_library(util STATIC ${UTIL_SRCS} )


add_executable(testcfg2  testcfg2.cpp)


##########add_executable(testcfg ${UTIL_SRCS} testcfg.cpp)
