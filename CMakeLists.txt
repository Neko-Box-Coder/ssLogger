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
set(ssLOG_BUILD_TYPE "STATIC" CACHE STRING "Build type for ssLogger (STATIC, SHARED, OBJECT)")
set_property(CACHE ssLOG_BUILD_TYPE PROPERTY STRINGS    "STATIC" 
                                                        "SHARED" 
                                                        "OBJECT")

set(ssLOG_USE_SOURCE 1 CACHE BOOL "Use source file instead of header only files")
set(ssLOG_BUILD_TESTS 1 CACHE BOOL "If true, this will build tests")

set(ssLOG_CALL_STACK 1 CACHE BOOL "Show call stack for all logged functions")
set(ssLOG_LOG_WITH_ASCII 0 CACHE BOOL "Call stack logging will be shown using ASCII characters")
set(ssLOG_SHOW_FILE_NAME 1 CACHE BOOL "Show file name for all logged functions")
set(ssLOG_SHOW_LINE_NUM 1 CACHE BOOL "Show line number for all logged functions")
set(ssLOG_SHOW_FUNC_NAME 1 CACHE BOOL "Show function name for all logged functions")
set(ssLOG_SHOW_DATE 1 CACHE BOOL "Show log date for all logged functions")
set(ssLOG_SHOW_TIME 1 CACHE BOOL "Show log time for all logged functions")
set(ssLOG_THREAD_SAFE_OUTPUT 1 CACHE BOOL "Use locks to ensure outputs are atomic. Can be turned off if only running in single thread for performance.")
set(ssLOG_SHOW_THREADS 1 CACHE BOOL "Shows the thread ID for the output")
set(ssLOG_LOG_TO_FILE 0 CACHE BOOL "Log to file instead for all logged functions")
set(ssLOG_LEVEL "WARNING" CACHE STRING 
"Compile time log level (NONE, FATAL, ERROR, WARNING, INFO, DEBUG)
NONE     (None of the levels will be printed, but will still print normal ssLOG_LINE or ssLOG_FUNC)
FATAL    (Program will crash)
ERROR    (Program might crash and **likely** to not function correctly)
WARNING  (Program won't crash but **might** not function correctly)
INFO     (Program state which **doesn't** spam the log)
DEBUG    (Program state which **does** spam the log)")

set(ssLOG_USE_ESCAPE_SEQUENCES 0 CACHE BOOL "Use escape sequences regardless of it being supported or not")

set_property(CACHE ssLOG_LEVEL PROPERTY STRINGS "NONE" 
                                                "FATAL" 
                                                "ERROR" 
                                                "WARNING" 
                                                "INFO" 
                                                "DEBUG")

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(STANDARD_COMPILE_FLAGS "/utf-8;/W4;")
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
    if("${ssLOG_BUILD_TYPE}" STREQUAL "STATIC")
        add_library(ssLogger STATIC 
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/ssLogInit.cpp"
        )
    elseif("${ssLOG_BUILD_TYPE}" STREQUAL "SHARED")
        add_library(ssLogger SHARED
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/ssLogInit.cpp"
        )
        
        target_compile_definitions(ssLogger PUBLIC ssLOG_DLL=1
                                            PRIVATE ssLOG_SHARED_EXPORT=1)
    elseif("${ssLOG_BUILD_TYPE}" STREQUAL "OBJECT")
        add_library(ssLogger OBJECT
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/ssLogInit.cpp"
        )
    endif()

    target_include_directories(ssLogger PUBLIC  "${CMAKE_CURRENT_LIST_DIR}/Include")

    target_compile_definitions(ssLogger PUBLIC ssLOG_CALL_STACK=$<BOOL:${ssLOG_CALL_STACK}>)
    target_compile_definitions(ssLogger PUBLIC ssLOG_ASCII=$<BOOL:${ssLOG_LOG_WITH_ASCII}>)
    target_compile_definitions(ssLogger PUBLIC ssLOG_SHOW_FILE_NAME=$<BOOL:${ssLOG_SHOW_FILE_NAME}>)
    target_compile_definitions(ssLogger PUBLIC ssLOG_SHOW_LINE_NUM=$<BOOL:${ssLOG_SHOW_LINE_NUM}>)
    target_compile_definitions(ssLogger PUBLIC ssLOG_SHOW_FUNC_NAME=$<BOOL:${ssLOG_SHOW_FUNC_NAME}>)
    target_compile_definitions(ssLogger PUBLIC ssLOG_SHOW_DATE=$<BOOL:${ssLOG_SHOW_DATE}>)
    target_compile_definitions(ssLogger PUBLIC ssLOG_SHOW_TIME=$<BOOL:${ssLOG_SHOW_TIME}>)
    target_compile_definitions(ssLogger PUBLIC ssLOG_THREAD_SAFE_OUTPUT=$<BOOL:${ssLOG_THREAD_SAFE_OUTPUT}>)
    target_compile_definitions(ssLogger PUBLIC ssLOG_LOG_TO_FILE=$<BOOL:${ssLOG_LOG_TO_FILE}>)
    target_compile_definitions(ssLogger PUBLIC ssLOG_USE_ESCAPE_SEQUENCES=$<BOOL:${ssLOG_USE_ESCAPE_SEQUENCES}>)
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
        target_compile_definitions(ssLogger PUBLIC _CRT_SECURE_NO_WARNINGS)
    endif()
    
    target_compile_options(ssLogger PRIVATE ${STANDARD_COMPILE_FLAGS})

else()
    add_library(ssLogger INTERFACE)
    target_include_directories(ssLogger INTERFACE "${CMAKE_CURRENT_LIST_DIR}/Include")
    
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_definitions(ssLogger INTERFACE _CRT_SECURE_NO_WARNINGS)
    endif()
    
    target_compile_options(ssLogger INTERFACE ${STANDARD_COMPILE_FLAGS})
endif()

if(${ssLOG_BUILD_TESTS})

    # ==========================================================
    # ssLogger for tests
    # ==========================================================

    # Build the library for testing
    if("${ssLOG_BUILD_TYPE}" STREQUAL "STATIC")
        add_library(ssLogger_SRC STATIC 
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/ssLogInit.cpp"
        )
    elseif("${ssLOG_BUILD_TYPE}" STREQUAL "SHARED")
        add_library(ssLogger_SRC SHARED
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/ssLogInit.cpp"
        )
        
        target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_DLL=1
                                                PRIVATE ssLOG_SHARED_EXPORT=1)
        
        if(WIN32)
            set_target_properties(ssLogger_SRC PROPERTIES
                    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/SourceTests/$<CONFIG>"
                    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/SourceTests/$<CONFIG>"
                    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/SourceTests/$<CONFIG>"
            )
        else()
            set_target_properties(ssLogger_SRC PROPERTIES
                    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/SourceTests"
                    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/SourceTests"
                    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/SourceTests"
            )
        endif()
    elseif("${ssLOG_BUILD_TYPE}" STREQUAL "OBJECT")
        add_library(ssLogger_SRC OBJECT
        "${CMAKE_CURRENT_SOURCE_DIR}/Src/ssLogInit.cpp"
        )
    endif()
    
    # target_sources(ssLogger_SRC PUBLIC "${PROJECT_SOURCE_DIR}/ssLogInit.cpp")
    target_include_directories(ssLogger_SRC PUBLIC  "${CMAKE_CURRENT_LIST_DIR}/Include")

    target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_CALL_STACK=$<BOOL:${ssLOG_CALL_STACK}>)
    target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_ASCII=$<BOOL:${ssLOG_LOG_WITH_ASCII}>)
    target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_SHOW_FILE_NAME=$<BOOL:${ssLOG_SHOW_FILE_NAME}>)
    target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_SHOW_LINE_NUM=$<BOOL:${ssLOG_SHOW_LINE_NUM}>)
    target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_SHOW_FUNC_NAME=$<BOOL:${ssLOG_SHOW_FUNC_NAME}>)
    target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_SHOW_DATE=$<BOOL:${ssLOG_SHOW_DATE}>)
    target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_SHOW_TIME=$<BOOL:${ssLOG_SHOW_TIME}>)
    target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_THREAD_SAFE_OUTPUT=$<BOOL:${ssLOG_THREAD_SAFE_OUTPUT}>)
    target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_LOG_TO_FILE=$<BOOL:${ssLOG_LOG_TO_FILE}>)
    target_compile_definitions(ssLogger_SRC PUBLIC ssLOG_USE_ESCAPE_SEQUENCES=$<BOOL:${ssLOG_USE_ESCAPE_SEQUENCES}>)
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
    target_include_directories(ssLogger_HEAD INTERFACE  "${CMAKE_CURRENT_LIST_DIR}/Include")
    #target_sources(ssLogger_HEAD INTERFACE
    #                        "${CMAKE_CURRENT_LIST_DIR}/Include/ssLogger/ssLog.hpp"
    #                        "${CMAKE_CURRENT_LIST_DIR}/Include/ssLogger/ssLogInit.hpp"
    #                        "${CMAKE_CURRENT_LIST_DIR}/Include/ssLogger/ssLogSwitches.hpp"
    #                        )

    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_definitions(ssLogger_HEAD INTERFACE _CRT_SECURE_NO_WARNINGS)
        target_compile_definitions(ssLogger_SRC PUBLIC _CRT_SECURE_NO_WARNINGS)
    endif()
    
    target_compile_options(ssLogger_HEAD INTERFACE ${STANDARD_COMPILE_FLAGS})
    target_compile_options(ssLogger_SRC PUBLIC ${STANDARD_COMPILE_FLAGS})

    # ==========================================================
    # Tests
    # ==========================================================

    function(AddFilesToExecutables folderToExe binSubFolder targetsToLink exeSuffix)
    
        file(GLOB TEST_CPP_FILES "${folderToExe}/*.cpp")
        
        foreach(CUR_SOURCE_PATH ${TEST_CPP_FILES})
            # Get the file name without path and extension
            get_filename_component(EXECUTABLE_NAME "${CUR_SOURCE_PATH}" NAME_WE)

            # if(NOT "${EXECUTABLE_NAME}" STREQUAL "LogMultiThreadTest")
            #     continue()
            # endif()

            #message("EXECUTABLE_NAME: ${EXECUTABLE_NAME}")
            #message("exeSuffix: ${exeSuffix}")
            #message("CUR_SOURCE_PATH: ${CUR_SOURCE_PATH}")
            #message("targetsToLink: ${targetsToLink}\n")

            # Add executable
            add_executable("${EXECUTABLE_NAME}${exeSuffix}" "${CUR_SOURCE_PATH}")

            target_link_libraries("${EXECUTABLE_NAME}${exeSuffix}" PUBLIC ${targetsToLink})

            # Sub-directory for build
            set_target_properties("${EXECUTABLE_NAME}${exeSuffix}" PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${binSubFolder})
            
            # target_compile_options("${EXECUTABLE_NAME}${exeSuffix}" PRIVATE "-E")

            # if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            #     target_compile_definitions("${EXECUTABLE_NAME}${exeSuffix}" PUBLIC _CRT_SECURE_NO_WARNINGS)
            #     target_compile_options("${EXECUTABLE_NAME}${exeSuffix}" PRIVATE "/utf-8;/W4;")
            # else()
            #     target_compile_options("${EXECUTABLE_NAME}${exeSuffix}" PRIVATE -Wall -Wpedantic -Werror)
            # endif()
            
            if(NOT WIN32 AND "${ssLOG_BUILD_TYPE}" STREQUAL "SHARED")
                set_target_properties("${EXECUTABLE_NAME}${exeSuffix}" PROPERTIES
                        BUILD_WITH_INSTALL_RPATH FALSE
                        LINK_FLAGS "-Wl,-rpath,$ORIGIN/")
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
