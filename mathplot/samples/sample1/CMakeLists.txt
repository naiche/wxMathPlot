# wxMathPlot Sample1 CMakeLists.txt
#
# Author: Davide Rondini
# Last Update: 2008-02-20
# License: wxWindows license

add_executable(mpSample1 mp1.cpp)
# Under Visual C++, the target type must be WINDOWS, not CONSOLE
if(WIN32 AND MSVC)
	set_target_properties(mpSample1 PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
endif(WIN32 AND MSVC)
# Linux cross compiling to x86 from x86_64 distribution
if(LINUX_64_32_CROSSCOMPILE)
#     set(CMAKE_LIBRARY_PATH "/usr/lib")
#     set(CMAKE_SYSTEM_LIBRARY_PATH "/usr/lib")
#     set(CMAKE_SYSTEM_PREFIX_PATH "/usr")
    set_source_files_properties(mp1.cpp PROPERTIES COMPILE_FLAGS "-m32 -DwxSIZE_T_IS_UINT")
    set_target_properties(mpSample1 PROPERTIES LINK_FLAGS "-m32")
endif(LINUX_64_32_CROSSCOMPILE)

target_link_libraries(mpSample1 ${wxWidgets_LIBRARIES} mathplot)
