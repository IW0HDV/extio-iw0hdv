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

include(GetGitRevisionDescription)
#
# parse the version information into pieces.
#
# http://brianmilco.blogspot.it/2012/11/cmake-automatically-use-git-tags-as.html
#
git_describe(VERSION --tags --dirty=-dirty --always)
string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+(.*)" "\\1" VERSION_SHA1 "${VERSION}")
set(VERSION_SHORT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/git-sha1.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/git-sha1.cpp" @ONLY)
#list(APPEND UTIL_SRCS "${CMAKE_CURRENT_BINARY_DIR}/git-sha1.cpp" git-sha1.h)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/git-raw.h.in" "${CMAKE_CURRENT_BINARY_DIR}/git-raw.h" @ONLY)


SET(UTIL_SRCS
		log.cpp  
		message_allocator.cpp    
		"${CMAKE_CURRENT_BINARY_DIR}/git-sha1.cpp" 
		git-sha1.h
)


add_library(util STATIC ${UTIL_SRCS} )


add_executable(testcfg2  testcfg2.cpp)

##########add_executable(testcfg ${UTIL_SRCS} testcfg.cpp)
