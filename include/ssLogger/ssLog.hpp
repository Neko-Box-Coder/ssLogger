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
#define ssLOG_CAT( A, B ) A ## B
#define ssLOG_SELECT( NAME, NUM ) ssLOG_CAT( NAME ## _, NUM )
#define ssLOG_COMPOSE( NAME, ARGS ) NAME ARGS

#define ssLOG_GET_COUNT( _0, _1, _2, _3, _4, _5, _6 /* ad nauseam */, COUNT, ... ) COUNT
#define ssLOG_EXPAND() ,,,,,, // 6 commas (or 7 empty tokens)
#define ssLOG_VA_SIZE( ... ) ssLOG_COMPOSE( ssLOG_GET_COUNT, (ssLOG_EXPAND __VA_ARGS__ (), 0, 6, 5, 4, 3, 2, 1) )

#ifndef _MSC_VER
#define ssLOG_VA_SELECT( NAME, ... ) ssLOG_SELECT( NAME, ssLOG_VA_SIZE(__VA_ARGS__) )(__VA_ARGS__)
#else

//MSVC workaround: https://stackoverflow.com/questions/48710758/how-to-fix-variadic-macro-related-issues-with-macro-overloading-in-msvc-mic
//This is honestly retarded.
#define ssLOG_VA_ARGS_FIX( macro, args ) macro args
#define ssLOG_VA_SELECT( NAME, ... ) ssLOG_VA_ARGS_FIX(ssLOG_SELECT, ( NAME, ssLOG_VA_SIZE( __VA_ARGS__ ) )) (__VA_ARGS__)
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
            time_t now = time(0);\
            auto nowString = std::string(ctime(&now));\
            ssLogFileStream.open( nowString.substr(0, nowString.size() - 1) +"_log.txt", std::ofstream::out);\
        }\
        ssLogFileStream << x << "\n";\
    }
#else
    #include <iostream>
    #define ssLOG_SIMPLE(x) std::cout<<x<<"\n";
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

// Nesting ssLOG_VA_SELECT doesn't work :/
// #define GET_FUNCTION_NAME( ... ) ssLOG_VA_SELECT( GET_FUNCTION_NAME, __VA_ARGS__ )
#if ssLOG_SHOW_FUNC_NAME
    #define ssLOG_GET_FUNCTION_NAME_0() "["<<__func__<<"]"
    #define ssLOG_GET_FUNCTION_NAME_1(x) "["<<x<<"]"
#else
    #define ssLOG_GET_FUNCTION_NAME_0() ""
    #define ssLOG_GET_FUNCTION_NAME_1(x) ""
#endif

#if ssLOG_SHOW_TIME
    #include <chrono>
    #include <sstream>
    #include <iomanip>
    #include <ctime>
    #define ssLOG_GET_TIME()\
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
    #define ssLOG_GET_TIME() ""
#endif

#if ssLOG_THREAD_SAFE
    #include <unordered_map>
    #include <thread>
    #include <mutex>

    #ifndef ssTHREAD_LOG_INFO_DECL
    #define ssTHREAD_LOG_INFO_DECL
    struct ssLogThreadLogInfo
    {
        int TabSpace = 0;
        std::stack<std::string> FuncNameStack = std::stack<std::string>();
    };
    #endif

    extern std::unordered_map<std::thread::id, ssLogThreadLogInfo> ssLogInfoMap;
    extern std::thread::id ssLastThreadID;
    extern std::mutex ssLogMutex;

    #define ssLOG_CHECK_THREAD_DIFF()\
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

    #define ssLOG_THREAD_SAFE_OP(x)\
    {\
        ssLogMutex.lock();\
        ssLOG_CHECK_THREAD_DIFF();\
        x\
        ssLogMutex.unlock();\
    }\

    #define ssLOG_GET_TAB_SPACE() ssLogInfoMap[std::this_thread::get_id()].TabSpace

    #define ssLOG_GET_FUNC_STACK() ssLogInfoMap[std::this_thread::get_id()].FuncNameStack    

#else
    extern int ssTabSpace;
    extern std::stack<std::string> ssFuncNameStack;

    #define ssLOG_THREAD_SAFE_OP(x) x

    #define ssLOG_GET_TAB_SPACE() ssTabSpace

    #define ssLOG_GET_FUNC_STACK() ssFuncNameStack

    #define ssLOG_CHECK_THREAD_DIFF(x) x
