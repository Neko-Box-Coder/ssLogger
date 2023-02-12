#ifndef ssLOG
#define ssLOG

#ifndef ssLOG_USE_SOURCE
    #include "ssLogger/ssLogSwitches.hpp"
#endif

#include <string>
#include <stack>

// =======================================================================
// Macros for allowing overloadable Macro functions
// =======================================================================

// https://stackoverflow.com/questions/16683146/can-macros-be-overloaded-by-number-of-arguments
#define INTERNAL_ssLOG_CAT( A, B ) A ## B
#define INTERNAL_ssLOG_SELECT( NAME, NUM ) INTERNAL_ssLOG_CAT( NAME ## _, NUM )
#define INTERNAL_ssLOG_COMPOSE( NAME, ARGS ) NAME ARGS

#define INTERNAL_ssLOG_GET_COUNT( _0, _1, _2, _3, _4, _5, _6 /* ad nauseam */, COUNT, ... ) COUNT
#define INTERNAL_ssLOG_EXPAND() ,,,,,, // 6 commas (or 7 empty tokens)
#define INTERNAL_ssLOG_VA_SIZE( ... ) INTERNAL_ssLOG_COMPOSE( INTERNAL_ssLOG_GET_COUNT, (INTERNAL_ssLOG_EXPAND __VA_ARGS__ (), 0, 6, 5, 4, 3, 2, 1) )

#ifndef _MSC_VER
#define INTERNAL_ssLOG_VA_SELECT( NAME, ... ) INTERNAL_ssLOG_SELECT( NAME, INTERNAL_ssLOG_VA_SIZE(__VA_ARGS__) )(__VA_ARGS__)
#else

//MSVC workaround: https://stackoverflow.com/questions/48710758/how-to-fix-variadic-macro-related-issues-with-macro-overloading-in-msvc-mic
//This is honestly retarded.
#define INTERNAL_ssLOG_VA_ARGS_FIX( macro, args ) macro args
#define INTERNAL_ssLOG_VA_SELECT( NAME, ... ) INTERNAL_ssLOG_VA_ARGS_FIX(INTERNAL_ssLOG_SELECT, ( NAME, INTERNAL_ssLOG_VA_SIZE( __VA_ARGS__ ) )) (__VA_ARGS__)
#endif

// =======================================================================
// Helper macro functions
// =======================================================================

#if ssLOG_LOG_TO_FILE
    #include <fstream>
    #include <ctime>
    extern std::ofstream ssLogFileStream;

    #define ssLOG_SIMPLE(x)\
    {\
        if(!ssLogFileStream.is_open())\
        {\
            time_t ssRawtime;\
            struct tm * ssTimeinfo;\
            char ssBuffer [80];\
            time(&ssRawtime);\
            ssTimeinfo = localtime(&ssRawtime);\
            strftime(ssBuffer, 80, "%a %b %d %H_%M_%S %Y", ssTimeinfo);\
            std::string nowString = std::string(ssBuffer)+"_log.txt";\
            ssLogFileStream.open( nowString, std::ofstream::out);\
            if(!ssLogFileStream.good())\
            {\
                throw string("Failed to create log file!!");\
            }\
        }\
        ssLogFileStream << x << "\n";\
    }
#else
    #include <iostream>
    #define ssLOG_SIMPLE(x)\
    {\
        std::cout<<x<<"\n";\
    }
#endif


#if ssLOG_SHOW_FILE_NAME
    #define ssLOG_GET_FILE_NAME()\
    []()\
    {\
        std::string ssLogfileName = __FILE__;\
        std::size_t ssLogfound = ssLogfileName.find_last_of("/\\");\
        return " in "+ssLogfileName.substr(ssLogfound+1);\
    }()
#else
    #define ssLOG_GET_FILE_NAME() ""
#endif

#if ssLOG_SHOW_LINE_NUM
    #define ssLOG_GET_LINE_NUM() " on line "<<__LINE__
#else
    #define ssLOG_GET_LINE_NUM() ""
#endif

