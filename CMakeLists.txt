cmake_minimum_required(VERSION 3.14)

if(TARGET ssLogger)
    return()
endif()

set (CMAKE_CXX_STANDARD 11)

# For Clang to do parsing
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set the project name
project(ssLoggerProject)

# ==========================================================
# Variables 
# ==========================================================
#set(CMAKE_FIND_DEBUG_MODE 1)
set(ssLOG_OUTPUT_STATIC 1 CACHE BOOL "Use and create static library instead")
set(ssLOG_USE_SOURCE 1 CACHE BOOL "Use source file instead of header only files")
set(ssLOG_BUILD_TESTS 1 CACHE BOOL "If true, this will build tests")

set(ssLOG_CALL_STACK 1 CACHE BOOL "Show call stack for all logged functions")
set(ssLOG_LOG_WITH_ASCII 0 CACHE BOOL "Call stack logging will be shown using ASCII characters")
set(ssLOG_SHOW_FILE_NAME 1 CACHE BOOL "Show file name for all logged functions")
set(ssLOG_SHOW_LINE_NUM 1 CACHE BOOL "Show line number for all logged functions")
set(ssLOG_SHOW_FUNC_NAME 1 CACHE BOOL "Show function name for all logged functions")
set(ssLOG_SHOW_DATE 1 CACHE BOOL "Show log date for all logged functions")
set(ssLOG_SHOW_TIME 1 CACHE BOOL "Show log time for all logged functions")
set(ssLOG_THREAD_SAFE 1 CACHE BOOL "Use std::thread and ensure thread safety for all logged functions")
set(ssLOG_LOG_TO_FILE 0 CACHE BOOL "Log to file instead for all logged functions")
set(ssLOG_LEVEL "WARNING" CACHE STRING 
"Log level (NONE, FATAL, ERROR, WARNING, INFO, DEBUG)
NONE     (None of the levels will be printed, but will still print normal ssLOG_LINE or ssLOG_FUNC)
FATAL    (Program will crash)
ERROR    (Program might crash and **likely** to not function correctly)
WARNING  (Program won't crash but **might** not function correctly)
INFO     (Program state which **doesn't** spam the log)
DEBUG    (Program state which **does** spam the log)")

set_property(CACHE ssLOG_LEVEL PROPERTY STRINGS "NONE" 
                                                "FATAL" 
                                                "ERROR" 
                                                "WARNING" 
                                                "INFO" 
                                                "DEBUG")

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(STANDARD_COMPILE_FLAGS "/utf-8;/W1;_CRT_SECURE_NO_WARNINGS")
else()
    set(STANDARD_COMPILE_FLAGS  "-Wall"
                                "-Wno-return-local-addr"
                                "-Wno-sign-compare"
                                #"-Wno-unused-variable"
                                #"-Wno-unused-but-set-variable"
                                "-Wno-unused-parameter"
                                "-Wno-switch"
                                "-Wno-gnu-zero-variadic-macro-arguments"
                                "-Wextra"
                                "-pedantic"
                                "-Werror")
endif()

# ==========================================================
# ssLogger for external use
# ==========================================================
if("${ssLOG_USE_SOURCE}")
    if("${ssLOG_OUTPUT_STATIC}")
        add_library(ssLogger STATIC 
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/ssLogInit.cpp"
        )
    else()
        add_library(ssLogger OBJECT
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/ssLogInit.cpp"
        )
    endif()

    target_include_directories(ssLogger PUBLIC  "${CMAKE_CURRENT_LIST_DIR}/Include"
                                                "${CMAKE_CURRENT_LIST_DIR}/External/termcolor/include")
    

    target_compile_definitions(ssLogger PUBLIC ssLOG_CALL_STACK=$<BOOL:${ssLOG_CALL_STACK}>)
    target_compile_definitions(ssLogger PUBLIC ssLOG_ASCII=$<BOOL:${ssLOG_LOG_WITH_ASCII}>)
    target_compile_definitions(ssLogger PUBLIC ssLOG_SHOW_FILE_NAME=$<BOOL:${ssLOG_SHOW_FILE_NAME}>)
    target_compile_definitions(ssLogger PUBLIC ssLOG_SHOW_LINE_NUM=$<BOOL:${ssLOG_SHOW_LINE_NUM}>)
    target_compile_definitions(ssLogger PUBLIC ssLOG_SHOW_FUNC_NAME=$<BOOL:${ssLOG_SHOW_FUNC_NAME}>)
    target_compile_definitions(ssLogger PUBLIC ssLOG_SHOW_DATE=$<BOOL:${ssLOG_SHOW_DATE}>)
    target_compile_definitions(ssLogger PUBLIC ssLOG_SHOW_TIME=$<BOOL:${ssLOG_SHOW_TIME}>)
    target_compile_definitions(ssLogger PUBLIC ssLOG_THREAD_SAFE=$<BOOL:${ssLOG_THREAD_SAFE}>)
    target_compile_definitions(ssLogger PUBLIC ssLOG_LOG_TO_FILE=$<BOOL:${ssLOG_LOG_TO_FILE}>)
    target_compile_definitions(ssLogger PUBLIC ssLOG_USE_SOURCE)

    if(${ssLOG_LEVEL} STREQUAL "NONE")
        target_compile_definitions(ssLogger PUBLIC ssLOG_LEVEL=0)
    elseif(${ssLOG_LEVEL} STREQUAL "FATAL")
        target_compile_definitions(ssLogger PUBLIC ssLOG_LEVEL=1)
    elseif(${ssLOG_LEVEL} STREQUAL "ERROR")
        target_compile_definitions(ssLogger PUBLIC ssLOG_LEVEL=2)
    elseif(${ssLOG_LEVEL} STREQUAL "WARNING")
        target_compile_definitions(ssLogger PUBLIC ssLOG_LEVEL=3)
    elseif(${ssLOG_LEVEL} STREQUAL "INFO")
        target_compile_definitions(ssLogger PUBLIC ssLOG_LEVEL=4)
    elseif(${ssLOG_LEVEL} STREQUAL "DEBUG")
        target_compile_definitions(ssLogger PUBLIC ssLOG_LEVEL=5)
    endif()

    #message("ssLOG_CALL_STACK: ${ssLOG_CALL_STACK}")
    #message("ssLOG_SHOW_FILE_NAME: ${ssLOG_SHOW_FILE_NAME}")

    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_definitions(ssLogger PUBLIC ${_CRT_SECURE_NO_WARNINGS})
    else()
        target_compile_options(ssLogger PRIVATE ${STANDARD_COMPILE_FLAGS})
    endif()

else()
    add_library(ssLogger INTERFACE)
    target_include_directories(ssLogger INTERFACE "${CMAKE_CURRENT_LIST_DIR}/Include")
    
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_definitions(ssLogger INTERFACE ${STANDARD_COMPILE_FLAGS})
    else()
        target_compile_options(ssLogger INTERFACE ${STANDARD_COMPILE_FLAGS})
    endif()
endif()

if(${ssLOG_BUILD_TESTS})

    # ==========================================================
    # ssLogger for tests
    # ==========================================================

    # Build the library for testing
    if("${ssLOG_OUTPUT_STATIC}")
        add_library(ssLogger_SRC STATIC 
            "${CMAKE_CURRENT_SOURCE_DIR}/Src/ssLogInit.cpp"
        )
    else()
        add_library(ssLogger_SRC OBJECT
            "${CMAKE_CURRENT_SOURCE_DIR}/Src/ssLogInit.cpp"
        )
    endif()
    
    # target_sources(ssLogger_SRC PUBLIC "${PROJECT_SOURCE_DIR}/ssLogInit.cpp")
    target_include_directories(ssLogger_SRC PUBLIC  "${CMAKE_CURRENT_LIST_DIR}/Include"
                                                    "${CMAKE_CURRENT_LIST_DIR}/External/termcolor/include")

    target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_CALL_STACK=$<BOOL:${ssLOG_CALL_STACK}>)
    target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_ASCII=$<BOOL:${ssLOG_LOG_WITH_ASCII}>)
    target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_SHOW_FILE_NAME=$<BOOL:${ssLOG_SHOW_FILE_NAME}>)
    target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_SHOW_LINE_NUM=$<BOOL:${ssLOG_SHOW_LINE_NUM}>)
    target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_SHOW_FUNC_NAME=$<BOOL:${ssLOG_SHOW_FUNC_NAME}>)
    target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_SHOW_DATE=$<BOOL:${ssLOG_SHOW_DAT}>)
    target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_SHOW_TIME=$<BOOL:${ssLOG_SHOW_TIME}>)
    target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_THREAD_SAFE=$<BOOL:${ssLOG_THREAD_SAFE}>)
    target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_LOG_TO_FILE=$<BOOL:${ssLOG_LOG_TO_FILE}>)
    target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_USE_SOURCE)

    if(${ssLOG_LEVEL} STREQUAL "NONE")
        target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_LEVEL=0)
    elseif(${ssLOG_LEVEL} STREQUAL "FATAL")
        target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_LEVEL=1)
    elseif(${ssLOG_LEVEL} STREQUAL "ERROR")
        target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_LEVEL=2)
    elseif(${ssLOG_LEVEL} STREQUAL "WARNING")
        target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_LEVEL=3)
    elseif(${ssLOG_LEVEL} STREQUAL "INFO")
        target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_LEVEL=4)
    elseif(${ssLOG_LEVEL} STREQUAL "DEBUG")
        target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_LEVEL=5)
    endif()


    add_library(ssLogger_HEAD INTERFACE)
    target_include_directories(ssLogger_HEAD INTERFACE  "${CMAKE_CURRENT_LIST_DIR}/Include"
                                                        "${CMAKE_CURRENT_LIST_DIR}/External/termcolor/include")
    #target_sources(ssLogger_HEAD INTERFACE
    #                        "${CMAKE_CURRENT_LIST_DIR}/Include/ssLogger/ssLog.hpp"
    #                        "${CMAKE_CURRENT_LIST_DIR}/Include/ssLogger/ssLogInit.hpp"
    #                        "${CMAKE_CURRENT_LIST_DIR}/Include/ssLogger/ssLogSwitches.hpp"
    #                        )

    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_definitions(ssLogger_SRC PUBLIC ${STANDARD_COMPILE_FLAGS})
        target_compile_definitions(ssLogger_HEAD INTERFACE ${STANDARD_COMPILE_FLAGS})
    else()
        target_compile_options(ssLogger_SRC PRIVATE ${STANDARD_COMPILE_FLAGS})
    endif()

    # ==========================================================
    # Tests
    # ==========================================================

    function(AddFilesToExecutables folderToExe binSubFolder targetsToLink exeSuffix)
    
        file(GLOB TEST_CPP_FILES "${folderToExe}/*.cpp")
        
        foreach(CUR_SOURCE_PATH ${TEST_CPP_FILES})
            # Get the file name without path and extension
            get_filename_component(EXECUTABLE_NAME "${CUR_SOURCE_PATH}" NAME_WE)    

            #message("EXECUTABLE_NAME: ${EXECUTABLE_NAME}")
            #message("exeSuffix: ${exeSuffix}")
            #message("CUR_SOURCE_PATH: ${CUR_SOURCE_PATH}")
            #message("targetsToLink: ${targetsToLink}\n")

            # Add executable
            add_executable("${EXECUTABLE_NAME}${exeSuffix}" "${CUR_SOURCE_PATH}")

            target_link_libraries("${EXECUTABLE_NAME}${exeSuffix}" PUBLIC ${targetsToLink})

            # Sub-directory for build
            set_target_properties("${EXECUTABLE_NAME}${exeSuffix}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${binSubFolder})

            if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
                #TODO
            else()
                target_compile_options("${EXECUTABLE_NAME}${exeSuffix}" PRIVATE -Wall -Wpedantic -Werror)
            endif()

        endforeach(CUR_SOURCE_PATH "${TEST_CPP_FILES}")
    
    endfunction()
    
    # Get all cpp test files
    #file(GLOB TEST_CPP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Src/Tests/*cpp")

    #get_target_property(MY_TARGET_SOURCES ssLogger_SRC SOURCES)
    
    #message("MY_TARGET_SOURCES: ${MY_TARGET_SOURCES}")

    AddFilesToExecutables(  "${CMAKE_CURRENT_SOURCE_DIR}/Src/Tests/"
                            "${CMAKE_CURRENT_BINARY_DIR}/SourceTests/"
                            ssLogger_SRC
                            "Source")

    AddFilesToExecutables(  "${CMAKE_CURRENT_SOURCE_DIR}/Src/Tests/"
                            "${CMAKE_CURRENT_BINARY_DIR}/HeaderOnlyTests/"
                            ssLogger_HEAD
                            "Header_Only")

endif()