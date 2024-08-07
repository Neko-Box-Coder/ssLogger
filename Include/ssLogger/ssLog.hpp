#ifndef ssLOG_HPP
#define ssLOG_HPP

#include "./ssLogSwitches.hpp"
#include "./ssLogAPIHelper.hpp"

#include <sstream>
#include <string>
#include <stack>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <utility>

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
#endif //_MSC_VER

// =======================================================================
// Error codes
// =======================================================================
#define ssLOG_FAILED_TO_CREATE_LOG_FILE 178
#define ssLOG_MISSING_FUNCTION_WRAPPER 179

// =======================================================================
// Helper macro functions
// =======================================================================
#if ssLOG_THREAD_SAFE_OUTPUT
    ssLOG_API extern std::mutex ssLogOutputMutex;
    #define INTERNAL_ssLOG_LOCK_OUTPUT() \
        bool startOutputLocked = ssLogInfoMap.at(std::this_thread::get_id()).outputLocked; \
        std::unique_lock<std::mutex> outputLock(ssLogOutputMutex, std::defer_lock); \
        if(outputLock.try_lock()) \
        { \
            startOutputLocked = false; \
            ssLogInfoMap.at(std::this_thread::get_id()).outputLocked = true; \
        } \
        else \
        { \
            if(!startOutputLocked) \
            { \
                outputLock.lock(); \
                ssLogInfoMap.at(std::this_thread::get_id()).outputLocked = true; \
            } \
        }
    
    #define INTERNAL_ssLOG_UNLOCK_OUTPUT() \
        if(!startOutputLocked) \
            ssLogInfoMap.at(std::this_thread::get_id()).outputLocked = false;
#else
    #define INTERNAL_ssLOG_LOCK_OUTPUT()
    #define INTERNAL_ssLOG_UNLOCK_OUTPUT()
#endif //ssLOG_THREAD_SAFE_OUTPUT

#if ssLOG_LOG_TO_FILE
    #include <fstream>
    #include <ctime>
    ssLOG_API extern std::ofstream ssLogFileStream;

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
    #include <iostream>
    #define ssLOG_BASE(x) \
    do{ \
        std::cout << x << std::endl; \
    } while(0)
#endif //ssLOG_LOG_TO_FILE

//TODO(NOW): If INTERNAL_ssLOG_BASE is inside ssLOG_BASE(x), it will cause infinite lock
#define INTERNAL_ssLOG_BASE(x) \
    do \
    { \
        if(INTERNAL_ssLOG_IS_CACHE_OUTPUT()) \
        { \
            INTERNAL_ssLOG_CURRENT_CACHE_OUTPUT() << x << std::endl; \
            break; \
        } \
        \
        INTERNAL_ssLOG_LOCK_OUTPUT(); \
        ssLOG_BASE(x); \
        INTERNAL_ssLOG_UNLOCK_OUTPUT(); \
    } while(0)


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
    ssLOG_API extern std::string(*Internal_ssLogGetDateTime)(void);
    #define INTERNAL_ssLOG_GET_DATE_TIME() Internal_ssLogGetDateTime()
#else
    #define INTERNAL_ssLOG_GET_DATE_TIME() ""
#endif

#include "./ssLogThreadInfo.hpp"

ssLOG_API extern std::unordered_map<std::thread::id, ssLogThreadInfo> ssLogInfoMap;
ssLOG_API extern int ssLogNewThreadID;
ssLOG_API extern std::mutex ssLogMapMutex;
ssLOG_API extern std::atomic<bool> ssLogNewThreadCacheByDefault;

#define INTERNAL_ssLOG_CHECK_NEW_THREAD() \
    do \
    { \
        std::unique_lock<std::mutex> lk(ssLogMapMutex, std::defer_lock); \
        while(!lk.try_lock()){} \
        if(ssLogInfoMap.find(std::this_thread::get_id()) == ssLogInfoMap.end()) \
        { \
            ssLogInfoMap[std::this_thread::get_id()].ID = ssLogNewThreadID++; \
            ssLogInfoMap[std::this_thread::get_id()].CacheOutput = ssLogNewThreadCacheByDefault.load(); \
        } \
    } while(0)

#if ssLOG_SHOW_THREADS
    #define INTERNAL_ssLOG_PRINT_THREAD_ID() "[Thread " << ssLogInfoMap.at(std::this_thread::get_id()).ID << "] "
#else
    #define INTERNAL_ssLOG_PRINT_THREAD_ID()
#endif

#define INTERNAL_ssLOG_TAB_SPACE() ssLogInfoMap.at(std::this_thread::get_id()).TabSpace

#define INTERNAL_ssLOG_FUNC_NAME_STACK() \
    ssLogInfoMap.at(std::this_thread::get_id()).FuncNameStack

#define INTERNAL_ssLOG_FUNC_LOG_LEVEL_STACK() \
    ssLogInfoMap.at(std::this_thread::get_id()).LogLevelStack

#define INTERNAL_ssLOG_CURRENT_LOG_LEVEL() \
    ssLogInfoMap.at(std::this_thread::get_id()).ssCurrentLogLevel

#define INTERNAL_ssLOG_IS_CACHE_OUTPUT() \
    ssLogInfoMap.at(std::this_thread::get_id()).CacheOutput

#define INTERNAL_ssLOG_CURRENT_CACHE_OUTPUT() \
    ssLogInfoMap.at(std::this_thread::get_id()).CurrentCachedOutput

