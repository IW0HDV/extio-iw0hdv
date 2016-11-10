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

find_library(LIBUSB10_LIBRARY
    NAMES usb-1.0
	PATH_SUFFIXES .a
	PATHS /mingw32/lib/
)
if (NOT LIBUSB10_LIBRARY)
	message( FATAL_ERROR "libusb-1.0 not found")
else ()
	#message ( "*************** libusb-1.0 found: "  ${LIBUSB10_LIBRARY})
	link_libraries(${LIBUSB10_LIBRARY})
endif ()

find_library(PTHREAD_LIBRARY
	NAMES pthread.a
	PATH_SUFFIXES .a
)
if (NOT PTHREAD_LIBRARY)
	message( FATAL_ERROR "pthread not found")
else ()
	message ( "*************** pthread found: "  ${PTHREAD_LIBRARY})
	include_directories(${CMAKE_SOURCE_DIR}/../pthreads)
	link_libraries(${PTHREAD_LIBRARY})
endif ()
 
include_directories(.)

include_directories(libairspy-1.0.9)
add_subdirectory (libairspy-1.0.9)


#
# merge all the RC files in only one resource.rc
#
ADD_CUSTOM_COMMAND(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
	COMMAND cat ${CMAKE_SOURCE_DIR}/util/logw.rc > ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
	COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/airspyw.rc  >> ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
	
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/airspyw.rc ${CMAKE_SOURCE_DIR}/util/logw.rc
)

SET(AIRSPY_SRCS
					extio_airspy.cpp  
					airspy.cpp
					gui_control.cpp  
					gui_splashscreen.cpp  
					${CMAKE_CURRENT_BINARY_DIR}/resource.rc
)


link_libraries(extiobase)
link_libraries(util)
link_libraries(guiutil)
link_libraries(libairspy-1.0.9)

add_library(extio_airspy_cmake_mgw SHARED ${AIRSPY_SRCS} )

#
# set the property in oder to avoid cmake generate -Wl,B=dynamic
# see http://www.cmake.org/pipermail/cmake/2011-March/043538.html
#
set_property(TARGET extio_airspy_cmake_mgw PROPERTY LINK_SEARCH_START_STATIC 1)

#target_link_libraries(extio_perseus_cmake_mgw ws2_32 )
#target_link_libraries(extio_perseus_cmake_mgw iphlpapi)

target_link_libraries(extio_airspy_cmake_mgw ${AIRSPY_LIBRARY} ${LIBUSB10_LIBRARY}    )
set_target_properties(extio_airspy_cmake_mgw
                      PROPERTIES
					  PREFIX "" # remove the trailing standard 'lib' prefix
					  OUTPUT_NAME "extio_airspy_cmake_mgw-${VERSION}"
                      )
###########################################################################



#message ( "CXX_CFLAGS:                "  ${CMAKE_CXX_FLAGS})			
#message ( "CMAKE_SHARED_LINKER_FLAGS: "  ${CMAKE_SHARED_LINKER_FLAGS})			

GENERATE_EXPORT_HEADER( extio_airspy_cmake_mgw
             BASE_NAME extio_airspy_cmake_mgw
             EXPORT_MACRO_NAME extio_airspy_cmake_mgw_EXPORT
             EXPORT_FILE_NAME MyLibrary_Export.h
             STATIC_DEFINE extio_airspy_cmake_mgw_BUILT_AS_STATIC
)

#
# copy in selected locations 
#
add_custom_target(dist_airspy
	COMMAND cp ${CMAKE_BINARY_DIR}/airspy/$<TARGET_FILE_NAME:extio_airspy_cmake_mgw> /c/Users/Public/HDSDR/
	COMMAND cp ${CMAKE_BINARY_DIR}/airspy/$<TARGET_FILE_NAME:extio_airspy_cmake_mgw> /c/Users/Public/Studio1_1.6/
)

