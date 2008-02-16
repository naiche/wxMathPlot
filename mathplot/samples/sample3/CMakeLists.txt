# wxMathPlot Sample3 CMakeLists.txt
#
# Author: Davide Rondini
# Last Update: 2008-02-16
# License: wxWindows license

add_executable(mpSample3 sample3.cpp)
# Under Windows, the target type must be WINDOWS, not CONSOLE
if(WIN32)
	set_target_properties(mpSample3 PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endif(WIN32)
target_link_libraries(mpSample3 ${wxWidgets_LIBRARIES} mathplot)
