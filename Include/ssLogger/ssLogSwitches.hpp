// =======================================================================
// Macros switches (if you don't want to use Cmake)
// =======================================================================

#ifndef ssLOG_SWITCHES_HPP
#define ssLOG_SWITCHES_HPP

    #ifndef ssLOG_CALL_STACK
        #define ssLOG_CALL_STACK 1
    #endif

    #ifndef ssLOG_ASCII
        #define ssLOG_ASCII 0
    #endif

    #ifndef ssLOG_SHOW_FILE_NAME
        #define ssLOG_SHOW_FILE_NAME 1
    #endif

    #ifndef ssLOG_SHOW_LINE_NUM
        #define ssLOG_SHOW_LINE_NUM 1
    #endif

    #ifndef ssLOG_SHOW_FUNC_NAME
        #define ssLOG_SHOW_FUNC_NAME 1
    #endif

    #ifndef ssLOG_SHOW_DATE
        #define ssLOG_SHOW_DATE 0
    #endif

    #ifndef ssLOG_SHOW_TIME
        #define ssLOG_SHOW_TIME 1
    #endif

    #ifndef ssLOG_THREAD_SAFE_OUTPUT
        #define ssLOG_THREAD_SAFE_OUTPUT 1
    #endif

    #ifndef ssLOG_LOG_TO_FILE
        #define ssLOG_LOG_TO_FILE 0
    #endif
    
    #define ssLOG_MODE_CONSOLE_AND_FILE 2
    #define ssLOG_MODE_FILE 1
    #define ssLOG_MODE_CONSOLE 0
    
    #ifndef ssLOG_MODE
        #define ssLOG_MODE ssLOG_MODE_CONSOLE
    #endif

    #define ssLOG_LEVEL_DEBUG 5
    #define ssLOG_LEVEL_INFO 4
    #define ssLOG_LEVEL_WARNING 3
    #define ssLOG_LEVEL_ERROR 2
    #define ssLOG_LEVEL_FATAL 1
    #define ssLOG_LEVEL_NONE 0

    #ifndef ssLOG_LEVEL
        #define ssLOG_LEVEL ssLOG_LEVEL_WARNING
    #endif

    #ifndef ssLOG_SHOW_THREADS
        #define ssLOG_SHOW_THREADS 1
    #endif

    #ifndef ssLOG_USE_ESCAPE_SEQUENCES
        #define ssLOG_USE_ESCAPE_SEQUENCES 0
    #endif

    #ifndef ssLOG_USE_WINDOWS_COLOR
        #define ssLOG_USE_WINDOWS_COLOR 0
    #endif
    
    #ifndef ssLOG_THREAD_VSPACE
        #define ssLOG_THREAD_VSPACE 4
    #endif
    
    #ifndef ssLOG_IMMEDIATE_FLUSH
        #define ssLOG_IMMEDIATE_FLUSH 0
    #endif
    
    #ifndef ssLOG_CALL_STACK_ONLY
        #define ssLOG_CALL_STACK_ONLY 0
    #endif
#endif
