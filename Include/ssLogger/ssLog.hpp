#ifndef ssLOG_HPP
#define ssLOG_HPP

#include "./ssLogSwitches.hpp"

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
#define INTERNAL_ssLOG_VA_SIZE( ... ) \
    INTERNAL_ssLOG_COMPOSE( INTERNAL_ssLOG_GET_COUNT, \
                            (INTERNAL_ssLOG_EXPAND __VA_ARGS__ (), 0, 6, 5, 4, 3, 2, 1) )

#ifndef _MSC_VER
    #define INTERNAL_ssLOG_VA_SELECT( NAME, ... ) \
        INTERNAL_ssLOG_SELECT( NAME, INTERNAL_ssLOG_VA_SIZE(__VA_ARGS__) )(__VA_ARGS__)
#else
    //MSVC workaround: https://stackoverflow.com/questions/48710758/how-to-fix-variadic-macro-related-issues-with-macro-overloading-in-msvc-mic
    //This is honestly retarded.
    #define INTERNAL_ssLOG_VA_ARGS_FIX( macro, args ) macro args
    #define INTERNAL_ssLOG_VA_SELECT( NAME, ... ) \
        INTERNAL_ssLOG_VA_ARGS_FIX( INTERNAL_ssLOG_SELECT, \
                                    ( NAME, INTERNAL_ssLOG_VA_SIZE( __VA_ARGS__ ) )) (__VA_ARGS__)