// Nesting INTERNAL_ssLOG_VA_SELECT doesn't work :/
// #define GET_FUNCTION_NAME( ... ) INTERNAL_ssLOG_VA_SELECT( GET_FUNCTION_NAME, __VA_ARGS__ )
#if ssLOG_SHOW_FUNC_NAME
    #define INTERNAL_ssLOG_GET_FUNCTION_NAME_0() "["<<__func__<<"]"
    #define INTERNAL_ssLOG_GET_FUNCTION_NAME_1(x) "["<<x<<"]"
#else
    #define INTERNAL_ssLOG_GET_FUNCTION_NAME_0() ""
    #define INTERNAL_ssLOG_GET_FUNCTION_NAME_1(x) ""
#endif

#if ssLOG_SHOW_TIME
    #include <chrono>
    #include <sstream>
    #include <iomanip>
    #include <ctime>
    #define INTERNAL_ssLOG_GET_TIME()\
    []()\
    {\
        std::chrono::system_clock::time_point ssLog_tp = std::chrono::system_clock::now();\
        std::stringstream ssLog_ss;\
        auto ssLog_time_t = std::chrono::system_clock::to_time_t(ssLog_tp);\
        /*No idea what this is used for*/\
        /*auto ssLog_time_t2 = std::chrono::system_clock::from_time_t(ssLog_time_t);*/\
        /*if (ssLog_time_t2 > ssLog_tp)*/\
            /*ssLog_time_t = std::chrono::system_clock::to_time_t(ssLog_tp - std::chrono::seconds(1));*/\
            \
        ssLog_ss    << std::put_time(std::localtime(&ssLog_time_t), "%Y-%m-%d %T")\
                    << "." << std::setfill('0') << std::setw(3)\
                    << (std::chrono::duration_cast<std::chrono::milliseconds>(ssLog_tp.time_since_epoch()).count() % 1000);\
        ssLog_ss    << " ";\
        return ssLog_ss.str();\
    }()
#else
    #define INTERNAL_ssLOG_GET_TIME() ""
#endif

#if ssLOG_THREAD_SAFE
    #include <unordered_map>
    #include <thread>
    #include <mutex>

    #ifndef INTERNAL_ssTHREAD_LOG_INFO_DECL
    #define INTERNAL_ssTHREAD_LOG_INFO_DECL
    struct ssLogThreadLogInfo
    {
        int TabSpace = 0;
        std::stack<std::string> FuncNameStack = std::stack<std::string>();
    };
    #endif

    extern std::unordered_map<std::thread::id, ssLogThreadLogInfo> ssLogInfoMap;
    extern std::thread::id ssLastThreadID;
    extern std::mutex ssLogMutex;

    #define INTERNAL_ssLOG_CHECK_THREAD_DIFF()\
    {\
        if(ssLastThreadID != std::this_thread::get_id())\
        {\
            ssLastThreadID = std::this_thread::get_id();\
            ssLOG_SIMPLE("");\
            ssLOG_SIMPLE("");\
            ssLOG_SIMPLE("===================================================================");\
            ssLOG_SIMPLE("Thread: "<<std::this_thread::get_id());\
            ssLOG_SIMPLE("===================================================================");\
        }\
    }

    #define INTERNAL_ssLOG_THREAD_SAFE_OP(x)\
    {\
        ssLogMutex.lock();\
        INTERNAL_ssLOG_CHECK_THREAD_DIFF();\
        x\
        ssLogMutex.unlock();\
    }\

    #define INTERNAL_ssLOG_GET_TAB_SPACE() ssLogInfoMap[std::this_thread::get_id()].TabSpace

    #define INTERNAL_ssLOG_GET_FUNC_STACK() ssLogInfoMap[std::this_thread::get_id()].FuncNameStack    

#else
    extern int ssTabSpace;
    extern std::stack<std::string> ssFuncNameStack;

    #define INTERNAL_ssLOG_THREAD_SAFE_OP(x) x

    #define INTERNAL_ssLOG_GET_TAB_SPACE() ssTabSpace

    #define INTERNAL_ssLOG_GET_FUNC_STACK() ssFuncNameStack

    #define INTERNAL_ssLOG_CHECK_THREAD_DIFF(x) x
#endif

// =======================================================================
// Macros for ssLOG_LINE and ssLOG_FUNC 
// =======================================================================

#define ssLOG_LINE( ... ) INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_LINE, __VA_ARGS__ )

