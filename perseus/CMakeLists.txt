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

find_library(PERSEUS_SDR_LIBRARY
	NAMES libperseus-sdr.a
	#PATH_SUFFIXES .a
    PATHS c:/mingw/lib
	NO_DEFAULT_PATH

)
if (NOT PERSEUS_SDR_LIBRARY)
	message( FATAL_ERROR "perseus lib not found")
else ()
	#message ( "libperseus-sdr found: "  ${PERSEUS_SDR_LIBRARY} )	
	##set (XXX_LIB_P ${PERSEUS_SDR_LIBRARY})
endif ()
 
find_library(LIBUSB10_LIBRARY
    NAMES libusb-1.0.a
	#PATH_SUFFIXES .a
	PATHS c:/mingw/lib
	NO_DEFAULT_PATH
)
if (NOT LIBUSB10_LIBRARY)
	message( FATAL_ERROR "libusb-1.0 not found")
else ()
	#message ( "libusb-1.0 found: "  ${LIBUSB10_LIBRARY})
	#link_libraries(${LIBUSB10_LIBRARY})
endif ()

include_directories(.)

#
# merge all the RC files in only one resource.rc
#
ADD_CUSTOM_COMMAND(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
	COMMAND cat ${CMAKE_SOURCE_DIR}/util/logw.rc > ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
	COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/perseusw.rc  >> ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
	
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/perseusw.rc ${CMAKE_SOURCE_DIR}/util/logw.rc
)


SET(PERSEUS_SRCS
                    extio_perseus.cpp  
                    perseus.cpp
                    gui_control.cpp  
                    gui_splashscreen.cpp  
                    ${CMAKE_CURRENT_BINARY_DIR}/resource.rc
)


link_libraries(extiobase)
link_libraries(util)
link_libraries(guiutil)

add_library(extio_perseus_cmake_mgw SHARED ${PERSEUS_SRCS} )

#
# set the property in oder to avoid cmake generate -Wl,B=dynamic
# see http://www.cmake.org/pipermail/cmake/2011-March/043538.html
#
set_property(TARGET extio_perseus_cmake_mgw PROPERTY LINK_SEARCH_START_STATIC 1)

#target_link_libraries(extio_perseus_cmake_mgw ws2_32 )
#target_link_libraries(extio_perseus_cmake_mgw iphlpapi)

target_link_libraries(extio_perseus_cmake_mgw ${PERSEUS_SDR_LIBRARY} ${LIBUSB10_LIBRARY}    )

###########################################################################



#message ( "CXX_CFLAGS:                "  ${CMAKE_CXX_FLAGS})			
#message ( "CMAKE_SHARED_LINKER_FLAGS: "  ${CMAKE_SHARED_LINKER_FLAGS})			

GENERATE_EXPORT_HEADER( extio_perseus_cmake_mgw
             BASE_NAME extio_perseus_cmake_mgw
             EXPORT_MACRO_NAME extio_perseus_cmake_mgw_EXPORT
             EXPORT_FILE_NAME MyLibrary_Export.h
             STATIC_DEFINE extio_perseus_cmake_mgw_BUILT_AS_STATIC
)

#
# copy in selected locations 
#
add_custom_target(dist_perseus
	COMMAND cp ${CMAKE_BINARY_DIR}/perseus/libextio_perseus_cmake_mgw.dll /c/Users/andrew/HDSDR_270/extio_perseus_cmake_mgw.dll
	COMMAND cp ${CMAKE_BINARY_DIR}/perseus/libextio_perseus_cmake_mgw.dll /c/Users/andrew/Studio1_1.6/extio_perseus_cmake_mgw.dll
)