#endif

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

    #define ssLOG_BASE(x) \
    do { \
        if(!ssLogFileStream.good()) \
            break; \
        \
        if(!ssLogFileStream.is_open()) \
        { \
            time_t ssRawtime; \
            struct tm* ssTimeinfo; \
            char ssBuffer [80]; \
            time(&ssRawtime); \
            ssTimeinfo = localtime(&ssRawtime); \
            strftime(ssBuffer, 80, "%a %b %d %H_%M_%S %Y", ssTimeinfo); \
            std::string nowString = std::string(ssBuffer)+"_log.txt"; \
            ssLogFileStream.open(nowString, std::ofstream::out); \
            \
            if(!ssLogFileStream.good()) \
                break; \
        }\
        ssLogFileStream << x << std::endl; \
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
    #define ssLOG_BASE(x) \
    do{ \
        std::cout<<x<<std::endl; \
    } while(0)
#endif

//NOTE: ssLOG_BASE replaces ssLOG_SIMPLE for legacy reasons
#define ssLOG_SIMPLE(x) ssLOG_BASE(x)

#if ssLOG_SHOW_FILE_NAME
    inline std::string Internal_ssLogGetFileName(std::string fileName)
    {
        std::size_t ssLogfound = fileName.find_last_of("/\\");
        return " in " + fileName.substr(ssLogfound+1);
    };

    #define INTERNAL_ssLOG_GET_FILE_NAME() Internal_ssLogGetFileName(__FILE__)
#else
    #define INTERNAL_ssLOG_GET_FILE_NAME() ""
#endif

#if ssLOG_SHOW_LINE_NUM
    #define INTERNAL_ssLOG_GET_LINE_NUM() " on line " << __LINE__
#else
    #define INTERNAL_ssLOG_GET_LINE_NUM() ""
#endif

//NOTE: Nesting INTERNAL_ssLOG_VA_SELECT doesn't work :/
//#define GET_FUNCTION_NAME( ... ) INTERNAL_ssLOG_VA_SELECT( GET_FUNCTION_NAME, __VA_ARGS__ )

#if ssLOG_SHOW_FUNC_NAME
    #define INTERNAL_ssLOG_GET_FUNCTION_NAME_0() __func__ << "()"
    #define INTERNAL_ssLOG_GET_FUNCTION_NAME_1(x) x << "()"
    
    #define INTERNAL_ssLOG_GET_CONTENT_NAME(x) x
#else
    #define INTERNAL_ssLOG_GET_FUNCTION_NAME_0() ""
    #define INTERNAL_ssLOG_GET_FUNCTION_NAME_1(x) ""
    
    #define INTERNAL_ssLOG_GET_CONTENT_NAME(x) ""
#endif

#if ssLOG_SHOW_TIME || ssLOG_SHOW_DATE
    extern std::string(*Internal_ssLogGetDateTime)(void);
    #define INTERNAL_ssLOG_GET_DATE_TIME() Internal_ssLogGetDateTime()
#else
    #define INTERNAL_ssLOG_GET_DATE_TIME() ""
#endif

#if ssLOG_THREAD_SAFE
    #include <unordered_map>
    #include <thread>
    #include <mutex>

    #include "./ssLogThreadInfo.hpp"

    extern std::unordered_map<std::thread::id, ssLogThreadInfo> ssLogInfoMap;
    extern int ssNewThreadID;
    extern int ssCurrentThreadID;
    extern std::thread::id ssLastThreadID;
    extern std::mutex ssLogMutex;
    extern std::stack<int> ssLogLevelStack;

    #define INTERNAL_ssLOG_CHECK_THREAD_DIFF() \
    { \
        if(ssLastThreadID != std::this_thread::get_id()) \
        {\
            ssLastThreadID = std::this_thread::get_id(); \
            if(ssLogInfoMap.find(ssLastThreadID) == ssLogInfoMap.end()) \
            { \
                ssLogInfoMap[ssLastThreadID].ID = ssNewThreadID; \
                ssNewThreadID++; \
            } \
            \
            ssCurrentThreadID = ssLogInfoMap[ssLastThreadID].ID; \
        }\
    }

    #define INTERNAL_ssLOG_GET_THREAD_ID() "[Thread " << ssCurrentThreadID << "] "

    #define INTERNAL_ssLOG_THREAD_SAFE_OP(x) \
    { \
        ssLogMutex.lock(); \
        INTERNAL_ssLOG_CHECK_THREAD_DIFF(); \
        x; \
        ssLogMutex.unlock(); \
    }

    #define INTERNAL_ssLOG_GET_TAB_SPACE() ssLogInfoMap[std::this_thread::get_id()].TabSpace

    #define INTERNAL_ssLOG_GET_FUNC_NAME_STACK() \
        ssLogInfoMap[std::this_thread::get_id()].FuncNameStack
    
    #define INTERNAL_ssLOG_GET_FUNC_LOG_LEVEL_STACK() \
        ssLogInfoMap[std::this_thread::get_id()].LogLevelStack    
#else
    extern int ssTabSpace;
    extern std::stack<std::string> ssFuncNameStack;
    extern std::stringstream ssCurrentPrepend;
    extern std::stack<int> ssLogLevelStack;

    #define INTERNAL_ssLOG_CHECK_THREAD_DIFF(x) x

    #define INTERNAL_ssLOG_GET_THREAD_ID() ""

    #define INTERNAL_ssLOG_THREAD_SAFE_OP(x) x

    #define INTERNAL_ssLOG_GET_TAB_SPACE() ssTabSpace

    #define INTERNAL_ssLOG_GET_FUNC_NAME_STACK() ssFuncNameStack
    
    #define INTERNAL_ssLOG_GET_FUNC_LOG_LEVEL_STACK() ssLogLevelStack
#endif

extern int ssCurrentLogLevel;
extern std::string(*Internal_ssLogGetPrepend)(void);
#define INTERNAL_ssLOG_GET_PREPEND() Internal_ssLogGetPrepend()

// =======================================================================
// Macros for ssLOG_LINE and ssLOG_FUNC 
// =======================================================================

#define ssLOG_LINE( ... ) do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_LINE, __VA_ARGS__ ) } while(0)

#define INTERNAL_ssLOG_LINE_NOT_SAFE( ... ) \
    do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_LINE_NOT_SAFE, __VA_ARGS__ ) } while(0)

//NOTE: ssLOG_CONTENT replaces ssLOG_FUNC_CONTENT
#define ssLOG_CONTENT( ... ) ssLOG_FUNC_CONTENT( __VA_ARGS__ )

#define INTERNAL_ssLOG_GET_LOG_LEVEL() ApplyLog

#if !ssLOG_CALL_STACK
    #define ssLOG_FUNC( ... ) do{}while(0)
    #define ssLOG_FUNC_ENTRY( ... ) do{}while(0)
    #define ssLOG_FUNC_EXIT( ... ) do{}while(0)
    #define ssLOG_FUNC_CONTENT( ... ) do{}while(0)

    #define INTERNAL_ssLOG_LINE_0() \
    { \
        INTERNAL_ssLOG_THREAD_SAFE_OP(INTERNAL_ssLOG_LINE_NOT_SAFE_0()); \
    }

    #define INTERNAL_ssLOG_LINE_1(debugText) \
    { \
        INTERNAL_ssLOG_THREAD_SAFE_OP(INTERNAL_ssLOG_LINE_NOT_SAFE_1(debugText)); \
    }
    
    #define INTERNAL_ssLOG_LINE_NOT_SAFE_0() \
    { \
        ssLOG_BASE( INTERNAL_ssLOG_GET_THREAD_ID() << \
                    INTERNAL_ssLOG_GET_DATE_TIME() << \
                    INTERNAL_ssLOG_GET_LOG_LEVEL() << \
                    INTERNAL_ssLOG_GET_PREPEND() << \
                    INTERNAL_ssLOG_GET_FUNCTION_NAME_0() << \
                    INTERNAL_ssLOG_GET_FILE_NAME() << \
                    INTERNAL_ssLOG_GET_LINE_NUM()); \
    }
    
    #define INTERNAL_ssLOG_LINE_NOT_SAFE_1(debugText) \
    { \
        ssLOG_BASE( INTERNAL_ssLOG_GET_THREAD_ID() << \
                    INTERNAL_ssLOG_GET_DATE_TIME() << \
                    INTERNAL_ssLOG_GET_LOG_LEVEL() << \
                    INTERNAL_ssLOG_GET_PREPEND() << \
                    INTERNAL_ssLOG_GET_FUNCTION_NAME_0() << \
                    INTERNAL_ssLOG_GET_FILE_NAME() << \
                    INTERNAL_ssLOG_GET_LINE_NUM() << ": " << \
                    debugText); \
    }
