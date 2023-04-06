cmake_minimum_required(VERSION 3.14)

set (CMAKE_CXX_STANDARD 11)

# For Clang to do parsing
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set the project name
project(ssLoggerProject)

# ==========================================================
# Variables 
# ==========================================================
set(CMAKE_FIND_DEBUG_MODE 1)
set(ssLOG_OUTPUT_STATIC "ON" CACHE BOOL "Use and create static library instead")
set(ssLOG_USE_SOURCE "ON" CACHE BOOL "Use source file instead of header only files")
set(ssLOG_BUILD_TESTS "ON" CACHE BOOL "If true, this will build tests")

set(ssLOG_CALL_STACK "ON" CACHE BOOL "Show call stack for all logged functions")
set(ssLOG_LOG_WITH_ASCII "OFF" CACHE BOOL "Call stack logging will be shown using ASCII characters")
set(ssLOG_SHOW_FILE_NAME "ON" CACHE BOOL "Show file name for all logged functions")
set(ssLOG_SHOW_LINE_NUM "ON" CACHE BOOL "Show line number for all logged functions")
set(ssLOG_SHOW_FUNC_NAME "ON" CACHE BOOL "Show function name for all logged functions")
set(ssLOG_SHOW_TIME "ON" CACHE BOOL "Show log time for all logged functions")
set(ssLOG_THREAD_SAFE "ON" CACHE BOOL "Use std::thread and ensure thread safety for all logged functions")
set(ssLOG_LOG_TO_FILE "OFF" CACHE BOOL "Log to file instead for all logged functions")

