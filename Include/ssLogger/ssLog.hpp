#ifndef ssLOG
#define ssLOG

#ifndef ssLOG_USE_SOURCE
    #include "ssLogger/ssLogSwitches.hpp"
#endif

#include <sstream>
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
// Macros for ssLOG_EXIT_PROGRAM
// =======================================================================
#define ssLOG_EXIT_PROGRAM( ... ) do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXIT_PROGRAM, __VA_ARGS__ ) } while(0)

#define INTERNAL_ssLOG_EXIT_PROGRAM_0()\
{\
    std::exit(1);\
}

#define INTERNAL_ssLOG_EXIT_PROGRAM_1(x)\
{\
    std::exit(x);\
}

// =======================================================================
// Error codes
// =======================================================================
#define ssLOG_FAILED_TO_CREATE_LOG_FILE 178
#define ssLOG_MISSING_FUNCTION_WRAPPER 179

// =======================================================================
// Helper macro functions
// =======================================================================
#if ssLOG_LOG_TO_FILE
    #include <fstream>
    #include <ctime>
    extern std::ofstream ssLogFileStream;

    #define ssLOG_BASE(x)\
    do {\
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
                INTERNAL_ssLOG_EXIT_PROGRAM_1(ssLOG_FAILED_TO_CREATE_LOG_FILE);\
            }\
        }\
        ssLogFileStream << x << "\n";\
    } while(0)
#else
    //#ifdef _WIN32
    //    #include <windows.h>
    //    #undef max
    //    #undef DELETE

    //    inline void ssLOG_SetWinConsoleUTF()
    //    {
    //        static bool ssCalled = false;
    //        if(!ssCalled)
    //        {
    //            ssCalled = true;
    //            SetConsoleOutputCP(CP_UTF8);
    //        }
    //    }
    //    #define INTERNAL_SET_WIN_CONSOLE() ssLOG_SetWinConsoleUTF()
    //#else
    //    #define INTERNAL_SET_WIN_CONSOLE()
    //#endif

    #include <iostream>
    #define ssLOG_BASE(x)\
    do{\
        std::cout<<x<<"\n";\
    } while(0)
#endif

//NOTE: Legacy for ssLOG_SIMPLE --> ssLOG_BASE
#define ssLOG_SIMPLE(x) ssLOG_BASE(x)

#if ssLOG_WRAP_WITH_BRACKET
    #define INTERNAL_ssLOG_WRAP_CONTENT(x) "[" << x << "]"
#else
    #define INTERNAL_ssLOG_WRAP_CONTENT(x) x
#endif


#if ssLOG_SHOW_FILE_NAME
    extern std::string (*Internal_ssLogGetFileName)(std::string);

    #define INTERNAL_ssLOG_GET_FILE_NAME() Internal_ssLogGetFileName(__FILE__)
#else
    #define INTERNAL_ssLOG_GET_FILE_NAME() ""
#endif

#if ssLOG_SHOW_LINE_NUM
    #define INTERNAL_ssLOG_GET_LINE_NUM() " on line "<<__LINE__
#else
    #define INTERNAL_ssLOG_GET_LINE_NUM() ""
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
    extern std::string(*Internal_ssLogGetTime)(void);
    #define INTERNAL_ssLOG_GET_TIME() Internal_ssLogGetTime()
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
            std::stringstream CurrentPrepend;
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
            ssLOG_BASE("");\
            ssLOG_BASE("");\
            ssLOG_BASE("===================================================================");\
            ssLOG_BASE("Thread: "<<std::this_thread::get_id());\
            ssLOG_BASE("===================================================================");\
        }\
    }

    #define INTERNAL_ssLOG_THREAD_SAFE_OP(x)\
    {\
        ssLogMutex.lock();\
        INTERNAL_ssLOG_CHECK_THREAD_DIFF();\
        x;\
        ssLogMutex.unlock();\
    }\

    #define INTERNAL_ssLOG_GET_TAB_SPACE() ssLogInfoMap[std::this_thread::get_id()].TabSpace

    #define INTERNAL_ssLOG_GET_FUNC_STACK() ssLogInfoMap[std::this_thread::get_id()].FuncNameStack    