ssLOG_API extern std::string(*Internal_ssLogGetPrepend)(void);
#define INTERNAL_ssLOG_GET_PREPEND() Internal_ssLogGetPrepend()

#define INTERNAL_ssLOG_TARGET_LEVEL() \
    ssLogInfoMap.at(std::this_thread::get_id()).ssTargetLogLevel


// =======================================================================
// Macros for ssLOG_LINE, ssLOG_FUNC, ssLOG_FUNC_ENTRY, ssLOG_FUNC_EXIT and ssLOG_FUNC_CONTENT
// =======================================================================

#define ssLOG_LINE( ... ) do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_LINE, __VA_ARGS__ ) } while(0)

//NOTE: ssLOG_CONTENT replaces ssLOG_FUNC_CONTENT
#define ssLOG_CONTENT( ... ) ssLOG_FUNC_CONTENT( __VA_ARGS__ )

#define INTERNAL_ssLOG_GET_LOG_LEVEL() ApplyLog

#if !ssLOG_CALL_STACK
    #define ssLOG_FUNC( ... ) do{}while(0)
    #define ssLOG_FUNC_ENTRY( ... ) do{}while(0)
    #define ssLOG_FUNC_EXIT( ... ) do{}while(0)
    #define ssLOG_FUNC_CONTENT(expr) expr; do{}while(0)

    #define INTERNAL_ssLOG_LINE_0() \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        if(INTERNAL_ssLOG_TARGET_LEVEL() >= INTERNAL_ssLOG_CURRENT_LOG_LEVEL()) \
        { \
            INTERNAL_ssLOG_BASE(INTERNAL_ssLOG_PRINT_THREAD_ID() << \
                                INTERNAL_ssLOG_GET_DATE_TIME() << \
                                INTERNAL_ssLOG_GET_LOG_LEVEL() << \
                                INTERNAL_ssLOG_GET_PREPEND() << \
                                INTERNAL_ssLOG_GET_FUNCTION_NAME_0() << \
                                INTERNAL_ssLOG_GET_FILE_NAME() << \
                                INTERNAL_ssLOG_GET_LINE_NUM()); \
       } \
       INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = 0;

    #define INTERNAL_ssLOG_LINE_1(debugText) \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        if(INTERNAL_ssLOG_TARGET_LEVEL() >= INTERNAL_ssLOG_CURRENT_LOG_LEVEL()) \
        { \
            INTERNAL_ssLOG_BASE(INTERNAL_ssLOG_PRINT_THREAD_ID() << \
                                INTERNAL_ssLOG_GET_DATE_TIME() << \
                                INTERNAL_ssLOG_GET_LOG_LEVEL() << \
                                INTERNAL_ssLOG_GET_PREPEND() << \
                                INTERNAL_ssLOG_GET_FUNCTION_NAME_0() << \
                                INTERNAL_ssLOG_GET_FILE_NAME() << \
                                INTERNAL_ssLOG_GET_LINE_NUM() << ": " << \
                                debugText); \
        } \
        INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = 0;
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
                    returnString += "|   ";
            #else
                if(tab == tabAmount - 1 && tree)
                    returnString += "├─► ";
                else
                    returnString += "│   ";
            #endif
        }

        return returnString;
    }

    #define INTERNAL_ssLOG_LINE_0() \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        if(INTERNAL_ssLOG_TARGET_LEVEL() >= INTERNAL_ssLOG_CURRENT_LOG_LEVEL()) \
        { \
            INTERNAL_ssLOG_BASE(INTERNAL_ssLOG_PRINT_THREAD_ID() << \
                                INTERNAL_ssLOG_GET_DATE_TIME() << \
                                Internal_ssLog_TabAdder(INTERNAL_ssLOG_TAB_SPACE(), true) << \
                                INTERNAL_ssLOG_GET_LOG_LEVEL() << \
                                INTERNAL_ssLOG_GET_PREPEND() << \
                                INTERNAL_ssLOG_GET_FUNCTION_NAME_0() << \
                                INTERNAL_ssLOG_GET_FILE_NAME() << \
                                INTERNAL_ssLOG_GET_LINE_NUM()); \
        } \
        INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = 0;

    #define INTERNAL_ssLOG_LINE_1(debugText) \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        if(INTERNAL_ssLOG_TARGET_LEVEL() >= INTERNAL_ssLOG_CURRENT_LOG_LEVEL()) \
        { \
            INTERNAL_ssLOG_BASE(INTERNAL_ssLOG_PRINT_THREAD_ID() << \
                                INTERNAL_ssLOG_GET_DATE_TIME() << \
                                Internal_ssLog_TabAdder(INTERNAL_ssLOG_TAB_SPACE(), true) << \
                                INTERNAL_ssLOG_GET_LOG_LEVEL() << \
                                INTERNAL_ssLOG_GET_PREPEND() << \
                                INTERNAL_ssLOG_GET_FUNCTION_NAME_0() << \
                                INTERNAL_ssLOG_GET_FILE_NAME() << \
                                INTERNAL_ssLOG_GET_LINE_NUM() << ": " << \
                                debugText); \
        } \
        INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = 0;
    
    //INTERNAL_ssLOG_LIMIT_EXPR turns any expression into string limiting it to 50 characters
    #define INTERNAL_ssLOG_LIMIT_EXPR(x) \
        (std::string(#x).size() > 50 ? std::string(#x).substr(0, 50) + " ..." : #x)
    
    #define INTERNAL_ssLOG_LIMIT_STR(x) \
        (std::string(x).size() > 50 ? std::string(x).substr(0, 50) + " ..." : x)

    #define ssLOG_FUNC( ... ) INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC, __VA_ARGS__ )

    #define ssLOG_FUNC_CONTENT(expr) \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        INTERNAL_ssLOG_FUNC_LOG_LEVEL_STACK().push(INTERNAL_ssLOG_CURRENT_LOG_LEVEL()); \
        if(INTERNAL_ssLOG_TARGET_LEVEL() >= INTERNAL_ssLOG_CURRENT_LOG_LEVEL()) \
        { \
            INTERNAL_ssLOG_BASE(INTERNAL_ssLOG_PRINT_THREAD_ID() << \
                                INTERNAL_ssLOG_GET_DATE_TIME() << \
                                Internal_ssLog_TabAdder(INTERNAL_ssLOG_TAB_SPACE())); \
            \
            INTERNAL_ssLOG_BASE(INTERNAL_ssLOG_PRINT_THREAD_ID() << \
                                INTERNAL_ssLOG_GET_DATE_TIME() << \
                                Internal_ssLog_TabAdder(INTERNAL_ssLOG_TAB_SPACE(), true) << \
                                INTERNAL_ssLOG_GET_LOG_LEVEL() << \
                                INTERNAL_ssLOG_GET_PREPEND() << \
                                INTERNAL_ssLOG_GET_CONTENT_NAME("[" << INTERNAL_ssLOG_LIMIT_EXPR(expr) << "]") << \
                                " BEGINS" << \
                                INTERNAL_ssLOG_GET_FILE_NAME() << \
                                INTERNAL_ssLOG_GET_LINE_NUM()); \
        } \
        \
        INTERNAL_ssLOG_FUNC_NAME_STACK().push(INTERNAL_ssLOG_LIMIT_EXPR(expr)); \
        INTERNAL_ssLOG_TAB_SPACE()++; \
        \
        expr; \
        \
        do \
        { \
            if( INTERNAL_ssLOG_FUNC_NAME_STACK().empty() || \
                INTERNAL_ssLOG_FUNC_NAME_STACK().top() != INTERNAL_ssLOG_LIMIT_EXPR(expr)) \
            { \
                INTERNAL_ssLOG_BASE("ssLOG_FUNC_EXIT is missing somewhere. " << \
                                    INTERNAL_ssLOG_FUNC_NAME_STACK().top() << \
                                    " is expected but" << INTERNAL_ssLOG_LIMIT_EXPR(expr) << " is found instead."); \
                break; \
            }\
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = INTERNAL_ssLOG_FUNC_LOG_LEVEL_STACK().top(); \
            INTERNAL_ssLOG_FUNC_LOG_LEVEL_STACK().pop(); \
            INTERNAL_ssLOG_FUNC_NAME_STACK().pop(); \
            INTERNAL_ssLOG_TAB_SPACE()--; \
            if(INTERNAL_ssLOG_TARGET_LEVEL() >= INTERNAL_ssLOG_CURRENT_LOG_LEVEL()) \
            { \
                INTERNAL_ssLOG_BASE(INTERNAL_ssLOG_PRINT_THREAD_ID() << \
                                    INTERNAL_ssLOG_GET_DATE_TIME() << \
                                    Internal_ssLog_TabAdder(INTERNAL_ssLOG_TAB_SPACE()) << \
                                    INTERNAL_ssLOG_GET_LOG_LEVEL() << \
                                    INTERNAL_ssLOG_GET_PREPEND() << \
                                    INTERNAL_ssLOG_GET_CONTENT_NAME("[" << INTERNAL_ssLOG_LIMIT_EXPR(expr) << "]") << \
                                    " EXITS" << \
                                    INTERNAL_ssLOG_GET_FILE_NAME() << \
                                    INTERNAL_ssLOG_GET_LINE_NUM()); \
                \
                INTERNAL_ssLOG_BASE(INTERNAL_ssLOG_PRINT_THREAD_ID() << \
                                    INTERNAL_ssLOG_GET_DATE_TIME() << \
                                    Internal_ssLog_TabAdder(INTERNAL_ssLOG_TAB_SPACE())); \
            } \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = 0; \
        } \
        while(0)

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
                INTERNAL_ssLOG_CHECK_NEW_THREAD();
                INTERNAL_ssLOG_FUNC_LOG_LEVEL_STACK().push(INTERNAL_ssLOG_CURRENT_LOG_LEVEL());

                if(INTERNAL_ssLOG_TARGET_LEVEL() >= INTERNAL_ssLOG_CURRENT_LOG_LEVEL())
                {
                    INTERNAL_ssLOG_BASE(INTERNAL_ssLOG_PRINT_THREAD_ID() <<
                                        INTERNAL_ssLOG_GET_DATE_TIME() << 
                                        Internal_ssLog_TabAdder(INTERNAL_ssLOG_TAB_SPACE()));
                    
                    INTERNAL_ssLOG_BASE(INTERNAL_ssLOG_PRINT_THREAD_ID() <<
                                        INTERNAL_ssLOG_GET_DATE_TIME() << 
                                        Internal_ssLog_TabAdder(INTERNAL_ssLOG_TAB_SPACE(), true) << 
                                        INTERNAL_ssLOG_GET_LOG_LEVEL() << 
                                        INTERNAL_ssLOG_GET_PREPEND() << 
                                        funcName << " BEGINS" <<
                                        fileName << lineNum);
                }
                
                INTERNAL_ssLOG_FUNC_NAME_STACK().push(funcName);
                INTERNAL_ssLOG_TAB_SPACE()++;
                
                FuncName = funcName;
                FileName = fileName;
            }

            inline ~Internal_ssLogFunctionScope()
            {
                if( INTERNAL_ssLOG_FUNC_NAME_STACK().empty() ||
                    INTERNAL_ssLOG_FUNC_NAME_STACK().top() != FuncName)
                {
                    INTERNAL_ssLOG_BASE("ssLOG_FUNC_EXIT is expecting " << 
                                        INTERNAL_ssLOG_FUNC_NAME_STACK().top() << ". " << 
                                        FuncName << " is found instead.");
                    
                    return;
                }
                INTERNAL_ssLOG_FUNC_NAME_STACK().pop();
                INTERNAL_ssLOG_TAB_SPACE()--;
                
                INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = INTERNAL_ssLOG_FUNC_LOG_LEVEL_STACK().top();
                INTERNAL_ssLOG_FUNC_LOG_LEVEL_STACK().pop();
                
                if(INTERNAL_ssLOG_TARGET_LEVEL() >= INTERNAL_ssLOG_CURRENT_LOG_LEVEL())
                {
                    INTERNAL_ssLOG_BASE(INTERNAL_ssLOG_PRINT_THREAD_ID() <<
                                        INTERNAL_ssLOG_GET_DATE_TIME() << 
                                        Internal_ssLog_TabAdder(INTERNAL_ssLOG_TAB_SPACE()) << 
                                        INTERNAL_ssLOG_GET_LOG_LEVEL() << 
                                        INTERNAL_ssLOG_GET_PREPEND() << 
                                        FuncName << " EXITS" << FileName);
                    
                    INTERNAL_ssLOG_BASE(INTERNAL_ssLOG_PRINT_THREAD_ID() <<
                                        INTERNAL_ssLOG_GET_DATE_TIME() <<
                                        Internal_ssLog_TabAdder(INTERNAL_ssLOG_TAB_SPACE()));
                }
                
                INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = 0;
            }
    };
    
    #define INTERNAL_ssLOG_TO_STRING(x) (std::stringstream() << x).str()
    
    #define INTERNAL_ssLOG_FUNC_0() \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        Internal_ssLogFunctionScope ssLogScopeObj = \
        Internal_ssLogFunctionScope(INTERNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_FUNCTION_NAME_0()), \
                                    INTERNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_FILE_NAME()), \
                                    INTERNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_LINE_NUM()))
    
    #define INTERNAL_ssLOG_SHORTEN_CUSTOM_FUNCTION_NAME_STRING(customFuncName) \
        INTERNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_FUNCTION_NAME_1(INTERNAL_ssLOG_LIMIT_STR(customFuncName)))
    
    #define INTERNAL_ssLOG_FUNC_1(customFuncName) \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        Internal_ssLogFunctionScope ssLogScopeObj = \
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
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        INTERNAL_ssLOG_FUNC_LOG_LEVEL_STACK().push(INTERNAL_ssLOG_CURRENT_LOG_LEVEL()); \
        if(INTERNAL_ssLOG_TARGET_LEVEL() >= INTERNAL_ssLOG_CURRENT_LOG_LEVEL()) \
        { \
            INTERNAL_ssLOG_BASE(INTERNAL_ssLOG_PRINT_THREAD_ID() << \
                                INTERNAL_ssLOG_GET_DATE_TIME() << \
                                Internal_ssLog_TabAdder(INTERNAL_ssLOG_TAB_SPACE())); \
            \
            INTERNAL_ssLOG_BASE(INTERNAL_ssLOG_PRINT_THREAD_ID() << \
                                INTERNAL_ssLOG_GET_DATE_TIME() << \
                                Internal_ssLog_TabAdder(INTERNAL_ssLOG_TAB_SPACE(), true) << \
                                INTERNAL_ssLOG_GET_LOG_LEVEL() << \
                                INTERNAL_ssLOG_GET_PREPEND() << \
                                INTERNAL_ssLOG_SHORTEN_CUSTOM_FUNCTION_NAME_STRING(customFuncName) << \
                                " BEGINS" << \
                                INTERNAL_ssLOG_GET_FILE_NAME() << \
                                INTERNAL_ssLOG_GET_LINE_NUM()); \
        } \
        \
        INTERNAL_ssLOG_FUNC_NAME_STACK().push(customFuncName); \
        INTERNAL_ssLOG_TAB_SPACE()++; \
        INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = 0;


    #define INTERNAL_ssLOG_FUNC_EXIT_1(customFuncName) \
        if( INTERNAL_ssLOG_FUNC_NAME_STACK().empty() || \
            INTERNAL_ssLOG_FUNC_NAME_STACK().top() != customFuncName) \
        { \
            INTERNAL_ssLOG_BASE("sLOG_FUNC_EXIT is expecting " << \
                                INTERNAL_ssLOG_FUNC_NAME_STACK().top() << ". "<< \
                                customFuncName << " is found instead."); \
            \
            break; \
        } \
        INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = INTERNAL_ssLOG_FUNC_LOG_LEVEL_STACK().top(); \
        INTERNAL_ssLOG_FUNC_NAME_STACK().pop(); \
        INTERNAL_ssLOG_TAB_SPACE()--; \
        INTERNAL_ssLOG_FUNC_LOG_LEVEL_STACK().pop(); \
        if(INTERNAL_ssLOG_TARGET_LEVEL() >= INTERNAL_ssLOG_CURRENT_LOG_LEVEL()) \
        { \
            INTERNAL_ssLOG_BASE(INTERNAL_ssLOG_PRINT_THREAD_ID() << \
                                INTERNAL_ssLOG_GET_DATE_TIME() << \
                                Internal_ssLog_TabAdder(INTERNAL_ssLOG_TAB_SPACE()) << \
                                INTERNAL_ssLOG_GET_LOG_LEVEL() << \
                                INTERNAL_ssLOG_GET_PREPEND() << \
                                INTERNAL_ssLOG_SHORTEN_CUSTOM_FUNCTION_NAME_STRING(customFuncName) << \
                                " EXITS" << \
                                INTERNAL_ssLOG_GET_FILE_NAME() << \
                                INTERNAL_ssLOG_GET_LINE_NUM()); \
            \
            INTERNAL_ssLOG_BASE(INTERNAL_ssLOG_PRINT_THREAD_ID() << \
                                INTERNAL_ssLOG_GET_DATE_TIME() << \
                                Internal_ssLog_TabAdder(INTERNAL_ssLOG_TAB_SPACE())); \
        } \
        INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = 0;
