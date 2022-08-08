set(INSTALL_DIR "" CACHE STRING "Install directory for configured files")
set(PROGRAM_VERSION "" CACHE STRING "Version override for build info")

set(USING_GIT_VERSION FALSE)

if(NOT INSTALL_DIR)
    message(FATAL_ERROR "Install directory not specified")
endif()

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    if(NOT PROGRAM_VERSION)
        execute_process(COMMAND ${GIT_EXECUTABLE} describe --always --dirty --abbrev=7 --exclude "*"
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        OUTPUT_STRIP_TRAILING_WHITESPACE
                        RESULT_VARIABLE GIT_VERSION_RESULT
                        OUTPUT_VARIABLE PROGRAM_VERSION)
        if (NOT GIT_VERSION_RESULT EQUAL "0")
            message(FATAL_ERROR "git describe --always --dirty --abbrev=7 --exclude \"*\" failed with ${GIT_VERSION_RESULT}")
        else()
            set(USING_GIT_VERSION TRUE)
        endif()
    endif()
endif()

if(NOT PROGRAM_VERSION)
    message(FATAL_ERROR "Program version not defined")
else()
    if (USING_GIT_VERSION)
        message(STATUS "Using git program version: " ${PROGRAM_VERSION})
    else()
        message(STATUS "Using overrided program version: " ${PROGRAM_VERSION})
    endif()
endif()

configure_file(build_info.h.in "${INSTALL_DIR}/build_info.h" @ONLY)

unset(INSTALL_DIR CACHE)
unset(PROGRAM_VERSION CACHE)
