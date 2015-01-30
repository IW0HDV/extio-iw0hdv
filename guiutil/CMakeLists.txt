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

SET(GUIUTIL_SRCS
		gui.cpp  
		guiutil.cpp    
		gui_impl.cpp    
)

add_library(guiutil STATIC ${GUIUTIL_SRCS} )
