# wxMathPlot Sample2 CMakeLists.txt
#
# Author: Davide Rondini
# Last Update: 2008-02-16
# License: wxWindows license

add_executable(mpSample2 mp2.cpp)
# Under Windows, the target type must be WINDOWS, not CONSOLE
if(WIN32)
	set_target_properties(mpSample2 PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endif(WIN32)
target_link_libraries(mpSample2 ${wxWidgets_LIBRARIES} mathplot)
