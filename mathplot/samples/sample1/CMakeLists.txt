# wxMathPlot Sample1 CMakeLists.txt
#
# Author: Davide Rondini
# Last Update: 2008-02-16
# License: wxWindows license

add_executable(mpSample1 mp1.cpp)
# Under Windows, the target type must be WINDOWS, not CONSOLE
if(WIN32)
	set_target_properties(mpSample1 PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endif(WIN32)
target_link_libraries(mpSample1 ${wxWidgets_LIBRARIES} mathplot)