#else
    extern int ssTabSpace;
    extern std::stack<std::string> ssFuncNameStack;
    extern std::stringstream ssCurrentPrepend;

    #define INTERNAL_ssLOG_THREAD_SAFE_OP(x) x

    #define INTERNAL_ssLOG_GET_TAB_SPACE() ssTabSpace

    #define INTERNAL_ssLOG_GET_FUNC_STACK() ssFuncNameStack
    
    #define INTERNAL_ssLOG_CHECK_THREAD_DIFF(x) x
#endif

extern std::string(*Internal_ssLogGetPrepend)(void);
#define INTERNAL_ssLOG_GET_PREPEND() Internal_ssLogGetPrepend()

// =======================================================================
// Macros for ssLOG_LINE and ssLOG_FUNC 
// =======================================================================

#define ssLOG_LINE( ... ) do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_LINE, __VA_ARGS__ ) } while(0)

#define INTERNAL_ssLOG_LINE_NOT_SAFE( ... ) do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_LINE_NOT_SAFE, __VA_ARGS__ ) } while(0)

#if !ssLOG_CALL_STACK
    #define ssLOG_FUNC( ... )
    #define ssLOG_FUNC_ENTRY( ... ) 
    #define ssLOG_FUNC_EXIT( ... )
    #define ssLOG_FUNC_CONTENT( ... )

    #define INTERNAL_ssLOG_LINE_0()\
    {\
        INTERNAL_ssLOG_THREAD_SAFE_OP(INTERNAL_ssLOG_LINE_NOT_SAFE_0());\
    }

    #define INTERNAL_ssLOG_LINE_1(debugText)\
    {\
        INTERNAL_ssLOG_THREAD_SAFE_OP(INTERNAL_ssLOG_LINE_NOT_SAFE_1(debugText));\
    }
    
    #define INTERNAL_ssLOG_LINE_NOT_SAFE_0()\
    {\
        ssLOG_BASE(INTERNAL_ssLOG_GET_TIME()<<INTERNAL_ssLOG_GET_LOG_LEVEL()<<INTERNAL_ssLOG_GET_PREPEND()<<INTERNAL_ssLOG_GET_FUNCTION_NAME_0()<<INTERNAL_ssLOG_GET_FILE_NAME()<<INTERNAL_ssLOG_GET_LINE_NUM());\
    }
    
    #define INTERNAL_ssLOG_LINE_NOT_SAFE_1(debugText)\
    {\
        ssLOG_BASE(INTERNAL_ssLOG_GET_TIME()<<INTERNAL_ssLOG_GET_LOG_LEVEL()<<INTERNAL_ssLOG_GET_PREPEND()<<INTERNAL_ssLOG_GET_FUNCTION_NAME_0()<<INTERNAL_ssLOG_GET_FILE_NAME()<<INTERNAL_ssLOG_GET_LINE_NUM()<<": " << INTERNAL_ssLOG_WRAP_CONTENT(debugText));\
    }

