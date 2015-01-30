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

find_library(PTHREAD_LIBRARY
    NAMES libpthreadGC2.a
	PATH_SUFFIXES .a
	PATHS ${CMAKE_SOURCE_DIR}/../pthreads
	NO_DEFAULT_PATH
)
if (NOT PTHREAD_LIBRARY)
	message( FATAL_ERROR "pthread not found")
else ()
	#message ( "pthread found: "  ${PTHREAD_LIBRARY})
	include_directories(${CMAKE_SOURCE_DIR}/../pthreads)
	link_libraries(${PTHREAD_LIBRARY})
endif ()




#
# merge all the RC files in only one resource.rc
#
ADD_CUSTOM_COMMAND(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
	COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/hpsdrw.rc > ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
	COMMAND cat ${CMAKE_SOURCE_DIR}/util/logw.rc >> ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
	
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/hpsdrw.rc ${CMAKE_SOURCE_DIR}/util/logw.rc
)

include_directories(.)

SET(HPSDR_SRCS
					extio_hpsdr.cpp
					gui_splashscreen.cpp  
					gui_control.cpp
					hpsdr.cpp
					intradllcomm.cpp  
					${CMAKE_CURRENT_BINARY_DIR}/resource.rc
)

link_libraries(extiobase)
link_libraries(util)
link_libraries(guiutil)

link_libraries(ws2_32 iphlpapi)

add_library(extio_hpsdr_cmake_mgw SHARED ${HPSDR_SRCS} )
## extio_hermes_mgw.def )

#message ( "CXX_CFLAGS:                "  ${CMAKE_CXX_FLAGS})			
#message ( "CMAKE_SHARED_LINKER_FLAGS: "  ${CMAKE_SHARED_LINKER_FLAGS})			

GENERATE_EXPORT_HEADER( extio_hpsdr_cmake_mgw
             BASE_NAME extio_hpsdr_cmake_mgw
             EXPORT_MACRO_NAME extio_hpsdr_cmake_mgw_EXPORT
             EXPORT_FILE_NAME MyLibrary_Export.h
             STATIC_DEFINE extio_hpsdr_cmake_mgw_BUILT_AS_STATIC
)

#
# copy in selected locations 
#
add_custom_target(dist_hpsdr
	COMMAND cp ${CMAKE_BINARY_DIR}/hpsdr/libextio_hpsdr_cmake_mgw.dll /c/Users/andrew/HDSDR_270/extio_hpsdr_cmake_mgw.dll
	COMMAND cp ${CMAKE_BINARY_DIR}/hpsdr/libextio_hpsdr_cmake_mgw.dll /c/Users/andrew/Studio1_1.6/extio_hpsdr_cmake_mgw.dll
)