#endif //!ssLOG_CALL_STACK

// =======================================================================
// Macros for ssLOG_PREPEND
// =======================================================================

#define ssLOG_PREPEND(x) \
    do{ \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        ssLogInfoMap.at(std::this_thread::get_id()).CurrentPrepend << x; \
    } while(0)


// =======================================================================
// Macros for ssLOG_ENABLE_CACHE_OUTPUT, ssLOG_DISABLE_CACHE_OUTPUT, 
// ssLOG_ENABLE_CACHE_OUTPUT_FOR_CURRENT_THREAD, ssLOG_DISABLE_CACHE_OUTPUT_FOR_CURRENT_THREAD, 
// ssLOG_ENABLE_CACHE_OUTPUT_FOR_NEW_THREADS, ssLOG_DISABLE_CACHE_OUTPUT_FOR_NEW_THREADS, 
// ssLOG_CACHE_OUTPUT_FOR_SCOPE and ssLOG_OUTPUT_ALL_CACHE
// =======================================================================
#define ssLOG_ENABLE_CACHE_OUTPUT() \
    do \
    { \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        std::unique_lock<std::mutex> lk(ssLogMapMutex); \
        for(auto it = ssLogInfoMap.begin(); it != ssLogInfoMap.end(); ++it) \
            it->second.CacheOutput = true; \
    } while(0)