#else
    inline std::string Internal_ssLog_TabAdder(int tabAmount, bool tree = false)
    {
        std::string returnString = "";
        for(int tab = 0; tab < tabAmount; tab++)
        {
            #if ssLOG_ASCII
                if(tab == tabAmount - 1 && tree)
                    returnString += "|=>";
                else
                    returnString += "|  ";
            #else
                if(tab == tabAmount - 1 && tree)
                    returnString += "├─►";
                else
                    returnString += "│  ";
            #endif
        }

        return returnString;
    }

    #define INTERNAL_ssLOG_LINE_0()\
    {\
        INTERNAL_ssLOG_THREAD_SAFE_OP(INTERNAL_ssLOG_LINE_NOT_SAFE_0());\
    }

    #define INTERNAL_ssLOG_LINE_1(debugText)\
    {\
        INTERNAL_ssLOG_THREAD_SAFE_OP(INTERNAL_ssLOG_LINE_NOT_SAFE_1(debugText));\
    }
    
    #define INTERNAL_ssLOG_LINE_NOT_SAFE_0()\
    {\
        ssLOG_BASE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE(), true)<<INTERNAL_ssLOG_GET_LOG_LEVEL()<<INTERNAL_ssLOG_GET_PREPEND()<<INTERNAL_ssLOG_GET_FUNCTION_NAME_0()<<INTERNAL_ssLOG_GET_FILE_NAME()<<INTERNAL_ssLOG_GET_LINE_NUM());\
    }

    #define INTERNAL_ssLOG_LINE_NOT_SAFE_1(debugText)\
    {\
        ssLOG_BASE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE(), true)<<INTERNAL_ssLOG_GET_LOG_LEVEL()<<INTERNAL_ssLOG_GET_PREPEND()<<INTERNAL_ssLOG_GET_FUNCTION_NAME_0()<<INTERNAL_ssLOG_GET_FILE_NAME()<<INTERNAL_ssLOG_GET_LINE_NUM()<<": "<<INTERNAL_ssLOG_WRAP_CONTENT(debugText));\
    }
    
    #define INTERNAL_ssLOG_Q(x) (std::string(#x).size() > 50 ? std::string(#x).substr(0, 50) + " ..." : #x)

    #define ssLOG_FUNC( ... ) INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC, __VA_ARGS__ )

    #define ssLOG_FUNC_CONTENT(expr)\
    INTERNAL_ssLOG_THREAD_SAFE_OP({\
        ssLOG_BASE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE()));\
        ssLOG_BASE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE(), true)<<INTERNAL_ssLOG_GET_PREPEND()<<INTERNAL_ssLOG_GET_FUNCTION_NAME_1(INTERNAL_ssLOG_Q(expr))<<INTERNAL_ssLOG_GET_FILE_NAME()<<INTERNAL_ssLOG_GET_LINE_NUM()<<": [Entry]");\
        INTERNAL_ssLOG_GET_FUNC_STACK().push(INTERNAL_ssLOG_Q(expr));\
        INTERNAL_ssLOG_GET_TAB_SPACE()++;\
    });\
    expr;\
    INTERNAL_ssLOG_THREAD_SAFE_OP({\
        if(INTERNAL_ssLOG_GET_FUNC_STACK().empty() || INTERNAL_ssLOG_GET_FUNC_STACK().top() != INTERNAL_ssLOG_Q(expr))\
        {\
            ssLOG_BASE("ssLOG_FUNC_EXIT is missing somewhere. "<<INTERNAL_ssLOG_GET_FUNC_STACK().top()<<" is expected but"<<INTERNAL_ssLOG_Q(expr)<<" is found instead.");\
            INTERNAL_ssLOG_EXIT_PROGRAM_1(ssLOG_MISSING_FUNCTION_WRAPPER);\
        }\
        INTERNAL_ssLOG_GET_FUNC_STACK().pop();\
        INTERNAL_ssLOG_GET_TAB_SPACE()--;\
        ssLOG_BASE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE())<<INTERNAL_ssLOG_GET_PREPEND()<<INTERNAL_ssLOG_GET_FUNCTION_NAME_1(INTERNAL_ssLOG_Q(expr))<<INTERNAL_ssLOG_GET_FILE_NAME()<<INTERNAL_ssLOG_GET_LINE_NUM()<<": [Exit]");\
        ssLOG_BASE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE()));\
    });\

    class Internal_ssLogFunctionScope
    {
        private:
            std::string FuncName;
            std::string FileName;
        
        public:
            inline Internal_ssLogFunctionScope(std::string funcName, std::string fileName, std::string lineNum)
            {
                INTERNAL_ssLOG_THREAD_SAFE_OP
                (
                    ssLOG_BASE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE()));
                    ssLOG_BASE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE(), true)<<INTERNAL_ssLOG_GET_PREPEND()<<funcName<<fileName<<lineNum<<": [Entry]");
                    INTERNAL_ssLOG_GET_FUNC_STACK().push(funcName);
                    INTERNAL_ssLOG_GET_TAB_SPACE()++;
                    
                    FuncName = funcName;
                    FileName = fileName;
                );
            }

            inline ~Internal_ssLogFunctionScope()
            {
                INTERNAL_ssLOG_THREAD_SAFE_OP
                (
                    if(INTERNAL_ssLOG_GET_FUNC_STACK().empty() || INTERNAL_ssLOG_GET_FUNC_STACK().top() != FuncName)
                    {
                        ssLOG_BASE("ssLOG_FUNC_EXIT is expecting "<<INTERNAL_ssLOG_GET_FUNC_STACK().top()<<". "<<FuncName<<" is found instead.");
                        INTERNAL_ssLOG_EXIT_PROGRAM_1(ssLOG_MISSING_FUNCTION_WRAPPER);
                    }
                    INTERNAL_ssLOG_GET_FUNC_STACK().pop();
                    INTERNAL_ssLOG_GET_TAB_SPACE()--;
                    ssLOG_BASE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE())<<INTERNAL_ssLOG_GET_PREPEND()<<FuncName<<FileName<<": [Exit]");
                    ssLOG_BASE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE()));
                );
            }
    };
    
    #define INTERRNAL_ssLOG_TO_STRING(x) (std::stringstream() << x).str()
    
    #define INTERNAL_ssLOG_FUNC_0() Internal_ssLogFunctionScope ssLogScopeObj = Internal_ssLogFunctionScope(INTERRNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_FUNCTION_NAME_0()), INTERRNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_FILE_NAME()), INTERRNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_LINE_NUM()))
    
    #define INTERNAL_ssLOG_FUNC_1(customFunc) Internal_ssLogFunctionScope ssLogScopeObj = Internal_ssLogFunctionScope(INTERRNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_FUNCTION_NAME_1(INTERNAL_ssLOG_Q(customFunc))), INTERRNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_FILE_NAME()), INTERRNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_LINE_NUM()))

    #define ssLOG_FUNC_ENTRY( ... ) do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_ENTRY, __VA_ARGS__ ) } while(0)
    #define ssLOG_FUNC_EXIT( ... ) do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_EXIT, __VA_ARGS__ ) } while(0)

    #define INTERNAL_ssLOG_FUNC_ENTRY_0()\
    INTERNAL_ssLOG_THREAD_SAFE_OP({\
        ssLOG_BASE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE()));\
        ssLOG_BASE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE(), true)<<INTERNAL_ssLOG_GET_PREPEND()<<INTERNAL_ssLOG_GET_FUNCTION_NAME_0()<<INTERNAL_ssLOG_GET_FILE_NAME()<<INTERNAL_ssLOG_GET_LINE_NUM()<<": [Entry]");\
        INTERNAL_ssLOG_GET_FUNC_STACK().push(__func__);\
        INTERNAL_ssLOG_GET_TAB_SPACE()++;\
    });

    #define INTERNAL_ssLOG_FUNC_EXIT_0()\
    INTERNAL_ssLOG_THREAD_SAFE_OP({\
        if(INTERNAL_ssLOG_GET_FUNC_STACK().empty() || INTERNAL_ssLOG_GET_FUNC_STACK().top() != __func__)\
        {\
            ssLOG_BASE("ssLOG_FUNC_EXIT is expecting "<<INTERNAL_ssLOG_GET_FUNC_STACK().top()<<". "<<__func__<<" is found instead.");\
            INTERNAL_ssLOG_EXIT_PROGRAM_1(ssLOG_MISSING_FUNCTION_WRAPPER);\
        }\
        INTERNAL_ssLOG_GET_FUNC_STACK().pop();\
        INTERNAL_ssLOG_GET_TAB_SPACE()--;\
        ssLOG_BASE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE())<<INTERNAL_ssLOG_GET_PREPEND()<<INTERNAL_ssLOG_GET_FUNCTION_NAME_0()<<INTERNAL_ssLOG_GET_FILE_NAME()<<INTERNAL_ssLOG_GET_LINE_NUM()<<": [Exit]");\
        ssLOG_BASE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE()));\
    });

    #define INTERNAL_ssLOG_FUNC_ENTRY_1(customFunc)\
    INTERNAL_ssLOG_THREAD_SAFE_OP({\
        ssLOG_BASE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE()));\
        ssLOG_BASE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE(), true)<<INTERNAL_ssLOG_GET_PREPEND()<<INTERNAL_ssLOG_GET_FUNCTION_NAME_1(INTERNAL_ssLOG_Q(customFunc))<<INTERNAL_ssLOG_GET_FILE_NAME()<<INTERNAL_ssLOG_GET_LINE_NUM()<<": [Entry]");\
        INTERNAL_ssLOG_GET_FUNC_STACK().push(customFunc);\
        INTERNAL_ssLOG_GET_TAB_SPACE()++;\
    });

    #define INTERNAL_ssLOG_FUNC_EXIT_1(customFunc)\
    INTERNAL_ssLOG_THREAD_SAFE_OP({\
        if(INTERNAL_ssLOG_GET_FUNC_STACK().empty() || INTERNAL_ssLOG_GET_FUNC_STACK().top() != customFunc)\
        {\
            ssLOG_BASE("ssLOG_FUNC_EXIT is expecting "<<INTERNAL_ssLOG_GET_FUNC_STACK().top()<<". "<<customFunc<<" is found instead.");\
            INTERNAL_ssLOG_EXIT_PROGRAM_1(ssLOG_MISSING_FUNCTION_WRAPPER);\
        }\
        INTERNAL_ssLOG_GET_FUNC_STACK().pop();\
        INTERNAL_ssLOG_GET_TAB_SPACE()--;\
        ssLOG_BASE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE())<<INTERNAL_ssLOG_GET_PREPEND()<<INTERNAL_ssLOG_GET_FUNCTION_NAME_1(INTERNAL_ssLOG_Q(customFunc))<<INTERNAL_ssLOG_GET_FILE_NAME()<<INTERNAL_ssLOG_GET_LINE_NUM()<<": [Exit]");\
        ssLOG_BASE(INTERNAL_ssLOG_GET_TIME()<<Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE()));\
    });


