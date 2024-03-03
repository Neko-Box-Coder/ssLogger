// =======================================================================
// Macros switches (if you don't wnat to use Cmake)
// =======================================================================

#ifndef ssLOG_SWITCHES_HPP
#define ssLOG_SWITCHES_HPP

#if !ssLOG_USE_SOURCE 

    #define ssLOG_CALL_STACK 1

    #define ssLOG_ASCII 0

    #define ssLOG_SHOW_FILE_NAME 1

    #define ssLOG_SHOW_LINE_NUM 1

    #define ssLOG_SHOW_FUNC_NAME 1

    #define ssLOG_SHOW_TIME 1

    #define ssLOG_THREAD_SAFE 1

    #define ssLOG_LOG_TO_FILE 0

    //5 is DEBUG    (Program state which **does** spam the log)
    //4 is INFO     (Program state which **doesn't** spam the log)
    //3 is WARNING  (Program won't crash but **might** not function correctly)
    //2 is ERROR    (Program might crash and **likely** to not function correctly)
    //1 is FATAL    (Program will crash)
    //0 is NONE     (None of the levels will be printed, but will still print normal ssLOG_LINE or ssLOG_FUNC)
    #define ssLOG_LEVEL 3

#endif

#endif