#endif

// =======================================================================
// Macros for ssLOG_LINE and ssLOG_FUNC 
// =======================================================================

#define ssLOG_LINE( ... ) ssLOG_VA_SELECT( ssLOG_LINE, __VA_ARGS__ )

#define ssLOG_LINE_B( ... ) ssLOG_VA_SELECT( ssLOG_LINE_B, __VA_ARGS__ )


#if !ssLOG_CALL_STACK
    #define ssLOG_FUNC_ENTRY( ... ) 
    #define ssLOG_FUNC_EXIT( ... )

    #define ssLOG_FUNC(expr) expr

    #define ssLOG_LINE_0()\
    {\
        ssLOG_THREAD_SAFE_OP(ssLOG_SIMPLE(ssLOG_GET_TIME()<<ssLOG_GET_FUNCTION_NAME_0()<<ssLOG_GET_FILE_NAME()<<ssLOG_GET_LINE_NUM()));\
    }

    #define ssLOG_LINE_1(debugText)\
    {\
        ssLOG_THREAD_SAFE_OP(ssLOG_SIMPLE(ssLOG_GET_TIME()<<ssLOG_GET_FUNCTION_NAME_0()<<ssLOG_GET_FILE_NAME()<<ssLOG_GET_LINE_NUM()<<": ["<<debugText<<"]"));\
    }