#endif

// =======================================================================
// Macros for ssLOG_PREPEND
// =======================================================================

#if ssLOG_THREAD_SAFE
    #define ssLOG_PREPEND(x) do{ INTERNAL_ssLOG_THREAD_SAFE_OP(ssLogInfoMap[std::this_thread::get_id()].CurrentPrepend << x;) } while(0)
#else
    #define ssLOG_PREPEND(x) do{ INTERNAL_ssLOG_THREAD_SAFE_OP(ssCurrentPrepend << x;) } while(0)
#endif

// =======================================================================
// Macros for output level
// =======================================================================

#define INTERNAL_ssLOG_DEBUG 5
#define INTERNAL_ssLOG_INFO 4
#define INTERNAL_ssLOG_WARNING 3
#define INTERNAL_ssLOG_ERROR 2
#define INTERNAL_ssLOG_FETAL 1

#if ssLOG_ASCII != 1 && ssLOG_LOG_TO_FILE != 1 && defined(ssLOG_USE_SOURCE)
    #include <cstdint>
    #include "termcolor/termcolor.hpp"
#elif ssLOG_ASCII != 1 && ssLOG_LOG_TO_FILE != 1
    #include <cstdint>
    #include "../../External/termcolor/include/termcolor/termcolor.hpp"
