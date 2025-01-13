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

    //5 is DEBUG    (Program state which **does** spam the log)
    //4 is INFO     (Program state which **doesn't** spam the log)
    //3 is WARNING  (Program won't crash but **might** not function correctly)
    //2 is ERROR    (Program might crash and **likely** to not function correctly)
    //1 is FATAL    (Program will crash)
    //0 is NONE     (None of the levels will be printed, but will still print normal ssLOG_LINE or ssLOG_FUNC)
    #ifndef ssLOG_LEVEL
        #define ssLOG_LEVEL 3
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
#endif