#define ssLOG_DISABLE_CACHE_OUTPUT() \
    do \
    { \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        for(auto it = ssLogInfoMap.begin(); it != ssLogInfoMap.end(); ++it) \
            it->second.CacheOutput = false; \
    } while(0)

#define ssLOG_ENABLE_CACHE_OUTPUT_FOR_CURRENT_THREAD() \
    do \
    { \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        INTERNAL_ssLOG_IS_CACHE_OUTPUT() = true; \
    } while(0)

#define ssLOG_DISABLE_CACHE_OUTPUT_FOR_CURRENT_THREAD() \
    do \
    { \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        INTERNAL_ssLOG_IS_CACHE_OUTPUT() = false; \
    } while(0)

#define ssLOG_ENABLE_CACHE_OUTPUT_FOR_NEW_THREADS() \
    do \
    { \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        ssLogNewThreadCacheByDefault.store(true); \
    } while(0)

#define ssLOG_DISABLE_CACHE_OUTPUT_FOR_NEW_THREADS() \
    do \
    { \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        ssLogNewThreadCacheByDefault.store(false); \
    } while(0)


class Internal_ssLogCacheScope
{
    public:
        inline Internal_ssLogCacheScope()
        {
            ssLOG_ENABLE_CACHE_OUTPUT_FOR_CURRENT_THREAD();
        }