#endif

#ifndef ssLOG_LEVEL
    #define ssLOG_LEVEL 0
#endif

extern int ssLogLevel;

#if ssLOG_ASCII != 1 && ssLOG_LOG_TO_FILE != 1
    template <typename CharT>
    inline std::basic_ostream<CharT>& ApplyLog(std::basic_ostream<CharT>& stream)
    {
        switch(ssLogLevel)
        {
            case INTERNAL_ssLOG_FETAL:
                stream << termcolor::colorize << termcolor::white << termcolor::on_red << "[FETAL]" << termcolor::reset << " ";
                ssLogLevel = 0;
                return stream;
            case INTERNAL_ssLOG_ERROR:
                stream << termcolor::colorize << termcolor::white << termcolor::on_bright_red << "[ERROR]" << termcolor::reset << " ";
                ssLogLevel = 0;
                return stream;
            case INTERNAL_ssLOG_WARNING:
                stream << termcolor::colorize << termcolor::white << termcolor::on_yellow << "[WARNING]" << termcolor::reset << " ";
                ssLogLevel = 0;
                return stream;
            case INTERNAL_ssLOG_INFO:
                stream << termcolor::colorize << termcolor::white << termcolor::on_grey << "[INFO]" << termcolor::reset << " ";
                ssLogLevel = 0;
                return stream;
            case INTERNAL_ssLOG_DEBUG:
                stream << termcolor::colorize << termcolor::white << termcolor::on_green << "[DEBUG]" << termcolor::reset << " ";
                ssLogLevel = 0;
                return stream;
            default:
                ssLogLevel = 0;
                return stream;
        }
    }