#else    
    inline std::string ssLog_TabAdder(int tabAmount, bool tree = false)
    {
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

    #define ssLOG_LINE_0()\
    {\
        ssLOG_THREAD_SAFE_OP(ssLOG_SIMPLE(ssLOG_GET_TIME()<<ssLog_TabAdder(ssLOG_GET_TAB_SPACE(), true)<<ssLOG_GET_FUNCTION_NAME_0()<<ssLOG_GET_FILE_NAME()<<ssLOG_GET_LINE_NUM()));\
    }

    #define ssLOG_LINE_1(debugText)\
    {\
        ssLOG_THREAD_SAFE_OP(ssLOG_SIMPLE(ssLOG_GET_TIME()<<ssLog_TabAdder(ssLOG_GET_TAB_SPACE(), true)<<ssLOG_GET_FUNCTION_NAME_0()<<ssLOG_GET_FILE_NAME()<<ssLOG_GET_LINE_NUM()<<": ["<<debugText<<"]"));\
    }
    
    #define ssLOG_Q(x) #x

    #define ssLOG_FUNC(expr)\
    ssLOG_THREAD_SAFE_OP({\
        ssLOG_SIMPLE(ssLOG_GET_TIME()<<ssLog_TabAdder(ssLOG_GET_TAB_SPACE()));\
        ssLOG_SIMPLE(ssLOG_GET_TIME()<<ssLog_TabAdder(ssLOG_GET_TAB_SPACE(), true)<<ssLOG_GET_FUNCTION_NAME_1(ssLOG_Q(expr))<<ssLOG_GET_FILE_NAME()<<ssLOG_GET_LINE_NUM()<<": [Entry]");\
        ssLOG_GET_FUNC_STACK().push(ssLOG_Q(expr));\
        ssLOG_GET_TAB_SPACE()++;\
    });\
    expr;\
    ssLOG_THREAD_SAFE_OP({\
        if(ssLOG_GET_FUNC_STACK().empty() || ssLOG_GET_FUNC_STACK().top() != ssLOG_Q(expr))\
        {\
            ssLOG_SIMPLE("ssLOG_FUNC_EXIT is missing somewhere. "<<ssLOG_GET_FUNC_STACK().top()<<" is expected but"<<ssLOG_Q(expr)<<" is found instead.");\
            std::exit(EXIT_FAILURE);\
        }\
        ssLOG_GET_FUNC_STACK().pop();\
        ssLOG_GET_TAB_SPACE()--;\
        ssLOG_SIMPLE(ssLOG_GET_TIME()<<ssLog_TabAdder(ssLOG_GET_TAB_SPACE())<<ssLOG_GET_FUNCTION_NAME_1(ssLOG_Q(expr))<<ssLOG_GET_FILE_NAME()<<ssLOG_GET_LINE_NUM()<<": [Exit]");\
        ssLOG_SIMPLE(ssLOG_GET_TIME()<<ssLog_TabAdder(ssLOG_GET_TAB_SPACE()));\
    });\


    #define ssLOG_FUNC_ENTRY( ... ) ssLOG_VA_SELECT( ssLOG_FUNC_ENTRY, __VA_ARGS__ )
    #define ssLOG_FUNC_EXIT( ... ) ssLOG_VA_SELECT( ssLOG_FUNC_EXIT, __VA_ARGS__ )

    #define ssLOG_FUNC_ENTRY_0()\
    ssLOG_THREAD_SAFE_OP({\
        ssLOG_SIMPLE(ssLOG_GET_TIME()<<ssLog_TabAdder(ssLOG_GET_TAB_SPACE()));\
        ssLOG_SIMPLE(ssLOG_GET_TIME()<<ssLog_TabAdder(ssLOG_GET_TAB_SPACE(), true)<<ssLOG_GET_FUNCTION_NAME_0()<<ssLOG_GET_FILE_NAME()<<ssLOG_GET_LINE_NUM()<<": [Entry]");\
        ssLOG_GET_FUNC_STACK().push(__func__);\
        ssLOG_GET_TAB_SPACE()++;\
    });

    #define ssLOG_FUNC_EXIT_0()\
    ssLOG_THREAD_SAFE_OP({\
        if(ssLOG_GET_FUNC_STACK().empty() || ssLOG_GET_FUNC_STACK().top() != __func__)\
        {\
            ssLOG_SIMPLE("ssLOG_FUNC_EXIT is expecting "<<ssLOG_GET_FUNC_STACK().top()<<". "<<__func__<<" is found instead.");\
            std::exit(EXIT_FAILURE);\
        }\
        ssLOG_GET_FUNC_STACK().pop();\
        ssLOG_GET_TAB_SPACE()--;\
        ssLOG_SIMPLE(ssLOG_GET_TIME()<<ssLog_TabAdder(ssLOG_GET_TAB_SPACE())<<ssLOG_GET_FUNCTION_NAME_0()<<ssLOG_GET_FILE_NAME()<<ssLOG_GET_LINE_NUM()<<": [Exit]");\
        ssLOG_SIMPLE(ssLOG_GET_TIME()<<ssLog_TabAdder(ssLOG_GET_TAB_SPACE()));\
    });

    #define ssLOG_FUNC_ENTRY_1(customFunc)\
    ssLOG_THREAD_SAFE_OP({\
        ssLOG_SIMPLE(ssLOG_GET_TIME()<<ssLog_TabAdder(ssLOG_GET_TAB_SPACE()));\
        ssLOG_SIMPLE(ssLOG_GET_TIME()<<ssLog_TabAdder(ssLOG_GET_TAB_SPACE(), true)<<ssLOG_GET_FUNCTION_NAME_1(ssLOG_Q(customFunc))<<ssLOG_GET_FILE_NAME()<<ssLOG_GET_LINE_NUM()<<": [Entry]");\
        ssLOG_GET_FUNC_STACK().push(customFunc);\
        ssLOG_GET_TAB_SPACE()++;\
    });

    #define ssLOG_FUNC_EXIT_1(customFunc)\
    ssLOG_THREAD_SAFE_OP({\
        if(ssLOG_GET_FUNC_STACK().empty() || ssLOG_GET_FUNC_STACK().top() != customFunc)\
        {\
            ssLOG_SIMPLE("ssLOG_FUNC_EXIT is expecting "<<ssLOG_GET_FUNC_STACK().top()<<". "<<customFunc<<" is found instead.");\
            std::exit(EXIT_FAILURE);\
        }\
        ssLOG_GET_FUNC_STACK().pop();\
        ssLOG_GET_TAB_SPACE()--;\
        ssLOG_SIMPLE(ssLOG_GET_TIME()<<ssLog_TabAdder(ssLOG_GET_TAB_SPACE())<<ssLOG_GET_FUNCTION_NAME_1(ssLOG_Q(customFunc))<<ssLOG_GET_FILE_NAME()<<ssLOG_GET_LINE_NUM()<<": [Exit]");\
        ssLOG_SIMPLE(ssLOG_GET_TIME()<<ssLog_TabAdder(ssLOG_GET_TAB_SPACE()));\
    });


#endif

#define ssLOG_EXIT_PROGRAM() std::exit(EXIT_FAILURE);


#endif