        inline ~Internal_ssLogCacheScope()
        {
            ssLOG_DISABLE_CACHE_OUTPUT_FOR_CURRENT_THREAD();
        }
};

#define ssLOG_CACHE_OUTPUT_IN_SCOPE() \
    Internal_ssLogCacheScope INTERNAL_ssLOG_SELECT(ssLogCacheScopeObj, __LINE__) = Internal_ssLogCacheScope()

#define ssLOG_OUTPUT_ALL_CACHE() \
    do \
    { \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        std::unique_lock<std::mutex> lk(ssLogMapMutex); \
        for(auto it = ssLogInfoMap.begin(); it != ssLogInfoMap.end(); ++it) \
        { \
            if(it->second.CurrentCachedOutput.rdbuf()->in_avail() == 0) \
                continue; \
            \
            ssLOG_BASE(it->second.CurrentCachedOutput.str()); \
            it->second.CurrentCachedOutput.str(""); \
            it->second.CurrentCachedOutput.clear(); \
        } \
    } while(0)

// =======================================================================
// Macros for ssLOG_BENCH_START and ssLOG_BENCH_END
// =======================================================================

#define ssLOG_BENCH_START( ... ) INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_START, __VA_ARGS__ )

#define ssLOG_BENCH_END( ... ) do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_END, __VA_ARGS__ ); } while(0)

