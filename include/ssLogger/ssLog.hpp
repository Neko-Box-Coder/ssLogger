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
#define CAT( A, B ) A ## B
#define SELECT( NAME, NUM ) CAT( NAME ## _, NUM )
#define COMPOSE( NAME, ARGS ) NAME ARGS

#define GET_COUNT( _0, _1, _2, _3, _4, _5, _6 /* ad nauseam */, COUNT, ... ) COUNT
#define EXPAND() ,,,,,, // 6 commas (or 7 empty tokens)
#define VA_SIZE( ... ) COMPOSE( GET_COUNT, (EXPAND __VA_ARGS__ (), 0, 6, 5, 4, 3, 2, 1) )

#define VA_SELECT( NAME, ... ) SELECT( NAME, VA_SIZE(__VA_ARGS__) )(__VA_ARGS__)


// =======================================================================
// Helper macro functions
// =======================================================================

#if ssLOG_LOG_TO_FILE
    #include <fstream>
    #include <ctime>
    extern std::ofstream LogFileStream;

    #define ssLOG_SIMPLE(x)\
    {\
        if(!LogFileStream.is_open())\
        {\
            time_t now = time(0);\
            LogFileStream.open(std::string(std::string(ctime(&now))+"_log.txt"), std::ofstream::out);\
        }\
        LogFileStream << x << "\n";\
    }
#else
    #include <iostream>
    #define ssLOG_SIMPLE(x) std::cout<<x<<"\n";
#endif


#if ssLOG_SHOW_FILE_NAME
    #define GET_FILE_NAME()\
    []()\
    {\
        std::string fileName = __FILE__;\
        std::size_t found = fileName.find_last_of("/\\");\
        return " in "+fileName.substr(found+1);\
    }()
#else
    #define GET_FILE_NAME() ""
#endif

#if ssLOG_SHOW_LINE_NUM
    #define GET_LINE_NUM() " on line "<<__LINE__
#else
    #define GET_LINE_NUM() ""
#endif

// Nesting VA_SELECT doesn't work :/
// #define GET_FUNCTION_NAME( ... ) VA_SELECT( GET_FUNCTION_NAME, __VA_ARGS__ )
#if ssLOG_SHOW_FUNC_NAME
    #define GET_FUNCTION_NAME_0() "["<<__func__<<"]"
    #define GET_FUNCTION_NAME_1(x) "["<<x<<"]"
#else
    #define GET_FUNCTION_NAME_0() ""
    #define GET_FUNCTION_NAME_1(x) ""
#endif

#if ssLOG_SHOW_TIME
    #include <chrono>
    #include <sstream>
    #include <iomanip>
    #include <ctime>
    #define GET_TIME()\
    []()\
    {\
        std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();\
        std::stringstream ss;\
        auto t = std::chrono::system_clock::to_time_t(tp);\
        auto tp2 = std::chrono::system_clock::from_time_t(t);\
        if (tp2 > tp)\
            t = std::chrono::system_clock::to_time_t(tp - std::chrono::seconds(1));\
            \
        ss  << std::put_time(std::localtime(&t), "%Y-%m-%d %T")\
            << "." << std::setfill('0') << std::setw(3)\
            << (std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count() % 1000);\
        ss  << " ";\
        return ss.str();\
    }()
#else
    #define GET_TIME() ""
#endif

#if ssLOG_THREAD_SAFE
    #include <unordered_map>
    #include <thread>
    #include <mutex>

    #ifndef ssTHREAD_LOG_INFO_DECL
    #define ssTHREAD_LOG_INFO_DECL
    struct ThreadLogInfo
    {
        int TabSpace = 0;
        std::stack<std::string> FuncNameStack = std::stack<std::string>();
    };
    #endif

    extern std::unordered_map<std::thread::id, ThreadLogInfo> ssLogInfoMap;
    extern std::thread::id ssLastThreadID;
    extern std::mutex ssLogMutex;

    #define CHECK_THREAD_DIFF()\
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

    #define THREAD_SAFE_OP(x)\
    {\
        ssLogMutex.lock();\
        CHECK_THREAD_DIFF();\
        x\
        ssLogMutex.unlock();\
    }\

    #define GET_TAB_SPACE() ssLogInfoMap[std::this_thread::get_id()].TabSpace

    #define GET_FUNC_STACK() ssLogInfoMap[std::this_thread::get_id()].FuncNameStack    

#else
    extern int ssTabSpace;
    extern std::stack<std::string> ssFuncNameStack;

    #define THREAD_SAFE_OP(x) x

    #define GET_TAB_SPACE() ssTabSpace

    #define GET_FUNC_STACK() ssFuncNameStack

    #define CHECK_THREAD_DIFF(x) x
#endif

// =======================================================================
// Macros for ssLOG_LINE and ssLOG_FUNC 
// =======================================================================

#define ssLOG_LINE( ... ) VA_SELECT( ssLOG_LINE, __VA_ARGS__ )

#define ssLOG_LINE_B( ... ) VA_SELECT( ssLOG_LINE_B, __VA_ARGS__ )


#if !ssLOG_CALL_STACK
    #define ssLOG_FUNC_ENTRY( ... ) 
    #define ssLOG_FUNC_EXIT( ... )

    #define ssLOG_FUNC(expr) expr

    #define ssLOG_LINE_0()\
    {\
        THREAD_SAFE_OP(ssLOG_SIMPLE(GET_TIME()<<GET_FUNCTION_NAME_0()<<GET_FILE_NAME()<<GET_LINE_NUM()));\
    }

    #define ssLOG_LINE_1(debugText)\
    {\
        THREAD_SAFE_OP(ssLOG_SIMPLE(GET_TIME()<<GET_FUNCTION_NAME_0()<<GET_FILE_NAME()<<GET_LINE_NUM()<<": ["<<debugText<<"]"));\
    }
