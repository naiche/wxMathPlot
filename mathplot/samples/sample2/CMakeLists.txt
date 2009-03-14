# wxMathPlot Sample2 CMakeLists.txt
#
# Author: Davide Rondini
# Last Update: 2008-02-20
# License: wxWindows license

add_executable(mpSample2 mp2.cpp)
# Under Windows, the target type must be WINDOWS, not CONSOLE
if(WIN32 AND MSVC)
	set_target_properties(mpSample2 PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endif(WIN32 AND MSVC)
# Linux cross compiling to x86 from x86_64 distribution
if(LINUX_64_32_CROSSCOMPILE)
    set_source_files_properties(mp2.cpp PROPERTIES COMPILE_FLAGS "-m32 -DwxSIZE_T_IS_UINT")
    set_target_properties(mpSample2 PROPERTIES LINK_FLAGS "-m32")
endif(LINUX_64_32_CROSSCOMPILE)
target_link_libraries(mpSample2 ${wxWidgets_LIBRARIES} mathplot)