#define INTERNAL_ssLOG_BENCH_START_0() \
    INTERNAL_ssLOG_BENCH_START_1("")

#define INTERNAL_ssLOG_BENCH_START_1(benchName) \
    std::make_pair(std::string(benchName), std::chrono::high_resolution_clock::now()); \
    do \
    { \
        if(std::string(benchName).empty()) \
        { \
            INTERNAL_ssLOG_LINE_1("Starting benchmark"); \
        } \
        else \
        { \
            INTERNAL_ssLOG_LINE_1("Starting benchmark \"" << benchName << "\""); \
        } \
    } while(0)


#define INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH_0() \
    INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH_1("")

#define INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH_1(benchName) \
    std::make_pair(std::string(benchName), std::chrono::high_resolution_clock::now())

#define INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH_0() \
    INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH_1("")

#define INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH_1(benchName) \
    do \
    { \
        if(std::string(benchName).empty()) \
        { \
            INTERNAL_ssLOG_LINE_1("Starting benchmark"); \
        } \
        else \
        { \
            INTERNAL_ssLOG_LINE_1("Starting benchmark \"" << benchName << "\""); \
        } \
    } while(0)

#define INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH( ... ) \
    INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH, __VA_ARGS__ )

#define INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH( ... ) \
    INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH, __VA_ARGS__ )

#define INTERNAL_ssLOG_BENCH_END_0() \
    static_assert(false, "ssLOG_BENCH_END must accept 1 argument")

#define INTERNAL_ssLOG_BENCH_END_1(startVar) \
    do \
    { \
        double ssLogBenchUs = static_cast<double> \
        ( \
            std::chrono::duration_cast<std::chrono::microseconds> \
            ( \
                std::chrono::high_resolution_clock::now() - startVar.second \
            ).count() \
        ); \
        \
        if(!startVar.first.empty()) \
        { \
            if(ssLogBenchUs > 1000000000) \
                do{ INTERNAL_ssLOG_LINE_1("Benchmark \"" << startVar.first << "\" toke " << ssLogBenchUs / 1000000000.0 << " minutes") } while(0); \
            else if(ssLogBenchUs > 1000000) \
                do{ INTERNAL_ssLOG_LINE_1("Benchmark \"" << startVar.first << "\" toke " << ssLogBenchUs / 1000000.0 << " seconds") } while(0); \
            else if(ssLogBenchUs > 1000) \
                do{ INTERNAL_ssLOG_LINE_1("Benchmark \"" << startVar.first << "\" toke " << ssLogBenchUs / 1000.0 << " milliseconds") } while(0); \
            else \
                do{ INTERNAL_ssLOG_LINE_1("Benchmark \"" << startVar.first << "\" toke " << ssLogBenchUs << " microseconds") } while(0); \
        } \
        else \
        { \
            if(ssLogBenchUs > 1000000000) \
                do{ INTERNAL_ssLOG_LINE_1("Benchmark toke " << ssLogBenchUs / 1000000000.0 << " minutes") } while(0); \
            else if(ssLogBenchUs > 1000000) \
                do{ INTERNAL_ssLOG_LINE_1("Benchmark toke " << ssLogBenchUs / 1000000.0 << " seconds") } while(0); \
            else if(ssLogBenchUs > 1000) \
                do{ INTERNAL_ssLOG_LINE_1("Benchmark toke " << ssLogBenchUs / 1000.0 << " milliseconds") } while(0); \
            else \
                do{ INTERNAL_ssLOG_LINE_1("Benchmark toke " << ssLogBenchUs << " microseconds") } while(0); \
        } \
    } while(0)


// =======================================================================
// Macros for ssLOG_SET_CURRENT_THREAD_TARGET_LEVEL
// =======================================================================

#define ssLOG_SET_CURRENT_THREAD_TARGET_LEVEL(targetLevel) \
    do \
    { \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        INTERNAL_ssLOG_TARGET_LEVEL() = targetLevel; \
    } \
    while(0)

// =======================================================================
// Macros for output level
// =======================================================================

#define ssLOG_LEVEL_DEBUG 5
#define ssLOG_LEVEL_INFO 4
#define ssLOG_LEVEL_WARNING 3
#define ssLOG_LEVEL_ERROR 2
#define ssLOG_LEVEL_FATAL 1

#include <cstdint>
#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
#endif

#if ssLOG_USE_ESCAPE_SEQUENCES
    #define TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES
#endif

#include "./termcolor.hpp"

