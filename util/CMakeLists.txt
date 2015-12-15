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

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/git-sha1.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/git-sha1.cpp" @ONLY)
#list(APPEND UTIL_SRCS "${CMAKE_CURRENT_BINARY_DIR}/git-sha1.cpp" git-sha1.h)

SET(UTIL_SRCS
		log.cpp  
		message_allocator.cpp    
		"${CMAKE_CURRENT_BINARY_DIR}/git-sha1.cpp" 
		git-sha1.h
)


add_library(util STATIC ${UTIL_SRCS} )


add_executable(testcfg2  testcfg2.cpp)

##########add_executable(testcfg ${UTIL_SRCS} testcfg.cpp)