set(ssLOG_INCLUDE_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/include")


#Find shared libraries in the same location as the executable
# SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath -Wl,$ORIGIN")
# set(CMAKE_CXX_FLAGS "-fpermissive -std=c++0x")  #For surpressing goto warning

# ==========================================================
# ssLogger for external use
# ==========================================================
if("${ssLOG_USE_SOURCE}")
    if("${ssLOG_OUTPUT_STATIC}")
        add_library(ssLogger STATIC 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ssLogInit.cpp"
        )
    else()
        add_library(ssLogger SHARED
        "${CMAKE_CURRENT_SOURCE_DIR}/src/ssLogInit.cpp"
        )
    endif()

    target_include_directories(ssLogger PUBLIC "${ssLOG_INCLUDE_LOCATION}")

    if(ssLOG_CALL_STACK)
        target_compile_definitions(ssLogger PUBLIC ssLOG_CALL_STACK=1)
    else()
        target_compile_definitions(ssLogger PUBLIC ssLOG_CALL_STACK=0)
    endif()
    
    if(ssLOG_LOG_WITH_ASCII)
        target_compile_definitions(ssLogger PUBLIC ssLOG_ASCII=1)
    else()
        target_compile_definitions(ssLogger PUBLIC ssLOG_ASCII=0)
    endif()

    if(ssLOG_SHOW_FILE_NAME)
        target_compile_definitions(ssLogger PUBLIC ssLOG_SHOW_FILE_NAME=1)
    else()
        target_compile_definitions(ssLogger PUBLIC ssLOG_SHOW_FILE_NAME=0)
    endif()

    if(ssLOG_SHOW_LINE_NUM)
        target_compile_definitions(ssLogger PUBLIC ssLOG_SHOW_LINE_NUM=1)
    else()
        target_compile_definitions(ssLogger PUBLIC ssLOG_SHOW_LINE_NUM=0)
    endif()

    if(ssLOG_SHOW_FUNC_NAME)
        target_compile_definitions(ssLogger PUBLIC ssLOG_SHOW_FUNC_NAME=1)
    else()
        target_compile_definitions(ssLogger PUBLIC ssLOG_SHOW_FUNC_NAME=0)
    endif()

    if(ssLOG_SHOW_TIME)
        target_compile_definitions(ssLogger PUBLIC ssLOG_SHOW_TIME=1)
    else()
        target_compile_definitions(ssLogger PUBLIC ssLOG_SHOW_TIME=0)
    endif()

    if(ssLOG_THREAD_SAFE)
        target_compile_definitions(ssLogger PUBLIC ssLOG_THREAD_SAFE=1)
    else()
        target_compile_definitions(ssLogger PUBLIC ssLOG_THREAD_SAFE=0)
    endif()

    if(ssLOG_LOG_TO_FILE)
        target_compile_definitions(ssLogger PUBLIC ssLOG_LOG_TO_FILE=1)
    else()
        target_compile_definitions(ssLogger PUBLIC ssLOG_LOG_TO_FILE=0)
    endif()

    target_compile_definitions(ssLogger PUBLIC ssLOG_USE_SOURCE)
else()
    add_library(ssLogger INTERFACE)
    target_include_directories(ssLogger INTERFACE "${ssLOG_INCLUDE_LOCATION}")
    target_sources(ssLogger INTERFACE
                            "${ssLOG_INCLUDE_LOCATION}/ssLogger/ssLog.hpp"
                            "${ssLOG_INCLUDE_LOCATION}/ssLogger/ssLogInit.hpp"
                            "${ssLOG_INCLUDE_LOCATION}/ssLogger/ssLogSwitches.hpp"
                            )
    
    # DEBUGGING
    # get_target_property(MY_PROJECT_SOURCES ssLogger INTERFACE_SOURCES)

    # foreach(src ${MY_PROJECT_SOURCES})
    #     message(STATUS "src='${src}'")
    # endforeach()
endif()

# DEBUGGING
# get_target_property(DEFS ssLogger COMPILE_DEFINITIONS)

# foreach(src ${DEFS})
#     message(STATUS "def='${src}'")
# endforeach()

# If you are on 32 bit linux, uncomment this
# set (CMAKE_CXX_FLAGS "-m32")
# set (CMAKE_C_FLAGS "-m32")


if(${ssLOG_BUILD_TESTS})

    # ==========================================================
    # ssLogger for tests
    # ==========================================================

    # # Build the library for testing
    # add_library(ssLogger_SRC STATIC 
    #     "${CMAKE_CURRENT_SOURCE_DIR}/src/ssLogInit.cpp"
    # )
    # # target_sources(ssLogger_SRC PUBLIC "${PROJECT_SOURCE_DIR}/ssLogInit.cpp")
    # target_include_directories(ssLogger_SRC PUBLIC "${ssLOG_INCLUDE_LOCATION}")
    # target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_CALL_STACK=0)
    # target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_SHOW_FILE_NAME=1)
    # target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_SHOW_LINE_NUM=1)
    # target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_SHOW_FUNC_NAME=1)
    # target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_SHOW_TIME=1)
    # target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_THREAD_SAFE=0)
    # target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_LOG_TO_FILE=1)
    # target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_USE_SOURCE)


    add_library(ssLogger_HEAD INTERFACE)
    target_include_directories(ssLogger_HEAD INTERFACE "${ssLOG_INCLUDE_LOCATION}")
    target_sources(ssLogger_HEAD INTERFACE
                            "${ssLOG_INCLUDE_LOCATION}/ssLogger/ssLog.hpp"
                            "${ssLOG_INCLUDE_LOCATION}/ssLogger/ssLogInit.hpp"
                            "${ssLOG_INCLUDE_LOCATION}/ssLogger/ssLogSwitches.hpp"
                            )

    # ==========================================================
    # Header Only Tests
    # ==========================================================

    # Get all cpp test files
    file(GLOB TEST_CPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/headerOnlyTests/*cpp")

    foreach(CUR_SOURCE_PATH ${TEST_CPP_FILES})
        # Get the file name without path and extension
        get_filename_component(EXECUTABLE_NAME "${CUR_SOURCE_PATH}" NAME_WE)    

        # message("${EXECUTABLE_NAME}\n")

        # Add executable
        add_executable("${EXECUTABLE_NAME}_hdo" "${CUR_SOURCE_PATH}")

        target_link_libraries("${EXECUTABLE_NAME}_hdo" PUBLIC ssLogger_HEAD)

        # Sub-directory for build
        set_target_properties("${EXECUTABLE_NAME}_hdo" PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/headerOnlyTests/${EXECUTABLE_NAME}")

    endforeach(CUR_SOURCE_PATH "${TEST_CPP_FILES}")

    # ==========================================================
    # Soruce Tests
    # ==========================================================

    # Get all cpp test files
    set(TEST_CPP_FILES "" STRING)
    file(GLOB TEST_CPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/sourceTests/*cpp")

    foreach(CUR_SOURCE_PATH ${TEST_CPP_FILES})
        # Get the file name without path and extension
        get_filename_component(EXECUTABLE_NAME "${CUR_SOURCE_PATH}" NAME_WE)    

        # DEBUGGING
        # message("${EXECUTABLE_NAME}\n")

        # Add executable
        add_executable("${EXECUTABLE_NAME}_src" "${CUR_SOURCE_PATH}")

        target_link_libraries("${EXECUTABLE_NAME}_src" PUBLIC ssLogger)

        # Sub-directory for build
        set_target_properties("${EXECUTABLE_NAME}_src" PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/sourceTests/${EXECUTABLE_NAME}")

    endforeach(CUR_SOURCE_PATH "${TEST_CPP_FILES}")

endif()