#if ssLOG_ASCII != 1 && ssLOG_LOG_TO_FILE != 1
    template <typename CharT>
    inline std::basic_ostream<CharT>& ApplyLog(std::basic_ostream<CharT>& stream)
    {
        switch(INTERNAL_ssLOG_CURRENT_LOG_LEVEL())
        {
            case ssLOG_LEVEL_FATAL:
                stream <<   termcolor::colorize << 
                            termcolor::grey << 
                            termcolor::on_red << 
                            "[FATAL]" << 
                            termcolor::reset << " ";
                
                INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = 0;
                return stream;
            
            case ssLOG_LEVEL_ERROR:
                stream <<   termcolor::colorize << 
                            termcolor::grey << 
                            termcolor::on_bright_red << 
                            "[ERROR]" << 
                            termcolor::reset << " ";
                
                INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = 0;
                return stream;
            
            case ssLOG_LEVEL_WARNING:
                stream <<   termcolor::colorize << 
                            termcolor::grey << 
                            termcolor::on_yellow << 
                            "[WARNING]" << 
                            termcolor::reset << " ";
                
                INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = 0;
                return stream;
            
            case ssLOG_LEVEL_INFO:
                stream <<   termcolor::colorize << 
                            termcolor::white << 
                            termcolor::on_grey << 
                            "[INFO]" << 
                            termcolor::reset << " ";
                
                INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = 0;
                return stream;
            
            case ssLOG_LEVEL_DEBUG:
                stream <<   termcolor::colorize << 
                            termcolor::grey << 
                            termcolor::on_green << 
                            "[DEBUG]" << 
                            termcolor::reset << " ";
                
                INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = 0;
                return stream;
            
            default:
                INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = 0;
                return stream;
        }
    }
#else
    template <typename CharT>
    inline std::basic_ostream<CharT>& ApplyLog(std::basic_ostream<CharT>& stream)
    {
        switch(INTERNAL_ssLOG_CURRENT_LOG_LEVEL())
        {
            case ssLOG_LEVEL_FATAL:
                stream << "[FATAL] ";
                INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = 0;
                return stream;
            
            case ssLOG_LEVEL_ERROR:
                stream << "[ERROR] ";
                INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = 0;
                return stream;
            
            case ssLOG_LEVEL_WARNING:
                stream << "[WARNING] ";
                INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = 0;
                return stream;
            
            case ssLOG_LEVEL_INFO:
                stream << "[INFO] ";
                INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = 0;
                return stream;
            
            case ssLOG_LEVEL_DEBUG:
                stream << "[DEBUG] ";
                INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = 0;
                return stream;
            
            default:
                INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = 0;
                return stream;
        }
    }
#endif //ssLOG_ASCII != 1 && ssLOG_LOG_TO_FILE != 1

#define INTERNAL_ssLOG_EXECUTE_COMMAND_0()

#define INTERNAL_ssLOG_EXECUTE_COMMAND_1(command) command

#define INTERNAL_ssLOG_EXECUTE_COMMAND_2(a, command) command


