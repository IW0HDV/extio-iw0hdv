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

include_directories(.)

SET(TEST_SRCS
	extio_test.cpp  
)


link_libraries(extiobase)
link_libraries(util)

add_library(extio_test_cmake_mgw SHARED ${TEST_SRCS} )

#
# set the property in oder to avoid cmake generate -Wl,B=dynamic
# see http://www.cmake.org/pipermail/cmake/2011-March/043538.html
#
set_property(TARGET extio_test_cmake_mgw PROPERTY LINK_SEARCH_START_STATIC 1)

#target_link_libraries(extio_perseus_cmake_mgw ws2_32 )
#target_link_libraries(extio_perseus_cmake_mgw iphlpapi)

target_link_libraries(extio_test_cmake_mgw )
set_target_properties(extio_test_cmake_mgw
	PROPERTIES
	PREFIX "" # remove the trailing standard 'lib' prefix
	OUTPUT_NAME "extio_test_cmake_mgw-${VERSION}"
)
###########################################################################



#message ( "CXX_CFLAGS:                "  ${CMAKE_CXX_FLAGS})			
#message ( "CMAKE_SHARED_LINKER_FLAGS: "  ${CMAKE_SHARED_LINKER_FLAGS})			

GENERATE_EXPORT_HEADER( extio_test_cmake_mgw
             BASE_NAME extio_test_cmake_mgw
             EXPORT_MACRO_NAME extio_test_cmake_mgw_EXPORT
             EXPORT_FILE_NAME MyLibrary_Export.h
             STATIC_DEFINE extio_test_cmake_mgw_BUILT_AS_STATIC
)

#
# copy in selected locations 
#
add_custom_target(dist_test
	COMMAND cp ${CMAKE_BINARY_DIR}/test/$<TARGET_FILE_NAME:extio_test_cmake_mgw> /c/Users/andrew/HDSDR_270/
	COMMAND cp ${CMAKE_BINARY_DIR}/test/$<TARGET_FILE_NAME:extio_test_cmake_mgw> /c/Users/andrew/Studio1_1.6/
)

