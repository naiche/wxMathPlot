# Main wxMathPlot CMakeLists.txt
# Manage project
#
# Author: Davide Rondini
# Last Update: 2008-02-20
# License: wxWindows license

project(wxMathPlot)

# Create an option to be chosen by the user:
if(UNIX)
    SET(LINUX_64_32_CROSSCOMPILE OFF CACHE BOOL "Cross compile from Linux x86_64 to Linux x86?")
    SET(GDB_DEBUG OFF CACHE BOOL "Build with gdb debugger support?")
endif(UNIX)

if(LINUX_64_32_CROSSCOMPILE)
    message(STATUS "Cross compiling from Linux x86_64 to Linux x86")
else(LINUX_64_32_CROSSCOMPILE)
    message(STATUS "Native build")
endif(LINUX_64_32_CROSSCOMPILE)

SET(wxWidgets_USE_LIBS base core)
find_package(wxWidgets)
if(wxWidgets_FOUND)
    include(${wxWidgets_USE_FILE})
    add_library(mathplot mathplot.cpp mathplot.h)
    if(LINUX_64_32_CROSSCOMPILE)
        set_source_files_properties(mathplot.cpp PROPERTIES COMPILE_FLAGS "-m32" )
        set_target_properties(mathplot PROPERTIES LINK_FLAGS "-m32")
    endif(LINUX_64_32_CROSSCOMPILE)
    if(GDB_DEBUG)
        set_source_files_properties(mathplot.cpp PROPERTIES COMPILE_FLAGS "-g -ggdb" )
        set_target_properties(mathplot PROPERTIES LINK_FLAGS "-g -ggdb")
    endif(GDB_DEBUG)
    if(LINUX_64_32_CROSSCOMPILE AND GDB_DEBUG)
        set_source_files_properties(mathplot.cpp PROPERTIES COMPILE_FLAGS "-m32 -g -ggdb" )
        set_target_properties(mathplot PROPERTIES LINK_FLAGS "-m32  -g -ggdb")
    endif(LINUX_64_32_CROSSCOMPILE AND GDB_DEBUG)
    target_link_libraries(mathplot ${wxWidgets_LIBRARIES})

    # Compile samples?
    SET( WXMATHPLOT_BUILD_EXAMPLES ON CACHE BOOL "Build examples?")
    IF(WXMATHPLOT_BUILD_EXAMPLES)
        add_subdirectory(samples)
    ENDIF(WXMATHPLOT_BUILD_EXAMPLES)
else(wxWidgets_FOUND)
    MESSAGE("wxWidgets not found!")
endif(wxWidgets_FOUND)