#if ssLOG_LEVEL >= ssLOG_LEVEL_FATAL
    #define ssLOG_FATAL(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_FATAL; \
            ssLOG_LINE(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_CONTENT_FATAL(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_FATAL; \
            ssLOG_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_CONTENT_FATAL(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_FATAL; \
            ssLOG_FUNC_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_ENTRY_FATAL(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_FATAL; \
            ssLOG_FUNC_ENTRY(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_EXIT_FATAL(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_FATAL; \
            ssLOG_FUNC_EXIT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_FATAL(...) \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_FATAL; \
        ssLOG_FUNC(__VA_ARGS__)
    
    #define ssLOG_BENCH_START_FATAL(...) \
        INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__); \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_FATAL; \
        INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH(__VA_ARGS__)
    
    #define ssLOG_BENCH_END_FATAL(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_FATAL; \
            ssLOG_BENCH_END(__VA_ARGS__); \
        } while(0)
#else
    #define ssLOG_FATAL(...) do{}while(0)
    #define ssLOG_CONTENT_FATAL(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_CONTENT_FATAL(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_ENTRY_FATAL(...) do{}while(0)
    #define ssLOG_FUNC_EXIT_FATAL(...) do{}while(0)
    #define ssLOG_FUNC_FATAL(...) do{}while(0)
    #define ssLOG_BENCH_START_FATAL(...) INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__)
    #define ssLOG_BENCH_END_FATAL(...) do{}while(0)
#endif //ssLOG_LEVEL >= ssLOG_LEVEL_FATAL

#if ssLOG_LEVEL >= ssLOG_LEVEL_ERROR
    #define ssLOG_ERROR(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_ERROR; \
            ssLOG_LINE(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_CONTENT_ERROR(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_ERROR; \
            ssLOG_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_CONTENT_ERROR(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_ERROR; \
            ssLOG_FUNC_CONTENT(__VA_ARGS__); \
        } while(0)
    #define ssLOG_FUNC_ENTRY_ERROR(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_ERROR; \
            ssLOG_FUNC_ENTRY(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_EXIT_ERROR(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_ERROR; \
            ssLOG_FUNC_EXIT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_ERROR(...) \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_ERROR; \
        ssLOG_FUNC(__VA_ARGS__)
    
    #define ssLOG_BENCH_START_ERROR(...) \
        INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__); \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_ERROR; \
        INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH(__VA_ARGS__)
    
    #define ssLOG_BENCH_END_ERROR(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_ERROR; \
            ssLOG_BENCH_END(__VA_ARGS__); \
        } while(0)
#else
    #define ssLOG_ERROR(...) do{}while(0)
    #define ssLOG_CONTENT_ERROR(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_CONTENT_ERROR(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_ENTRY_ERROR(...) do{}while(0)
    #define ssLOG_FUNC_EXIT_ERROR(...) do{}while(0)
    #define ssLOG_FUNC_ERROR(...) do{}while(0)
    #define ssLOG_BENCH_START_ERROR(...) INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__)
    #define ssLOG_BENCH_END_ERROR(...) do{}while(0)
#endif //ssLOG_LEVEL >= ssLOG_LEVEL_ERROR

#if ssLOG_LEVEL >= ssLOG_LEVEL_WARNING
    #define ssLOG_WARNING(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_WARNING; \
            ssLOG_LINE(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_CONTENT_WARNING(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_WARNING; \
            ssLOG_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_CONTENT_WARNING(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_WARNING; \
            ssLOG_FUNC_CONTENT(__VA_ARGS__); \
        } while(0)
    #define ssLOG_FUNC_ENTRY_WARNING(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_WARNING; \
            ssLOG_FUNC_ENTRY(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_EXIT_WARNING(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_WARNING; \
            ssLOG_FUNC_EXIT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_WARNING(...) \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_WARNING; \
        ssLOG_FUNC(__VA_ARGS__)
    
    #define ssLOG_BENCH_START_WARNING(...) \
        INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__); \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_WARNING; \
        INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH(__VA_ARGS__)
    
    #define ssLOG_BENCH_END_WARNING(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_WARNING; \
            ssLOG_BENCH_END(__VA_ARGS__); \
        } while(0)
#else
    #define ssLOG_WARNING(...) do{}while(0)
    #define ssLOG_CONTENT_WARNING(...) INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    #define ssLOG_FUNC_CONTENT_WARNING(...) INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    #define ssLOG_FUNC_ENTRY_WARNING(...) do{}while(0)
    #define ssLOG_FUNC_EXIT_WARNING(...) do{}while(0)
    #define ssLOG_FUNC_WARNING(...) do{}while(0)
    #define ssLOG_BENCH_START_WARNING(...) INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__)
    #define ssLOG_BENCH_END_WARNING(...) do{}while(0)
#endif //ssLOG_LEVEL >= ssLOG_LEVEL_WARNING

#if ssLOG_LEVEL >= ssLOG_LEVEL_INFO
    #define ssLOG_INFO(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_INFO; \
            ssLOG_LINE(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_CONTENT_INFO(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_INFO; \
            ssLOG_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_CONTENT_INFO(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_INFO; \
            ssLOG_FUNC_CONTENT(__VA_ARGS__); \
        } while(0)
    #define ssLOG_FUNC_ENTRY_INFO(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_INFO; \
            ssLOG_FUNC_ENTRY(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_EXIT_INFO(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_INFO; \
            ssLOG_FUNC_EXIT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_INFO(...) \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_INFO; \
        ssLOG_FUNC(__VA_ARGS__)
    
    #define ssLOG_BENCH_START_INFO(...) \
        INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__); \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_INFO; \
        INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH(__VA_ARGS__)
    
    #define ssLOG_BENCH_END_INFO(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_INFO; \
            ssLOG_BENCH_END(__VA_ARGS__); \
        } while(0)
#else
    #define ssLOG_INFO(...) do{}while(0)
    #define ssLOG_CONTENT_INFO(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_CONTENT_INFO(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_ENTRY_INFO(...) do{}while(0)
    #define ssLOG_FUNC_EXIT_INFO(...) do{}while(0)
    #define ssLOG_FUNC_INFO(...) do{}while(0)
    #define ssLOG_BENCH_START_INFO(...) INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__)
    #define ssLOG_BENCH_END_INFO(...) do{}while(0)
#endif //ssLOG_LEVEL >= ssLOG_LEVEL_INFO

#if ssLOG_LEVEL >= ssLOG_LEVEL_DEBUG
    #define ssLOG_DEBUG(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_DEBUG; \
            ssLOG_LINE(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_CONTENT_DEBUG(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_DEBUG; \
            ssLOG_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_CONTENT_DEBUG(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_DEBUG; \
            ssLOG_FUNC_CONTENT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_ENTRY_DEBUG(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_DEBUG; \
            ssLOG_FUNC_ENTRY(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_EXIT_DEBUG(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_DEBUG; \
            ssLOG_FUNC_EXIT(__VA_ARGS__); \
        } while(0)
    
    #define ssLOG_FUNC_DEBUG(...) \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_DEBUG; \
        ssLOG_FUNC(__VA_ARGS__)
    
    #define ssLOG_BENCH_START_DEBUG(...) \
        INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__); \
        INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
        INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_DEBUG; \
        INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH(__VA_ARGS__)
    
    #define ssLOG_BENCH_END_DEBUG(...) \
        do{ \
            INTERNAL_ssLOG_CHECK_NEW_THREAD(); \
            INTERNAL_ssLOG_CURRENT_LOG_LEVEL() = ssLOG_LEVEL_DEBUG; \
            ssLOG_BENCH_END(__VA_ARGS__); \
        } while(0)
#else
    #define ssLOG_DEBUG(...) do{}while(0)
    #define ssLOG_CONTENT_DEBUG(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_CONTENT_DEBUG(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_ENTRY_DEBUG(...) do{}while(0)
    #define ssLOG_FUNC_EXIT_DEBUG(...) do{}while(0)
    #define ssLOG_FUNC_DEBUG(...) do{}while(0)
    #define ssLOG_BENCH_START_DEBUG(...) INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__)
    #define ssLOG_BENCH_END_DEBUG(...) do{}while(0)
#endif //ssLOG_LEVEL >= ssLOG_LEVEL_DEBUG


#endif //ssLOG_HPP