#if !ssLOG_CALL_STACK
    #define ssLOG_FUNC_ENTRY( ... ) 
    #define ssLOG_FUNC_EXIT( ... )

    #define ssLOG_FUNC(expr) expr

    #define INTERNAL_ssLOG_LINE_0()\
    {\
        INTERNAL_ssLOG_THREAD_SAFE_OP(ssLOG_SIMPLE(INTERNAL_ssLOG_GET_TIME()<<INTERNAL_ssLOG_GET_FUNCTION_NAME_0()<<ssLOG_GET_FILE_NAME()<<ssLOG_GET_LINE_NUM()));\
    }

    #define INTERNAL_ssLOG_LINE_1(debugText)\
    {\
        INTERNAL_ssLOG_THREAD_SAFE_OP(ssLOG_SIMPLE(INTERNAL_ssLOG_GET_TIME()<<INTERNAL_ssLOG_GET_FUNCTION_NAME_0()<<ssLOG_GET_FILE_NAME()<<ssLOG_GET_LINE_NUM()<<": ["<<debugText<<"]"));\
    }
#else    
    #ifdef _WIN32
    #include <windows.h>
    #undef max
    #undef DELETE
    #endif
    inline std::string Internal_ssLog_TabAdder(int tabAmount, bool tree = false)
    {
        //Yeah.... this is ugly. Can't do much because we don't really have initialization function
        #ifdef _WIN32
        static bool ssCalled = false;
        if(!ssCalled)
        {
            ssCalled = true;
            SetConsoleOutputCP(CP_UTF8);
        }
        #endif

        std::string returnString = "";
        for(int tab = 0; tab < tabAmount; tab++)
        {
            if(tab == tabAmount - 1 && tree)
                returnString += "├─►";
            else
                returnString += "│  ";
        }

        return returnString;
    }

    #define INTERNAL_ssLOG_LINE_0()\
    {\
        INTERNAL_ssLOG_THREAD_SAFE_OP(ssLOG_SIMPLE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE(), true)<<INTERNAL_ssLOG_GET_FUNCTION_NAME_0()<<ssLOG_GET_FILE_NAME()<<ssLOG_GET_LINE_NUM()));\
    }

    #define INTERNAL_ssLOG_LINE_1(debugText)\
    {\
        INTERNAL_ssLOG_THREAD_SAFE_OP(ssLOG_SIMPLE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE(), true)<<INTERNAL_ssLOG_GET_FUNCTION_NAME_0()<<ssLOG_GET_FILE_NAME()<<ssLOG_GET_LINE_NUM()<<": ["<<debugText<<"]"));\
    }
    
    #define INTERNAL_ssLOG_Q(x) #x

    #define ssLOG_FUNC(expr)\
    INTERNAL_ssLOG_THREAD_SAFE_OP({\
        ssLOG_SIMPLE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE()));\
        ssLOG_SIMPLE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE(), true)<<INTERNAL_ssLOG_GET_FUNCTION_NAME_1(INTERNAL_ssLOG_Q(expr))<<ssLOG_GET_FILE_NAME()<<ssLOG_GET_LINE_NUM()<<": [Entry]");\
        INTERNAL_ssLOG_GET_FUNC_STACK().push(INTERNAL_ssLOG_Q(expr));\
        INTERNAL_ssLOG_GET_TAB_SPACE()++;\
    });\
    expr;\
    INTERNAL_ssLOG_THREAD_SAFE_OP({\
        if(INTERNAL_ssLOG_GET_FUNC_STACK().empty() || INTERNAL_ssLOG_GET_FUNC_STACK().top() != INTERNAL_ssLOG_Q(expr))\
        {\
            ssLOG_SIMPLE("ssLOG_FUNC_EXIT is missing somewhere. "<<INTERNAL_ssLOG_GET_FUNC_STACK().top()<<" is expected but"<<INTERNAL_ssLOG_Q(expr)<<" is found instead.");\
            std::exit(EXIT_FAILURE);\
        }\
        INTERNAL_ssLOG_GET_FUNC_STACK().pop();\
        INTERNAL_ssLOG_GET_TAB_SPACE()--;\
        ssLOG_SIMPLE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE())<<INTERNAL_ssLOG_GET_FUNCTION_NAME_1(INTERNAL_ssLOG_Q(expr))<<ssLOG_GET_FILE_NAME()<<ssLOG_GET_LINE_NUM()<<": [Exit]");\
        ssLOG_SIMPLE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE()));\
    });\


    #define ssLOG_FUNC_ENTRY( ... ) INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_ENTRY, __VA_ARGS__ )
    #define ssLOG_FUNC_EXIT( ... ) INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_EXIT, __VA_ARGS__ )

    #define INTERNAL_ssLOG_FUNC_ENTRY_0()\
    INTERNAL_ssLOG_THREAD_SAFE_OP({\
        ssLOG_SIMPLE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE()));\
        ssLOG_SIMPLE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE(), true)<<INTERNAL_ssLOG_GET_FUNCTION_NAME_0()<<ssLOG_GET_FILE_NAME()<<ssLOG_GET_LINE_NUM()<<": [Entry]");\
        INTERNAL_ssLOG_GET_FUNC_STACK().push(__func__);\
        INTERNAL_ssLOG_GET_TAB_SPACE()++;\
    });

    #define INTERNAL_ssLOG_FUNC_EXIT_0()\
    INTERNAL_ssLOG_THREAD_SAFE_OP({\
        if(INTERNAL_ssLOG_GET_FUNC_STACK().empty() || INTERNAL_ssLOG_GET_FUNC_STACK().top() != __func__)\
        {\
            ssLOG_SIMPLE("ssLOG_FUNC_EXIT is expecting "<<INTERNAL_ssLOG_GET_FUNC_STACK().top()<<". "<<__func__<<" is found instead.");\
            std::exit(EXIT_FAILURE);\
        }\
        INTERNAL_ssLOG_GET_FUNC_STACK().pop();\
        INTERNAL_ssLOG_GET_TAB_SPACE()--;\
        ssLOG_SIMPLE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE())<<INTERNAL_ssLOG_GET_FUNCTION_NAME_0()<<ssLOG_GET_FILE_NAME()<<ssLOG_GET_LINE_NUM()<<": [Exit]");\
        ssLOG_SIMPLE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE()));\
    });

    #define INTERNAL_ssLOG_FUNC_ENTRY_1(customFunc)\
    INTERNAL_ssLOG_THREAD_SAFE_OP({\
        ssLOG_SIMPLE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE()));\
        ssLOG_SIMPLE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE(), true)<<INTERNAL_ssLOG_GET_FUNCTION_NAME_1(INTERNAL_ssLOG_Q(customFunc))<<ssLOG_GET_FILE_NAME()<<ssLOG_GET_LINE_NUM()<<": [Entry]");\
        INTERNAL_ssLOG_GET_FUNC_STACK().push(customFunc);\
        INTERNAL_ssLOG_GET_TAB_SPACE()++;\
    });

    #define INTERNAL_ssLOG_FUNC_EXIT_1(customFunc)\
    INTERNAL_ssLOG_THREAD_SAFE_OP({\
        if(INTERNAL_ssLOG_GET_FUNC_STACK().empty() || INTERNAL_ssLOG_GET_FUNC_STACK().top() != customFunc)\
        {\
            ssLOG_SIMPLE("ssLOG_FUNC_EXIT is expecting "<<INTERNAL_ssLOG_GET_FUNC_STACK().top()<<". "<<customFunc<<" is found instead.");\
            std::exit(EXIT_FAILURE);\
        }\
        INTERNAL_ssLOG_GET_FUNC_STACK().pop();\
        INTERNAL_ssLOG_GET_TAB_SPACE()--;\
        ssLOG_SIMPLE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE())<<INTERNAL_ssLOG_GET_FUNCTION_NAME_1(INTERNAL_ssLOG_Q(customFunc))<<ssLOG_GET_FILE_NAME()<<ssLOG_GET_LINE_NUM()<<": [Exit]");\
        ssLOG_SIMPLE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE()));\
    });


#endif

#include <stdexcept>

//#define ssLOG_EXIT_PROGRAM() std::exit(EXIT_FAILURE);
#define ssLOG_EXIT_PROGRAM() throw std::exception();


#endif