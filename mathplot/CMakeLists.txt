# Main wxMathPlot CMakeLists.txt
# Manage project
#
# Author: Davide Rondini
# Last Update: 2008-02-16
# License: wxWindows license

project(wxMathPlot)

SET(wxWidgets_USE_LIBS base core)
find_package(wxWidgets)
if(wxWidgets_FOUND)
	include(${wxWidgets_USE_FILE})
    add_library(mathplot mathplot.cpp mathplot.h)
	target_link_libraries(mathplot ${wxWidgets_LIBRARIES})
	add_subdirectory(samples)
else(wxWidgets_FOUND)
   MESSAGE("wxWidgets not found!")
endif(wxWidgets_FOUND)