#else
    inline std::string Internal_ssLog_TabAdder(int tabAmount, bool tree = false)
    {
        std::string returnString = "";
        for(int tab = 0; tab < tabAmount; tab++)
        {
            #if ssLOG_ASCII
                if(tab == tabAmount - 1 && tree)
                    returnString += "|=> ";
                else
                    returnString += "|  ";
            #else
                if(tab == tabAmount - 1 && tree)
                    returnString += "├─► ";
                else
                    returnString += "│  ";
            #endif
        }

        return returnString;
    }

    #define INTERNAL_ssLOG_LINE_0() \
    { \
        INTERNAL_ssLOG_THREAD_SAFE_OP(INTERNAL_ssLOG_LINE_NOT_SAFE_0()); \
    }

    #define INTERNAL_ssLOG_LINE_1(debugText) \
    { \
        INTERNAL_ssLOG_THREAD_SAFE_OP(INTERNAL_ssLOG_LINE_NOT_SAFE_1(debugText)); \
    }
    
    #define INTERNAL_ssLOG_LINE_NOT_SAFE_0() \
    { \
        ssLOG_BASE( INTERNAL_ssLOG_GET_THREAD_ID() << \
                    INTERNAL_ssLOG_GET_DATE_TIME() << \
                    Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE(), true) << \
                    INTERNAL_ssLOG_GET_LOG_LEVEL() << \
                    INTERNAL_ssLOG_GET_PREPEND() << \
                    INTERNAL_ssLOG_GET_FUNCTION_NAME_0() << \
                    INTERNAL_ssLOG_GET_FILE_NAME() << \
                    INTERNAL_ssLOG_GET_LINE_NUM()); \
    }

    #define INTERNAL_ssLOG_LINE_NOT_SAFE_1(debugText) \
    { \
        ssLOG_BASE( INTERNAL_ssLOG_GET_THREAD_ID() << \
                    INTERNAL_ssLOG_GET_DATE_TIME() << \
                    Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE(), true) << \
                    INTERNAL_ssLOG_GET_LOG_LEVEL() << \
                    INTERNAL_ssLOG_GET_PREPEND() << \
                    INTERNAL_ssLOG_GET_FUNCTION_NAME_0() << \
                    INTERNAL_ssLOG_GET_FILE_NAME() << \
                    INTERNAL_ssLOG_GET_LINE_NUM() << ": " << \
                    debugText); \
    }
    
    //INTERNAL_ssLOG_LIMIT_EXPR turns any expression into string limiting it to 50 characters
    #define INTERNAL_ssLOG_LIMIT_EXPR(x) \
        (std::string(#x).size() > 50 ? std::string(#x).substr(0, 50) + " ..." : #x)
    
    #define INTERNAL_ssLOG_LIMIT_STR(x) \
        (std::string(x).size() > 50 ? std::string(x).substr(0, 50) + " ..." : x)

    #define ssLOG_FUNC( ... ) INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC, __VA_ARGS__ )

    #define ssLOG_FUNC_CONTENT(expr) \
    INTERNAL_ssLOG_THREAD_SAFE_OP \
    ( \
        INTERNAL_ssLOG_GET_FUNC_LOG_LEVEL_STACK().push(ssCurrentLogLevel); \
        ssLOG_BASE( INTERNAL_ssLOG_GET_THREAD_ID() << \
                    INTERNAL_ssLOG_GET_DATE_TIME() << \
                    Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE())); \
        \
        ssLOG_BASE( INTERNAL_ssLOG_GET_THREAD_ID() << \
                    INTERNAL_ssLOG_GET_DATE_TIME() << \
                    Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE(), true) << \
                    INTERNAL_ssLOG_GET_LOG_LEVEL() << \
                    INTERNAL_ssLOG_GET_PREPEND() << \
                    INTERNAL_ssLOG_GET_CONTENT_NAME("[" << INTERNAL_ssLOG_LIMIT_EXPR(expr) << "]") << \
                    " BEGINS" << \
                    INTERNAL_ssLOG_GET_FILE_NAME() << \
                    INTERNAL_ssLOG_GET_LINE_NUM()); \
        \
        INTERNAL_ssLOG_GET_FUNC_NAME_STACK().push(INTERNAL_ssLOG_LIMIT_EXPR(expr)); \
        INTERNAL_ssLOG_GET_TAB_SPACE()++; \
    ); \
    expr; \
    INTERNAL_ssLOG_THREAD_SAFE_OP \
    ( \
        do \
        { \
            if( INTERNAL_ssLOG_GET_FUNC_NAME_STACK().empty() || \
                INTERNAL_ssLOG_GET_FUNC_NAME_STACK().top() != INTERNAL_ssLOG_LIMIT_EXPR(expr)) \
            { \
                ssLOG_BASE( "ssLOG_FUNC_EXIT is missing somewhere. " << \
                            INTERNAL_ssLOG_GET_FUNC_NAME_STACK().top() << \
                            " is expected but" << INTERNAL_ssLOG_LIMIT_EXPR(expr) << " is found instead."); \
                break; \
            }\
            ssCurrentLogLevel = INTERNAL_ssLOG_GET_FUNC_LOG_LEVEL_STACK().top(); \
            INTERNAL_ssLOG_GET_FUNC_LOG_LEVEL_STACK().pop(); \
            INTERNAL_ssLOG_GET_FUNC_NAME_STACK().pop(); \
            INTERNAL_ssLOG_GET_TAB_SPACE()--; \
            ssLOG_BASE( INTERNAL_ssLOG_GET_THREAD_ID() << \
                        INTERNAL_ssLOG_GET_DATE_TIME() << \
                        Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE()) << \
                        INTERNAL_ssLOG_GET_LOG_LEVEL() << \
                        INTERNAL_ssLOG_GET_PREPEND() << \
                        INTERNAL_ssLOG_GET_CONTENT_NAME("[" << INTERNAL_ssLOG_LIMIT_EXPR(expr) << "]") << \
                        " ENDS" << \
                        INTERNAL_ssLOG_GET_FILE_NAME() << \
                        INTERNAL_ssLOG_GET_LINE_NUM()); \
            \
            ssLOG_BASE( INTERNAL_ssLOG_GET_THREAD_ID() << \
                        INTERNAL_ssLOG_GET_DATE_TIME() << \
                        Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE())); \
        } \
        while(0) \
    );

    template <typename CharT>
    std::basic_ostream<CharT>& ApplyLog(std::basic_ostream<CharT>& stream);

    class Internal_ssLogFunctionScope
    {
        private:
            std::string FuncName;
            std::string FileName;
        
        public:
            inline Internal_ssLogFunctionScope( std::string funcName, 
                                                std::string fileName, 
                                                std::string lineNum)
            {
                INTERNAL_ssLOG_THREAD_SAFE_OP
                (
                    INTERNAL_ssLOG_GET_FUNC_LOG_LEVEL_STACK().push(ssCurrentLogLevel);
                    ssLOG_BASE( INTERNAL_ssLOG_GET_THREAD_ID() <<
                                INTERNAL_ssLOG_GET_DATE_TIME() << 
                                Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE()));
                    
                    ssLOG_BASE( INTERNAL_ssLOG_GET_THREAD_ID() <<
                                INTERNAL_ssLOG_GET_DATE_TIME() << 
                                Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE(), true) << 
                                INTERNAL_ssLOG_GET_LOG_LEVEL() << 
                                INTERNAL_ssLOG_GET_PREPEND() << 
                                funcName << " BEGINS" <<
                                fileName << lineNum);
                    
                    INTERNAL_ssLOG_GET_FUNC_NAME_STACK().push(funcName);
                    INTERNAL_ssLOG_GET_TAB_SPACE()++;
                    
                    FuncName = funcName;
                    FileName = fileName;
                );
            }

            inline ~Internal_ssLogFunctionScope()
            {
                INTERNAL_ssLOG_THREAD_SAFE_OP
                (
                    do
                    {
                        if( INTERNAL_ssLOG_GET_FUNC_NAME_STACK().empty() ||
                            INTERNAL_ssLOG_GET_FUNC_NAME_STACK().top() != FuncName)
                        {
                            ssLOG_BASE( "ssLOG_FUNC_EXIT is expecting " << 
                                        INTERNAL_ssLOG_GET_FUNC_NAME_STACK().top() << ". " << 
                                        FuncName << " is found instead.");

                            break;
                        }
                        INTERNAL_ssLOG_GET_FUNC_NAME_STACK().pop();
                        INTERNAL_ssLOG_GET_TAB_SPACE()--;
                        
                        ssCurrentLogLevel = INTERNAL_ssLOG_GET_FUNC_LOG_LEVEL_STACK().top();
                        INTERNAL_ssLOG_GET_FUNC_LOG_LEVEL_STACK().pop();
                        ssLOG_BASE( INTERNAL_ssLOG_GET_THREAD_ID() <<
                                    INTERNAL_ssLOG_GET_DATE_TIME() << 
                                    Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE()) << 
                                    INTERNAL_ssLOG_GET_LOG_LEVEL() << 
                                    INTERNAL_ssLOG_GET_PREPEND() << 
                                    FuncName << " ENDS" << FileName);
                        
                        ssLOG_BASE( INTERNAL_ssLOG_GET_THREAD_ID() <<
                                    INTERNAL_ssLOG_GET_DATE_TIME() <<
                                    Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE()));
                    } \
                    while(0) \
                );
            }
    };
    
    #define INTERNAL_ssLOG_TO_STRING(x) (std::stringstream() << x).str()
    
    #define INTERNAL_ssLOG_FUNC_0() Internal_ssLogFunctionScope ssLogScopeObj = \
        Internal_ssLogFunctionScope(INTERNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_FUNCTION_NAME_0()), \
                                    INTERNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_FILE_NAME()), \
                                    INTERNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_LINE_NUM()))
    
    #define INTERNAL_ssLOG_SHORTEN_CUSTOM_FUNCTION_NAME_STRING(customFuncName) \
        INTERNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_FUNCTION_NAME_1(INTERNAL_ssLOG_LIMIT_STR(customFuncName)))
    
    #define INTERNAL_ssLOG_FUNC_1(customFuncName) Internal_ssLogFunctionScope ssLogScopeObj = \
        Internal_ssLogFunctionScope(INTERNAL_ssLOG_SHORTEN_CUSTOM_FUNCTION_NAME_STRING(customFuncName), \
                                    INTERNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_FILE_NAME()), \
                                    INTERNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_LINE_NUM()))

    #define ssLOG_FUNC_ENTRY( ... ) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_ENTRY, __VA_ARGS__ ) } while(0)
    
    #define ssLOG_FUNC_EXIT( ... ) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_EXIT, __VA_ARGS__ ) } while(0)

    #define INTERNAL_ssLOG_FUNC_ENTRY_0() INTERNAL_ssLOG_FUNC_ENTRY_1( __func__ )
    
    #define INTERNAL_ssLOG_FUNC_EXIT_0() INTERNAL_ssLOG_FUNC_EXIT_1( __func__ )

    #define INTERNAL_ssLOG_FUNC_ENTRY_1(customFuncName) \
    INTERNAL_ssLOG_THREAD_SAFE_OP \
    ( \
        INTERNAL_ssLOG_GET_FUNC_LOG_LEVEL_STACK().push(ssCurrentLogLevel); \
        ssLOG_BASE( INTERNAL_ssLOG_GET_THREAD_ID() << \
                    INTERNAL_ssLOG_GET_DATE_TIME() << \
                    Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE())); \
        \
        ssLOG_BASE( INTERNAL_ssLOG_GET_THREAD_ID() << \
                    INTERNAL_ssLOG_GET_DATE_TIME() << \
                    Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE(), true) << \
                    INTERNAL_ssLOG_GET_LOG_LEVEL() << \
                    INTERNAL_ssLOG_GET_PREPEND() << \
                    INTERNAL_ssLOG_SHORTEN_CUSTOM_FUNCTION_NAME_STRING(customFuncName) << \
                    " BEGINS" << \
                    INTERNAL_ssLOG_GET_FILE_NAME() << \
                    INTERNAL_ssLOG_GET_LINE_NUM()); \
        \
        INTERNAL_ssLOG_GET_FUNC_NAME_STACK().push(customFuncName); \
        INTERNAL_ssLOG_GET_TAB_SPACE()++; \
    );

    #define INTERNAL_ssLOG_FUNC_EXIT_1(customFuncName) \
    INTERNAL_ssLOG_THREAD_SAFE_OP \
    ( \
        do \
        { \
            if( INTERNAL_ssLOG_GET_FUNC_NAME_STACK().empty() || \
                INTERNAL_ssLOG_GET_FUNC_NAME_STACK().top() != customFuncName) \
            { \
                ssLOG_BASE( "ssLOG_FUNC_EXIT is expecting " << \
                            INTERNAL_ssLOG_GET_FUNC_NAME_STACK().top() << ". "<< \
                            customFuncName << " is found instead."); \
                \
                break; \
            }\
            ssCurrentLogLevel = INTERNAL_ssLOG_GET_FUNC_LOG_LEVEL_STACK().top(); \
            INTERNAL_ssLOG_GET_FUNC_NAME_STACK().pop(); \
            INTERNAL_ssLOG_GET_TAB_SPACE()--; \
            INTERNAL_ssLOG_GET_FUNC_LOG_LEVEL_STACK().pop(); \
            ssLOG_BASE( INTERNAL_ssLOG_GET_THREAD_ID() << \
                        INTERNAL_ssLOG_GET_DATE_TIME() << \
                        Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE()) << \
                        INTERNAL_ssLOG_GET_LOG_LEVEL() << \
                        INTERNAL_ssLOG_GET_PREPEND() << \
                        INTERNAL_ssLOG_SHORTEN_CUSTOM_FUNCTION_NAME_STRING(customFuncName) << \
                        " EXITS" << \
                        INTERNAL_ssLOG_GET_FILE_NAME() << \
                        INTERNAL_ssLOG_GET_LINE_NUM()); \
            \
            ssLOG_BASE( INTERNAL_ssLOG_GET_THREAD_ID() << \
                        INTERNAL_ssLOG_GET_DATE_TIME() << \
                        Internal_ssLog_TabAdder(INTERNAL_ssLOG_GET_TAB_SPACE())); \
        } \
        while(0) \
    );