#else
    template <typename CharT>
    inline std::basic_ostream<CharT>& ApplyLog(std::basic_ostream<CharT>& stream)
    {
        switch(ssLogLevel)
        {
            case INTERNAL_ssLOG_FETAL:
                stream << "[FETAL] ";
                ssLogLevel = 0;
                return stream;
            case INTERNAL_ssLOG_ERROR:
                stream << "[ERROR] ";
                ssLogLevel = 0;
                return stream;
            case INTERNAL_ssLOG_WARNING:
                stream << "[WARNING] ";
                ssLogLevel = 0;
                return stream;
            case INTERNAL_ssLOG_INFO:
                stream << "[INFO] ";
                ssLogLevel = 0;
                return stream;
            case INTERNAL_ssLOG_DEBUG:
                stream << "[DEBUG] ";
                ssLogLevel = 0;
                return stream;
            default:
                ssLogLevel = 0;
                return stream;
        }
    }
#endif

#define INTERNAL_ssLOG_GET_LOG_LEVEL() ApplyLog

#if ssLOG_LEVEL >= INTERNAL_ssLOG_FETAL
    #define ssLOG_FETAL(...) do{ INTERNAL_ssLOG_THREAD_SAFE_OP( ssLogLevel = INTERNAL_ssLOG_FETAL; INTERNAL_ssLOG_LINE_NOT_SAFE(__VA_ARGS__); )} while(0)
#else
    #define ssLOG_FETAL(...) do{} while(0) 
#endif

#if ssLOG_LEVEL >= INTERNAL_ssLOG_ERROR
    #define ssLOG_ERROR(...) do{ INTERNAL_ssLOG_THREAD_SAFE_OP( ssLogLevel = INTERNAL_ssLOG_ERROR; INTERNAL_ssLOG_LINE_NOT_SAFE(__VA_ARGS__); )} while(0)
#else
    #define ssLOG_ERROR(...) do{} while(0)
#endif

#if ssLOG_LEVEL >= INTERNAL_ssLOG_WARNING
    #define ssLOG_WARNING(...) do{ INTERNAL_ssLOG_THREAD_SAFE_OP( ssLogLevel = INTERNAL_ssLOG_WARNING; INTERNAL_ssLOG_LINE_NOT_SAFE(__VA_ARGS__); )} while(0)
#else
    #define ssLOG_WARNING(...) do{} while(0)
#endif

#if ssLOG_LEVEL >= INTERNAL_ssLOG_INFO
    #define ssLOG_INFO(...) do{ INTERNAL_ssLOG_THREAD_SAFE_OP( ssLogLevel = INTERNAL_ssLOG_INFO; INTERNAL_ssLOG_LINE_NOT_SAFE(__VA_ARGS__); )} while(0)
#else
    #define ssLOG_INFO(...) do{} while(0)
#endif

#if ssLOG_LEVEL >= INTERNAL_ssLOG_DEBUG
    #define ssLOG_DEBUG(...) do{ INTERNAL_ssLOG_THREAD_SAFE_OP( ssLogLevel = INTERNAL_ssLOG_DEBUG; INTERNAL_ssLOG_LINE_NOT_SAFE(__VA_ARGS__); )} while(0)
#else
    #define ssLOG_DEBUG(...) do{} while(0)
#endif


#endif