#else    
    inline std::string TabAdder(int tabAmount, bool tree = false)
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
        THREAD_SAFE_OP(ssLOG_SIMPLE(GET_TIME()<<TabAdder(GET_TAB_SPACE(), true)<<GET_FUNCTION_NAME_0()<<GET_FILE_NAME()<<GET_LINE_NUM()));\
    }

    #define ssLOG_LINE_1(debugText)\
    {\
        THREAD_SAFE_OP(ssLOG_SIMPLE(GET_TIME()<<TabAdder(GET_TAB_SPACE(), true)<<GET_FUNCTION_NAME_0()<<GET_FILE_NAME()<<GET_LINE_NUM()<<": ["<<debugText<<"]"));\
    }
    
    #define Q(x) #x

    #define ssLOG_FUNC(expr)\
    THREAD_SAFE_OP({\
        ssLOG_SIMPLE(GET_TIME()<<TabAdder(GET_TAB_SPACE()));\
        ssLOG_SIMPLE(GET_TIME()<<TabAdder(GET_TAB_SPACE(), true)<<GET_FUNCTION_NAME_1(Q(expr))<<GET_FILE_NAME()<<GET_LINE_NUM()<<": [Entry]");\
        GET_FUNC_STACK().push(Q(expr));\
        GET_TAB_SPACE()++;\
    });\
    expr;\
    THREAD_SAFE_OP({\
        if(GET_FUNC_STACK().empty() || GET_FUNC_STACK().top() != Q(expr))\
        {\
            ssLOG_SIMPLE("ssLOG_FUNC_EXIT is missing somewhere. "<<GET_FUNC_STACK().top()<<" is expected but"<<Q(expr)<<" is found instead.");\
            std::exit(EXIT_FAILURE);\
        }\
        GET_FUNC_STACK().pop();\
        GET_TAB_SPACE()--;\
        ssLOG_SIMPLE(GET_TIME()<<TabAdder(GET_TAB_SPACE())<<GET_FUNCTION_NAME_1(Q(expr))<<GET_FILE_NAME()<<GET_LINE_NUM()<<": [Exit]");\
        ssLOG_SIMPLE(GET_TIME()<<TabAdder(GET_TAB_SPACE()));\
    });\


    #define ssLOG_FUNC_ENTRY( ... ) VA_SELECT( ssLOG_FUNC_ENTRY, __VA_ARGS__ )
    #define ssLOG_FUNC_EXIT( ... ) VA_SELECT( ssLOG_FUNC_EXIT, __VA_ARGS__ )

    #define ssLOG_FUNC_ENTRY_0()\
    THREAD_SAFE_OP({\
        ssLOG_SIMPLE(GET_TIME()<<TabAdder(GET_TAB_SPACE()));\
        ssLOG_SIMPLE(GET_TIME()<<TabAdder(GET_TAB_SPACE(), true)<<GET_FUNCTION_NAME_0()<<GET_FILE_NAME()<<GET_LINE_NUM()<<": [Entry]");\
        GET_FUNC_STACK().push(__func__);\
        GET_TAB_SPACE()++;\
    });

    #define ssLOG_FUNC_EXIT_0()\
    THREAD_SAFE_OP({\
        if(GET_FUNC_STACK().empty() || GET_FUNC_STACK().top() != __func__)\
        {\
            ssLOG_SIMPLE("ssLOG_FUNC_EXIT is expecting "<<GET_FUNC_STACK().top()<<". "<<__func__<<" is found instead.");\
            std::exit(EXIT_FAILURE);\
        }\
        GET_FUNC_STACK().pop();\
        GET_TAB_SPACE()--;\
        ssLOG_SIMPLE(GET_TIME()<<TabAdder(GET_TAB_SPACE())<<GET_FUNCTION_NAME_0()<<GET_FILE_NAME()<<GET_LINE_NUM()<<": [Exit]");\
        ssLOG_SIMPLE(GET_TIME()<<TabAdder(GET_TAB_SPACE()));\
    });

    #define ssLOG_FUNC_ENTRY_1(customFunc)\
    THREAD_SAFE_OP({\
        ssLOG_SIMPLE(GET_TIME()<<TabAdder(GET_TAB_SPACE()));\
        ssLOG_SIMPLE(GET_TIME()<<TabAdder(GET_TAB_SPACE(), true)<<GET_FUNCTION_NAME_1(Q(customFunc))<<GET_FILE_NAME()<<GET_LINE_NUM()<<": [Entry]");\
        GET_FUNC_STACK().push(customFunc);\
        GET_TAB_SPACE()++;\
    });

    #define ssLOG_FUNC_EXIT_1(customFunc)\
    THREAD_SAFE_OP({\
        if(GET_FUNC_STACK().empty() || GET_FUNC_STACK().top() != customFunc)\
        {\
            ssLOG_SIMPLE("ssLOG_FUNC_EXIT is expecting "<<GET_FUNC_STACK().top()<<". "<<customFunc<<" is found instead.");\
            std::exit(EXIT_FAILURE);\
        }\
        GET_FUNC_STACK().pop();\
        GET_TAB_SPACE()--;\
        ssLOG_SIMPLE(GET_TIME()<<TabAdder(GET_TAB_SPACE())<<GET_FUNCTION_NAME_1(Q(customFunc))<<GET_FILE_NAME()<<GET_LINE_NUM()<<": [Exit]");\
        ssLOG_SIMPLE(GET_TIME()<<TabAdder(GET_TAB_SPACE()));\
    });

    #define ssLOG_EXIT_PROGRAM() std::exit(EXIT_FAILURE);

#endif


#endif