#endif

// =======================================================================
// Macros for ssLOG_PREPEND
// =======================================================================

#if ssLOG_THREAD_SAFE
    #define ssLOG_PREPEND(x) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP(ssLogInfoMap[std::this_thread::get_id()].CurrentPrepend << x;) \
        } while(0)
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
#define INTERNAL_ssLOG_FATAL 1

#include <cstdint>
#include "../../External/termcolor/include/termcolor/termcolor.hpp"

#ifndef ssLOG_LEVEL
    #define ssLOG_LEVEL 0
#endif

#if ssLOG_ASCII != 1 && ssLOG_LOG_TO_FILE != 1
    template <typename CharT>
    inline std::basic_ostream<CharT>& ApplyLog(std::basic_ostream<CharT>& stream)
    {
        switch(ssCurrentLogLevel)
        {
            case INTERNAL_ssLOG_FATAL:
                stream <<   termcolor::colorize << 
                            termcolor::white << 
                            termcolor::on_red << 
                            "[FATAL]" << 
                            termcolor::reset << " ";
                
                ssCurrentLogLevel = 0;
                return stream;
            case INTERNAL_ssLOG_ERROR:
                stream <<   termcolor::colorize << 
                            termcolor::white << 
                            termcolor::on_bright_red << 
                            "[ERROR]" << 
                            termcolor::reset << " ";
                
                ssCurrentLogLevel = 0;
                return stream;
            case INTERNAL_ssLOG_WARNING:
                stream <<   termcolor::colorize << 
                            termcolor::white << 
                            termcolor::on_yellow << 
                            "[WARNING]" << 
                            termcolor::reset << " ";
                
                ssCurrentLogLevel = 0;
                return stream;
            case INTERNAL_ssLOG_INFO:
                stream <<   termcolor::colorize << 
                            termcolor::white << 
                            termcolor::on_grey << 
                            "[INFO]" << 
                            termcolor::reset << " ";
                
                ssCurrentLogLevel = 0;
                return stream;
            case INTERNAL_ssLOG_DEBUG:
                stream <<   termcolor::colorize << 
                            termcolor::white << 
                            termcolor::on_green << 
                            "[DEBUG]" << 
                            termcolor::reset << " ";
                
                ssCurrentLogLevel = 0;
                return stream;
            default:
                ssCurrentLogLevel = 0;
                return stream;
        }
    }
#else
    template <typename CharT>
    inline std::basic_ostream<CharT>& ApplyLog(std::basic_ostream<CharT>& stream)
    {
        switch(ssCurrentLogLevel)
        {
            case INTERNAL_ssLOG_FATAL:
                stream << "[FATAL] ";
                ssCurrentLogLevel = 0;
                return stream;
            case INTERNAL_ssLOG_ERROR:
                stream << "[ERROR] ";
                ssCurrentLogLevel = 0;
                return stream;
            case INTERNAL_ssLOG_WARNING:
                stream << "[WARNING] ";
                ssCurrentLogLevel = 0;
                return stream;
            case INTERNAL_ssLOG_INFO:
                stream << "[INFO] ";
                ssCurrentLogLevel = 0;
                return stream;
            case INTERNAL_ssLOG_DEBUG:
                stream << "[DEBUG] ";
                ssCurrentLogLevel = 0;
                return stream;
            default:
                ssCurrentLogLevel = 0;
                return stream;
        }
    }
#endif

#define INTERNAL_ssLOG_EXECUTE_COMMAND_0()

#define INTERNAL_ssLOG_EXECUTE_COMMAND_1(command) command

#define INTERNAL_ssLOG_EXECUTE_COMMAND_2(a, command) command


#if ssLOG_LEVEL >= INTERNAL_ssLOG_FATAL
    #define ssLOG_FATAL(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP \
            ( \
                ssCurrentLogLevel = INTERNAL_ssLOG_FATAL; \
                INTERNAL_ssLOG_LINE_NOT_SAFE(__VA_ARGS__); \
            ) \
        } while(0)
    
    #define ssLOG_CONTENT_FATAL(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP(ssCurrentLogLevel = INTERNAL_ssLOG_FATAL;) \
            ssLOG_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_CONTENT_FATAL(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP(ssCurrentLogLevel = INTERNAL_ssLOG_FATAL;) \
            ssLOG_FUNC_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_ENTRY_FATAL(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP(ssCurrentLogLevel = INTERNAL_ssLOG_FATAL;) \
            ssLOG_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_EXIT_FATAL(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP(ssCurrentLogLevel = INTERNAL_ssLOG_FATAL;) \
            ssLOG_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_FATAL(...) \
        INTERNAL_ssLOG_THREAD_SAFE_OP( ssCurrentLogLevel = INTERNAL_ssLOG_FATAL; ) \
        ssLOG_FUNC(__VA_ARGS__);
#else
    #define ssLOG_FATAL(...) do{}while(0)
    #define ssLOG_CONTENT_FATAL(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_CONTENT_FATAL(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_ENTRY_FATAL(...) do{}while(0)
    #define ssLOG_FUNC_EXIT_FATAL(...) do{}while(0)
    #define ssLOG_FUNC_FATAL(...) do{}while(0)
#endif

#if ssLOG_LEVEL >= INTERNAL_ssLOG_ERROR
    #define ssLOG_ERROR(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP \
            ( \
                ssCurrentLogLevel = INTERNAL_ssLOG_ERROR; \
                INTERNAL_ssLOG_LINE_NOT_SAFE(__VA_ARGS__); \
            ) \
        } while(0)
    
    #define ssLOG_CONTENT_ERROR(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP(ssCurrentLogLevel = INTERNAL_ssLOG_ERROR;) \
            ssLOG_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_CONTENT_ERROR(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP( \
                ssCurrentLogLevel = INTERNAL_ssLOG_ERROR; \
                ) ssLOG_FUNC_CONTENT(__VA_ARGS__); } while(0)
    #define ssLOG_FUNC_ENTRY_ERROR(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP(ssCurrentLogLevel = INTERNAL_ssLOG_ERROR;) \
            ssLOG_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_EXIT_ERROR(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP(ssCurrentLogLevel = INTERNAL_ssLOG_ERROR;) \
            ssLOG_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_ERROR(...) \
        INTERNAL_ssLOG_THREAD_SAFE_OP( ssCurrentLogLevel = INTERNAL_ssLOG_ERROR; ) ssLOG_FUNC(__VA_ARGS__);
#else
    #define ssLOG_ERROR(...) do{}while(0)
    #define ssLOG_CONTENT_ERROR(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_CONTENT_ERROR(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    #define ssLOG_FUNC_ENTRY_ERROR(...) do{}while(0)
    #define ssLOG_FUNC_EXIT_ERROR(...) do{}while(0)
    #define ssLOG_FUNC_ERROR(...) do{}while(0)
#endif

#if ssLOG_LEVEL >= INTERNAL_ssLOG_WARNING
    #define ssLOG_WARNING(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP \
            ( \
                ssCurrentLogLevel = INTERNAL_ssLOG_WARNING; \
                INTERNAL_ssLOG_LINE_NOT_SAFE(__VA_ARGS__); \
            ) \
        } while(0)
    
    #define ssLOG_CONTENT_WARNING(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP(ssCurrentLogLevel = INTERNAL_ssLOG_WARNING;) \
            ssLOG_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_CONTENT_WARNING(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP( \
                ssCurrentLogLevel = INTERNAL_ssLOG_WARNING; \
                ) ssLOG_FUNC_CONTENT(__VA_ARGS__); } while(0)
    #define ssLOG_FUNC_ENTRY_WARNING(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP(ssCurrentLogLevel = INTERNAL_ssLOG_WARNING;) \
            ssLOG_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_EXIT_WARNING(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP(ssCurrentLogLevel = INTERNAL_ssLOG_WARNING;) \
            ssLOG_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_WARNING(...) \
        INTERNAL_ssLOG_THREAD_SAFE_OP( ssCurrentLogLevel = INTERNAL_ssLOG_WARNING; ) \
        ssLOG_FUNC(__VA_ARGS__);
#else
    #define ssLOG_WARNING(...) do{}while(0)
    #define ssLOG_CONTENT_WARNING(...) INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    #define ssLOG_FUNC_CONTENT_WARNING(...) INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    #define ssLOG_FUNC_ENTRY_WARNING(...) do{}while(0)
    #define ssLOG_FUNC_EXIT_WARNING(...) do{}while(0)
    #define ssLOG_FUNC_WARNING(...) do{}while(0)
#endif

#if ssLOG_LEVEL >= INTERNAL_ssLOG_INFO
    #define ssLOG_INFO(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP \
            ( \
                ssCurrentLogLevel = INTERNAL_ssLOG_INFO; \
                INTERNAL_ssLOG_LINE_NOT_SAFE(__VA_ARGS__); \
            ) \
        } while(0)
    
    #define ssLOG_CONTENT_INFO(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP(ssCurrentLogLevel = INTERNAL_ssLOG_INFO;) \
            ssLOG_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_CONTENT_INFO(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP( \
                ssCurrentLogLevel = INTERNAL_ssLOG_INFO; \
                ) ssLOG_FUNC_CONTENT(__VA_ARGS__); } while(0)
    #define ssLOG_FUNC_ENTRY_INFO(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP(ssCurrentLogLevel = INTERNAL_ssLOG_INFO;) \
            ssLOG_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_EXIT_INFO(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP(ssCurrentLogLevel = INTERNAL_ssLOG_INFO;) \
            ssLOG_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_INFO(...) \
        INTERNAL_ssLOG_THREAD_SAFE_OP( ssCurrentLogLevel = INTERNAL_ssLOG_INFO; ) ssLOG_FUNC(__VA_ARGS__);
#else
    #define ssLOG_INFO(...) do{}while(0)
    #define ssLOG_CONTENT_INFO(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_CONTENT_INFO(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_ENTRY_INFO(...) do{}while(0)
    #define ssLOG_FUNC_EXIT_INFO(...) do{}while(0)
    #define ssLOG_FUNC_INFO(...) do{}while(0)
#endif

#if ssLOG_LEVEL >= INTERNAL_ssLOG_DEBUG
    #define ssLOG_DEBUG(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP \
            ( \
                ssCurrentLogLevel = INTERNAL_ssLOG_DEBUG; \
                INTERNAL_ssLOG_LINE_NOT_SAFE(__VA_ARGS__); \
            ) \
        } while(0)
    
    #define ssLOG_CONTENT_DEBUG(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP(ssCurrentLogLevel = INTERNAL_ssLOG_DEBUG;) \
            ssLOG_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_CONTENT_DEBUG(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP(ssCurrentLogLevel = INTERNAL_ssLOG_DEBUG;) \
            ssLOG_FUNC_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_ENTRY_DEBUG(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP(ssCurrentLogLevel = INTERNAL_ssLOG_DEBUG;) \
            ssLOG_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_EXIT_DEBUG(...) \
        do{ \
            INTERNAL_ssLOG_THREAD_SAFE_OP(ssCurrentLogLevel = INTERNAL_ssLOG_DEBUG;) \
            ssLOG_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_DEBUG(...) \
        INTERNAL_ssLOG_THREAD_SAFE_OP( ssCurrentLogLevel = INTERNAL_ssLOG_DEBUG; ) ssLOG_FUNC(__VA_ARGS__);
#else
    #define ssLOG_DEBUG(...) do{}while(0)
    #define ssLOG_CONTENT_DEBUG(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_CONTENT_DEBUG(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_ENTRY_DEBUG(...) do{}while(0)
    #define ssLOG_FUNC_EXIT_DEBUG(...) do{}while(0)
    #define ssLOG_FUNC_DEBUG(...) do{}while(0)
#endif


#endif

