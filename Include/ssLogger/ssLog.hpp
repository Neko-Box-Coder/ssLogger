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
#include <algorithm>

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
#endif //#if ssLOG_THREAD_SAFE_OUTPUT


#if ssLOG_IMMEDIATE_FLUSH
    #define ssLOG_ENDL std::endl
#else
    #define ssLOG_ENDL "\n"
#endif

// =======================================================================
// Macros for ssLOG_BASE and ssLOG_FLUSH
// =======================================================================

#define INTERNAL_UNSAFE_ssLOG_CACHE_OUTPUT_AND_BREAK_IF_CACHING(x) \
    if(InternalUnsafe_ssLogIsCacheOutput()) \
    { \
        std::stringstream localss; \
        localss << x; \
        InternalUnsafe_ssLogAppendCurrentCacheOutput(localss); \
        break; \
    }

//If we are only logging to file
#if ssLOG_LOG_TO_FILE || ssLOG_MODE == ssLOG_MODE_FILE
    #define INTERNAL_UNSAFE_ssLOG_TO_CONSOLE(x) 
    #define INTERNAL_UNSAFE_ssLOG_TO_FILE(x) \
        do \
        { \
            INTERNAL_UNSAFE_ssLOG_CACHE_OUTPUT_AND_BREAK_IF_CACHING(x); \
            std::stringstream localss; \
            localss << x; \
            InternalUnsafe_ssLogToFileWithNewline(localss); \
        } while(0)
    
    #define ssLOG_BASE(x) INTERNAL_UNSAFE_ssLOG_TO_FILE(x)
    #define ssLOG_FLUSH() \
        if(ssLogFileStream.good() && ssLogFileStream.is_open()) \
            ssLogFileStream.flush()

//If we are only logging to console
#elif !ssLOG_LOG_TO_FILE && ssLOG_MODE == ssLOG_MODE_CONSOLE
    #include <iostream>
    
    #define INTERNAL_UNSAFE_ssLOG_TO_CONSOLE(x) \
        do \
        { \
            INTERNAL_UNSAFE_ssLOG_CACHE_OUTPUT_AND_BREAK_IF_CACHING(x); \
            std::cout << x << ssLOG_ENDL; \
        } while(0)
    #define INTERNAL_UNSAFE_ssLOG_TO_FILE(x) 
    
    #define ssLOG_BASE(x) INTERNAL_UNSAFE_ssLOG_TO_CONSOLE(x)
    #define ssLOG_FLUSH() std::cout << std::flush

//If we are logging to both
#else
    #include <iostream>
    
    static_assert(!ssLOG_LOG_TO_FILE, "ssLOG_LOG_TO_FILE should not be set");
    static_assert(  ssLOG_MODE == ssLOG_MODE_CONSOLE_AND_FILE, 
                    "ssLOG_MODE should be ssLOG_MODE_CONSOLE_AND_FILE");
    
    //NOTE: Since these are called together, we only need to cache in the console call
    #define INTERNAL_UNSAFE_ssLOG_TO_CONSOLE(x) \
        do \
        { \
            INTERNAL_UNSAFE_ssLOG_CACHE_OUTPUT_AND_BREAK_IF_CACHING(x); \
            std::cout << x << ssLOG_ENDL; \
        } while(0)
        
    #define INTERNAL_UNSAFE_ssLOG_TO_FILE(x) \
        do \
        { \
            if(InternalUnsafe_ssLogIsCacheOutput()) break; \
            std::stringstream localss; \
            localss << x; \
            InternalUnsafe_ssLogToFileWithNewline(localss); \
        } while(0)
    
    #define ssLOG_BASE(x) \
        do \
        { \
            INTERNAL_UNSAFE_ssLOG_TO_CONSOLE(x); \
            INTERNAL_UNSAFE_ssLOG_TO_FILE(x); \
        } while(0)
    
    #define ssLOG_FLUSH() \
        do \
        { \
            std::cout << std::flush; \
            if(ssLogFileStream.good() && ssLogFileStream.is_open()) \
                ssLogFileStream.flush(); \
        } while(0)
#endif //#if ssLOG_MODE == ssLOG_MODE_CONSOLE_AND_FILE

//NOTE: Reason needs to differentiate this against ssLOG_BASE is because this allows us to log the
//      same thing with and without color or a variation of text for console vs file
#define INTERNAL_UNSAFE_ssLOG_TO_CONSOLE_LOCKED(x) \
    do \
    { \
        INTERNAL_ssLOG_LOCK_OUTPUT(); \
        INTERNAL_UNSAFE_ssLOG_TO_CONSOLE(x); \
        INTERNAL_ssLOG_UNLOCK_OUTPUT(); \
    } while(0)

#define INTERNAL_UNSAFE_ssLOG_TO_FILE_LOCKED(x) \
    do \
    { \
        INTERNAL_ssLOG_LOCK_OUTPUT(); \
        INTERNAL_UNSAFE_ssLOG_TO_FILE(x); \
        INTERNAL_ssLOG_UNLOCK_OUTPUT(); \
    } while(0)

//NOTE: ssLOG_BASE replaces ssLOG_SIMPLE for legacy reasons
#define ssLOG_SIMPLE(x) ssLOG_BASE(x)

#if ssLOG_SHOW_FILE_NAME
    #if __cplusplus >= 201703L
        #define INTERNAL_ssLOG_CONST constexpr const
    #else
        #define INTERNAL_ssLOG_CONST const
    #endif

    #if ssLOG_SHOW_LINE_NUM
        #define INTERNAL_ssLOG_GET_FILE_NAME() \
            []() \
            { \
                INTERNAL_ssLOG_CONST char* constFilename = Internal_ssLogGetFileName(__FILE__); \
                return std::string("[") + constFilename; \
            }()
    #else
        #define INTERNAL_ssLOG_GET_FILE_NAME() \
            []() \
            { \
                INTERNAL_ssLOG_CONST char* constFilename = Internal_ssLogGetFileName(__FILE__); \
                return std::string("[") + constFilename + "] "; \
            }()
    #endif
#else
    #define INTERNAL_ssLOG_GET_FILE_NAME() ""
#endif

#if ssLOG_SHOW_LINE_NUM
    #if ssLOG_SHOW_FILE_NAME
        #define INTERNAL_ssLOG_GET_LINE_NUM() std::string(":") + std::to_string(__LINE__) + "] "
    #else
        #define INTERNAL_ssLOG_GET_LINE_NUM() std::string("[line ") + std::to_string(__LINE__) + "] "
    #endif
#else
    #define INTERNAL_ssLOG_GET_LINE_NUM() ""
#endif

#if ssLOG_PREPEND_LOC == ssLOG_PREPEND_LOC_BEFORE_FUNC_NAME
    #define INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_FUNC() InternalUnsafe_ssLogGetPrepend()
    #define INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_FILE() ""
    #define INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_MESSAGE() ""
#elif ssLOG_PREPEND_LOC == ssLOG_PREPEND_LOC_BEFORE_FILE_NAME
    #define INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_FUNC() ""
    #define INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_FILE() InternalUnsafe_ssLogGetPrepend()
    #define INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_MESSAGE() ""
#elif ssLOG_PREPEND_LOC == ssLOG_PREPEND_LOC_BEFORE_MESSAGE
    #define INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_FUNC() ""
    #define INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_FILE() ""
    #define INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_MESSAGE() InternalUnsafe_ssLogGetPrepend()
#else
    #error Invalid ssLOG_PREPEND_LOC
#endif

//NOTE: Nesting INTERNAL_ssLOG_VA_SELECT doesn't work :/
//#define GET_FUNCTION_NAME( ... ) INTERNAL_ssLOG_VA_SELECT( GET_FUNCTION_NAME, __VA_ARGS__ )

#if ssLOG_SHOW_FUNC_NAME
    #define INTERNAL_ssLOG_GET_FUNCTION_NAME_0() std::string("[") + __func__ + "()] "
    #define INTERNAL_ssLOG_GET_FUNCTION_NAME_1(x) std::string("[") + x + "()] "
    
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

#define INTERNAL_ssLOG_MAP_READ_GUARD() \
    Internal_ssLogMapReadGuard mapGuard = Internal_ssLogMapReadGuard(); \
    (void)mapGuard;

// =======================================================================
// Macros for ssLOG_LINE, ssLOG_FUNC, ssLOG_FUNC_ENTRY, ssLOG_FUNC_EXIT and ssLOG_FUNC_CONTENT
// =======================================================================

#if !ssLOG_CALL_STACK_ONLY
    #define ssLOG_LINE( ... ) do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_LINE, __VA_ARGS__ ) } while(0)
#else
    #define ssLOG_LINE(...) do {} while(false)
#endif

//NOTE: ssLOG_CONTENT replaces ssLOG_FUNC_CONTENT
#define ssLOG_CONTENT( ... ) ssLOG_FUNC_CONTENT( __VA_ARGS__ )


#if ssLOG_SHOW_THREADS
    #define INTERNAL_UNSAFE_ssLOG_PRINT_THREAD_ID() \
        "[Thread " << InternalUnsafe_ssLogGetThreadId() << "] "
#else
    #define INTERNAL_UNSAFE_ssLOG_PRINT_THREAD_ID()
#endif

#if !ssLOG_CALL_STACK
    #define ssLOG_FUNC( ... ) do{}while(0)
    #define ssLOG_FUNC_ENTRY( ... ) do{}while(0)
    #define ssLOG_FUNC_EXIT( ... ) do{}while(0)
    #define ssLOG_FUNC_CONTENT(expr) expr; do{}while(0)

    #define INTERNAL_ssLOG_FUNC_CONTENT_LEVELED(expr, level) expr;
    #define INTERNAL_ssLOG_FUNC_LEVELED_IMPL_1(...)
    #define INTERNAL_ssLOG_FUNC_LEVELED_IMPL_2(...) 
    
    #define INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_0() \
        static_assert(false, "INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL must provide a level")

    #define INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_1(...) 
    #define INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(...) 

    #define INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_0() \
        static_assert(false, "INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL must provide a level")

    #define INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_1(...) 
    #define INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(...) 
#else
    //INTERNAL_ssLOG_LIMIT_EXPR turns any expression into string limiting it to 50 characters
    #define INTERNAL_ssLOG_LIMIT_EXPR(x) \
        (std::string(#x).size() > 50 ? std::string(#x).substr(0, 50) + " ..." : #x)
    
    #define INTERNAL_ssLOG_LIMIT_STR(x) \
        (x.size() > 50 ? x.substr(0, 50) + " ..." : x)

    #define ssLOG_FUNC( ... ) INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC, __VA_ARGS__ )

    #define INERNAL_ssLOG_FUNC_LEVELED( ... ) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_LEVELED_IMPL, __VA_ARGS__ )

    #if !ssLOG_CALL_STACK_ONLY
        #define ssLOG_FUNC_CONTENT(expr) INTERNAL_ssLOG_FUNC_CONTENT_LEVELED(expr, 0)
    #else
        #define ssLOG_FUNC_CONTENT(expr) expr
    #endif

    #define INTERNAL_ssLOG_FUNC_CONTENT_LEVELED(expr, level) \
        std::string ssLogLimitedExpr = INTERNAL_ssLOG_LIMIT_EXPR(expr); \
        Internal_ssLogFuncEntry(std::string("[") + ssLogLimitedExpr + "]", \
                                INTERNAL_ssLOG_GET_FILE_NAME(), \
                                INTERNAL_ssLOG_GET_LINE_NUM(), \
                                level); \
        \
        expr; \
        \
        Internal_ssLogFuncExit( std::string("[") + ssLogLimitedExpr + "]", \
                                INTERNAL_ssLOG_GET_FILE_NAME(), \
                                INTERNAL_ssLOG_GET_LINE_NUM())
    
    #define INTERNAL_ssLOG_TO_STRING(x) (std::stringstream() << x).str()
    
    #define INTERNAL_ssLOG_FUNC_0() INTERNAL_ssLOG_FUNC_LEVELED_IMPL_1(0)
    
    #define INTERNAL_ssLOG_SHORTEN_CUSTOM_FUNCTION_NAME_STR(customFuncName) \
        INTERNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_FUNCTION_NAME_1(INTERNAL_ssLOG_LIMIT_STR(customFuncName)))
    
    #define INTERNAL_ssLOG_FUNC_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_LEVELED_IMPL_2(customFuncName, 0)

    #define INTERNAL_ssLOG_FUNC_LEVELED_IMPL_0() \
        static_assert(false, "INTERNAL_ssLOG_FUNC_LEVELED_IMPL must provide a level")

    #define INTERNAL_ssLOG_FUNC_LEVELED_IMPL_1(level) \
        Internal_ssLogFunctionScope ssLogScopeObj = \
        Internal_ssLogFunctionScope(INTERNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_FUNCTION_NAME_0()), \
                                    INTERNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_FILE_NAME()), \
                                    INTERNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_LINE_NUM()), \
                                    level)
    
    #define INTERNAL_ssLOG_FUNC_LEVELED_IMPL_2(customFuncName, level) \
        Internal_ssLogFunctionScope ssLogScopeObj = \
        Internal_ssLogFunctionScope(INTERNAL_ssLOG_SHORTEN_CUSTOM_FUNCTION_NAME_STR(std::string(customFuncName)), \
                                    INTERNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_FILE_NAME()), \
                                    INTERNAL_ssLOG_TO_STRING(INTERNAL_ssLOG_GET_LINE_NUM()), \
                                    level)

    #define ssLOG_FUNC_ENTRY( ... ) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_ENTRY, __VA_ARGS__ ) } while(0)
    
    #define ssLOG_FUNC_EXIT( ... ) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_EXIT, __VA_ARGS__ ) } while(0)

    #define INTERNAL_ssLOG_FUNC_ENTRY_0() \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(__func__, 0)
    
    #define INTERNAL_ssLOG_FUNC_EXIT_0() \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(__func__, 0)

    #define INTERNAL_ssLOG_FUNC_ENTRY_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(customFuncName, 0)

    #define INTERNAL_ssLOG_FUNC_EXIT_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(customFuncName, 0)

    #define INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_0() \
        static_assert(false, "INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL must provide a level")

    #define INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_1(level) \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(__func__, level)

    #define INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(customFuncName, level) \
        Internal_ssLogFuncEntry(INTERNAL_ssLOG_SHORTEN_CUSTOM_FUNCTION_NAME_STR(std::string(customFuncName)), \
                                INTERNAL_ssLOG_GET_FILE_NAME(), \
                                INTERNAL_ssLOG_GET_LINE_NUM(), \
                                level);

    #define INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_0() \
        static_assert(false, "INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL must provide a level")

    #define INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_1(level) \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(__func__, level)

    #define INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(customFuncName, level) \
        Internal_ssLogFuncExit( INTERNAL_ssLOG_SHORTEN_CUSTOM_FUNCTION_NAME_STR(std::string(customFuncName)), \
                                INTERNAL_ssLOG_GET_FILE_NAME(), \
                                INTERNAL_ssLOG_GET_LINE_NUM() \
                                /* NOTE: It should use the level from Entry */);
#endif //#if !ssLOG_CALL_STACK

#define INTERNAL_ssLOG_LINE_0() \
    Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                        INTERNAL_ssLOG_GET_FILE_NAME(), \
                        INTERNAL_ssLOG_GET_LINE_NUM(), "", \
                        0);

#define INTERNAL_ssLOG_LINE_1(debugText) \
    std::stringstream localssLogString; \
    localssLogString << debugText; \
    Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                        INTERNAL_ssLOG_GET_FILE_NAME(), \
                        INTERNAL_ssLOG_GET_LINE_NUM(), \
                        localssLogString.str(), \
                        0);

// =======================================================================
// Macros for ssLOG_PREPEND
// =======================================================================

#if !ssLOG_CALL_STACK_ONLY
    #define ssLOG_PREPEND(x) \
        do \
        { \
            std::stringstream localssLogString; \
            localssLogString << x; \
            Internal_ssLogSetPrepend(localssLogString); \
        } while(0)
    
    #define ssLOG_PREPEND_RESET() Internal_ssLogResetPrepend()
#else
    #define ssLOG_PREPEND(x) do{} while(0)
    #define ssLOG_PREPEND_RESET() do{} while(0)
#endif

// =======================================================================
// Macros for ssLOG_ENABLE_LOG_TO_FILE, ssLOG_IS_LOG_TO_FILE_ENABLE, ssLOG_SET_LOG_FILENAME,
// ssLOG_GET_LOG_FILENAME
// =======================================================================

#if ssLOG_LOG_TO_FILE || \
    ssLOG_MODE == ssLOG_MODE_FILE || \
    ssLOG_MODE == ssLOG_MODE_CONSOLE_AND_FILE

    #define ssLOG_ENABLE_LOG_TO_FILE(enable) Internal_ssLogEnableLogToFile(enable)
    #define ssLOG_IS_LOG_TO_FILE_ENABLED() Internal_ssLogIsEnabledLogToFile()
    #define ssLOG_SET_LOG_FILENAME(filename) Internal_ssLogSetLogFilename(filename)
    #define ssLOG_GET_LOG_FILENAME() Internal_ssLogGetLogFilename()
#else
    #define ssLOG_ENABLE_LOG_TO_FILE(enable) 
    #define ssLOG_IS_LOG_TO_FILE_ENABLED() false
    #define ssLOG_SET_LOG_FILENAME(filename) 
    #define ssLOG_GET_LOG_FILENAME() ""
#endif

// =======================================================================
// Macros for ssLOG_ENABLE_CACHE_OUTPUT, ssLOG_DISABLE_CACHE_OUTPUT, 
// ssLOG_ENABLE_CACHE_OUTPUT_FOR_CURRENT_THREAD, ssLOG_DISABLE_CACHE_OUTPUT_FOR_CURRENT_THREAD, 
// ssLOG_ENABLE_CACHE_OUTPUT_FOR_NEW_THREADS, ssLOG_DISABLE_CACHE_OUTPUT_FOR_NEW_THREADS, 
// ssLOG_CACHE_OUTPUT_FOR_SCOPE, ssLOG_OUTPUT_ALL_CACHE and ssLOG_OUTPUT_ALL_CACHE_GROUPED
// =======================================================================

#define ssLOG_ENABLE_CACHE_OUTPUT() Internal_ssLogSetAllCacheOutput(true)

#define ssLOG_DISABLE_CACHE_OUTPUT() Internal_ssLogSetAllCacheOutput(false)

#define ssLOG_ENABLE_CACHE_OUTPUT_FOR_CURRENT_THREAD() Internal_ssLogSetCacheOutput(true)

#define ssLOG_DISABLE_CACHE_OUTPUT_FOR_CURRENT_THREAD() Internal_ssLogSetCacheOutput(false)

#define ssLOG_IS_CACHE_OUTPUT_FOR_CURRENT_THREAD() Internal_ssLogIsCacheOutput()

#define ssLOG_ENABLE_CACHE_OUTPUT_FOR_NEW_THREADS() ssLogNewThreadCacheByDefault.store(true)

#define ssLOG_DISABLE_CACHE_OUTPUT_FOR_NEW_THREADS() ssLogNewThreadCacheByDefault.store(false)

#define ssLOG_CACHE_OUTPUT_IN_SCOPE() \
    Internal_ssLogCacheScope \
    INTERNAL_ssLOG_SELECT(ssLogCacheScopeObj, __LINE__) = Internal_ssLogCacheScope()

#define ssLOG_OUTPUT_ALL_CACHE() Internal_ssLogOutputAllCache()

#define ssLOG_OUTPUT_ALL_CACHE_GROUPED() Internal_ssLogOutputAllCacheGrouped()

#define ssLOG_RESET_ALL_THREAD_INFO() Internal_ssLogResetAllThreadInfo()

// =======================================================================
// Macros for ssLOG_BENCH_START and ssLOG_BENCH_END
// =======================================================================

#if !ssLOG_CALL_STACK_ONLY
    #define ssLOG_BENCH_START( ... ) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_START, __VA_ARGS__ )
    #define ssLOG_BENCH_END( ... ) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_END, __VA_ARGS__ ); } while(0)
#else
    #define ssLOG_BENCH_START( ... ) INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__)
    #define ssLOG_BENCH_END( ... ) do{} while(false)
#endif

#define INTERNAL_ssLOG_BENCH_START_0() INTERNAL_ssLOG_BENCH_START_1("")

#define INTERNAL_ssLOG_BENCH_START_1(benchName) \
    INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(benchName); \
    INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH_LEVELED(benchName, 0)

#define INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(benchName) \
    std::make_pair(std::string(benchName), std::chrono::high_resolution_clock::now())

#define INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH_LEVELED(benchName, level) \
    do \
    { \
        std::string benchNameStr = benchName; \
        std::stringstream localssLogString; \
        if(benchNameStr.empty()) \
            localssLogString << "Starting benchmark"; \
        else \
            localssLogString << "Starting benchmark \"" << benchNameStr << "\""; \
        \
        Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), \
                            localssLogString.str(), \
                            level); \
    } while(0)

#define INTERNAL_ssLOG_BENCH_END_0() \
    static_assert(false, "ssLOG_BENCH_END must accept 1 argument")

#define INTERNAL_ssLOG_BENCH_END_1(startVar) \
    Interna_ssLogBenchEnd(  startVar, \
                            INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), \
                            0)

// =======================================================================
// Macros for ssLOG_SET_CURRENT_THREAD_TARGET_LEVEL & ssLOG_GET_CURRENT_THREAD_TARGET_LEVEL
// =======================================================================

#define ssLOG_SET_CURRENT_THREAD_TARGET_LEVEL(targetLevel) \
    Internal_ssLogSetCurrentThreadTargetLevel(targetLevel)

#define ssLOG_GET_CURRENT_THREAD_TARGET_LEVEL() Internal_ssLogGetCurrentThreadTargetLevel()


// =======================================================================
// Functions/Classes Implementations
// =======================================================================

#if ssLOG_LOG_TO_FILE || \
    ssLOG_MODE == ssLOG_MODE_FILE || \
    ssLOG_MODE == ssLOG_MODE_CONSOLE_AND_FILE
    
    #include <fstream>
    #include <ctime>
    ssLOG_API extern std::ofstream ssLogFileStream;
    ssLOG_API extern bool ssLogEnableLogToFile;
    ssLOG_API extern std::string ssLogOutputFileName;
    ssLOG_API extern bool ssLogReopenLogFile;
    
    inline void InternalUnsafe_ssLogToFileWithNewline(const std::stringstream& localss)
    {
        if(!ssLogEnableLogToFile || !ssLogFileStream.good() || !ssLogFileStream.is_open())
            return;
        if(localss.rdbuf()->in_avail())
            ssLogFileStream << localss.rdbuf() << ssLOG_ENDL;
        else
            ssLogFileStream << ssLOG_ENDL;
    }
#endif

#if ssLOG_SHOW_FILE_NAME
    #if __cplusplus >= 202002L
        consteval
    #elif __cplusplus >= 201703L
        constexpr
    #else
        inline
    #endif
    const char* Internal_ssLogGetFileName(const char* path)
    {
        const char* result = path;
        while(*path != '\0')
        {
            if(*path == '/' || *path == '\\')
                result = ++path;
            else
                ++path;
        }
        
        return result;
    };
#endif

#include "./ssLogThreadInfo.hpp"

ssLOG_API extern std::unordered_map<std::thread::id, ssLogThreadInfo> ssLogInfoMap;
ssLOG_API extern int ssLogNewThreadID;
ssLOG_API extern std::mutex ssLogMapWriteMutex;
ssLOG_API extern std::atomic<bool> ssLogNewThreadCacheByDefault;
ssLOG_API extern std::atomic<int> ssLogReadCount;

class Internal_ssLogMapReadGuard
{
    public:
        inline Internal_ssLogMapReadGuard()
        {
            ssLogMapWriteMutex.lock();
            ssLogReadCount++;
            ssLogMapWriteMutex.unlock();
        }

        inline ~Internal_ssLogMapReadGuard()
        {
            ssLogReadCount--;
        }
};

inline void Internal_ssLogPreActionChecks()
{
    //Check if we need to open a new text file
    #if ssLOG_LOG_TO_FILE || \
        ssLOG_MODE == ssLOG_MODE_FILE || \
        ssLOG_MODE == ssLOG_MODE_CONSOLE_AND_FILE
    {
        std::unique_lock<std::mutex> lk(ssLogMapWriteMutex);
        while(ssLogReadCount > 0) {}    //Wait until all reads are done
        
        if((ssLogEnableLogToFile && ssLogReopenLogFile) || !ssLogFileStream.good())
        {
            if(ssLogFileStream.is_open())
                ssLogFileStream.close();
            
            ssLogReopenLogFile = false;
            if(ssLogOutputFileName.empty())
            {
                time_t rawTime;
                struct tm* timeinfo;
                char filenameBuffer [80];
                time(&rawTime);
                timeinfo = localtime(&rawTime);
                strftime(filenameBuffer, 80, "%a %b %d %H_%M_%S %Y", timeinfo);
                ssLogOutputFileName = std::string(filenameBuffer)+"_log.txt";
            }
            
            ssLogFileStream.open(ssLogOutputFileName, std::ofstream::out);
        }
    }
    #endif
    
    //Check if this is a new thread, if so add to entry
    {
        bool needsNewEntry = false;
        //Reading map
        {
            INTERNAL_ssLOG_MAP_READ_GUARD();
            needsNewEntry = ssLogInfoMap.find(std::this_thread::get_id()) == ssLogInfoMap.end();
        }
        
        //Adding map entry
        if(needsNewEntry)
        {
            std::unique_lock<std::mutex> lk(ssLogMapWriteMutex);
            while(ssLogReadCount > 0) {}    //Wait until all reads are done
            
            ssLogInfoMap[std::this_thread::get_id()].ID = ssLogNewThreadID++;
            ssLogInfoMap[std::this_thread::get_id()].CacheOutput = 
                ssLogNewThreadCacheByDefault.load();
        }
    }
}

inline int InternalUnsafe_ssLogGetThreadId()
{
    return ssLogInfoMap.at(std::this_thread::get_id()).ID;
}

inline std::string InternalUnsafe_ssLogGetPrepend()
{
    return ssLogInfoMap.at(std::this_thread::get_id()).CurrentPrepend;
}

inline void Internal_ssLogSetPrepend(std::stringstream& ss)
{
    Internal_ssLogPreActionChecks();
    //Accessing map entry
    INTERNAL_ssLOG_MAP_READ_GUARD();
    ssLogInfoMap.at(std::this_thread::get_id()).CurrentPrepend = ss.str();
}

inline void Internal_ssLogResetPrepend()
{
    Internal_ssLogPreActionChecks();
    //Accessing map entry
    INTERNAL_ssLOG_MAP_READ_GUARD();
    ssLogInfoMap.at(std::this_thread::get_id()).CurrentPrepend.clear();
}

inline void InternalUnsafe_ssLogIncrementTabSpace()
{
    ssLogInfoMap.at(std::this_thread::get_id()).TabSpace++;
}

inline void InternalUnsafe_ssLogDecrementTabSpace()
{
    ssLogInfoMap.at(std::this_thread::get_id()).TabSpace--;
}

inline int InternalUnsafe_ssLogGetTabSpace()
{
    return ssLogInfoMap.at(std::this_thread::get_id()).TabSpace;
}

inline std::stack<std::string>& InternalUnsafe_ssLogGetFuncNameStack()
{
    return ssLogInfoMap.at(std::this_thread::get_id()).FuncNameStack;
}

inline std::stack<int>& InternalUnsafe_ssLogGetLogLevelStack()
{
    return ssLogInfoMap.at(std::this_thread::get_id()).LogLevelStack;
}

inline void Internal_ssLogSetCacheOutput(bool cache)
{
    Internal_ssLogPreActionChecks();
    //Accessing map entry
    INTERNAL_ssLOG_MAP_READ_GUARD();
    ssLogInfoMap.at(std::this_thread::get_id()).CacheOutput = cache;
}

inline bool InternalUnsafe_ssLogIsCacheOutput()
{
    return ssLogInfoMap.at(std::this_thread::get_id()).CacheOutput;
}

inline bool Internal_ssLogIsCacheOutput()
{
    Internal_ssLogPreActionChecks();
    //Accessing map entry
    INTERNAL_ssLOG_MAP_READ_GUARD();
    return InternalUnsafe_ssLogIsCacheOutput();
}

inline void InternalUnsafe_ssLogAppendCurrentCacheOutput(const std::stringstream& localss)
{
    ssLogInfoMap.at(std::this_thread::get_id())
                .CurrentCachedOutput.push_back(std::make_pair(  std::chrono::system_clock::now(), 
                                                                localss.str()));
}

inline int InternalUnsafe_ssLogGetTargetLogLevel()
{
    return ssLogInfoMap.at(std::this_thread::get_id()).ssTargetLogLevel;
}

inline std::string InternalUnsafe_ssLogGetThreadVSpace()
{
    return std::string().append(InternalUnsafe_ssLogGetThreadId() * ssLOG_THREAD_VSPACE, ' ');
}

#if ssLOG_LOG_TO_FILE || \
    ssLOG_MODE == ssLOG_MODE_FILE || \
    ssLOG_MODE == ssLOG_MODE_CONSOLE_AND_FILE
    
    inline void Internal_ssLogEnableLogToFile(bool enable)
    {
        std::unique_lock<std::mutex> lk(ssLogMapWriteMutex);
        while(ssLogReadCount > 0) {}    //Wait until all reads are done
        ssLogEnableLogToFile = enable;
    }
    
    inline bool Internal_ssLogIsEnabledLogToFile()
    {
        std::unique_lock<std::mutex> lk(ssLogMapWriteMutex);
        while(ssLogReadCount > 0) {}    //Wait until all reads are done
        return ssLogEnableLogToFile;
    }
    
    inline void Internal_ssLogSetLogFilename(const std::string& filename)
    {
        std::unique_lock<std::mutex> lk(ssLogMapWriteMutex);
        while(ssLogReadCount > 0) {}    //Wait until all reads are done
        ssLogOutputFileName = filename;
        ssLogReopenLogFile = true;
    }
    
    inline std::string Internal_ssLogGetLogFilename()
    {
        std::unique_lock<std::mutex> lk(ssLogMapWriteMutex);
        while(ssLogReadCount > 0) {}    //Wait until all reads are done
        return ssLogOutputFileName;
    }
#endif

class Internal_ssLogLevelApplier
{
    public:
        int Level;
        inline Internal_ssLogLevelApplier(int level) : Level(level)
        {}
        
        inline Internal_ssLogLevelApplier() : Level(0)
        {}
        
        template <typename CharT>
        friend std::basic_ostream<CharT>& operator<<(   std::basic_ostream<CharT>&, 
                                                        Internal_ssLogLevelApplier&);
};

//NOTE: Used only by color
inline std::string Internal_ssLogLevelNoColor(int level)
{
    switch(level)
    {
        case ssLOG_LEVEL_FATAL:
            return "[FATAL] ";
        case ssLOG_LEVEL_ERROR:
            return "[ERROR] ";
        case ssLOG_LEVEL_WARNING:
            return "[WARNING] ";
        case ssLOG_LEVEL_INFO:
            return "[INFO] ";
        case ssLOG_LEVEL_DEBUG:
            return "[DEBUG] ";
        default:
            return "";
    }
}

#if !ssLOG_CALL_STACK
    inline void Internal_ssLogLine( std::string funcName, 
                                    std::string fileName, 
                                    std::string lineNum,
                                    std::string message,
                                    int level)
    {
        Internal_ssLogPreActionChecks();
        
        //Accessing map entry
        {
            INTERNAL_ssLOG_MAP_READ_GUARD();
            if(InternalUnsafe_ssLogGetTargetLogLevel() >= level)
            {
                Internal_ssLogLevelApplier levelApplier = Internal_ssLogLevelApplier(level);
                (void)levelApplier;
                INTERNAL_UNSAFE_ssLOG_TO_CONSOLE_LOCKED
                (
                    InternalUnsafe_ssLogGetThreadVSpace() <<
                    INTERNAL_UNSAFE_ssLOG_PRINT_THREAD_ID() <<
                    INTERNAL_ssLOG_GET_DATE_TIME() <<
                    levelApplier <<
                    INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_FUNC() <<
                    funcName <<
                    INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_FILE() <<
                    fileName <<
                    lineNum << 
                    INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_MESSAGE() <<
                    message
                );
                
                INTERNAL_UNSAFE_ssLOG_TO_FILE_LOCKED
                (
                    InternalUnsafe_ssLogGetThreadVSpace() <<
                    INTERNAL_UNSAFE_ssLOG_PRINT_THREAD_ID() <<
                    INTERNAL_ssLOG_GET_DATE_TIME() <<
                    Internal_ssLogLevelNoColor(level) <<
                    INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_FUNC() <<
                    funcName <<
                    INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_FILE() <<
                    fileName <<
                    lineNum << 
                    INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_MESSAGE() <<
                    message
                );
            }
        }
    }
#else //#if !ssLOG_CALL_STACK
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

    inline void InternalUnsafe_ssLogEmptyLine()
    {
        INTERNAL_UNSAFE_ssLOG_TO_CONSOLE_LOCKED
        (
            InternalUnsafe_ssLogGetThreadVSpace() <<
            INTERNAL_UNSAFE_ssLOG_PRINT_THREAD_ID() << 
            INTERNAL_ssLOG_GET_DATE_TIME() << 
            Internal_ssLog_TabAdder(InternalUnsafe_ssLogGetTabSpace())
        );
        
        INTERNAL_UNSAFE_ssLOG_TO_FILE_LOCKED
        (
            InternalUnsafe_ssLogGetThreadVSpace() <<
            INTERNAL_UNSAFE_ssLOG_PRINT_THREAD_ID() << 
            INTERNAL_ssLOG_GET_DATE_TIME() << 
            Internal_ssLog_TabAdder(InternalUnsafe_ssLogGetTabSpace())
        );
    }

    inline void Internal_ssLogLine( std::string funcName, 
                                    std::string fileName, 
                                    std::string lineNum,
                                    std::string message,
                                    int level)
    {
        Internal_ssLogPreActionChecks();
        
        //Accessing map entry
        {
            INTERNAL_ssLOG_MAP_READ_GUARD();
            if(InternalUnsafe_ssLogGetTargetLogLevel() >= level)
            {
                Internal_ssLogLevelApplier levelApplier = Internal_ssLogLevelApplier(level);
                (void)levelApplier;
                INTERNAL_UNSAFE_ssLOG_TO_CONSOLE_LOCKED
                (
                    InternalUnsafe_ssLogGetThreadVSpace() <<
                    INTERNAL_UNSAFE_ssLOG_PRINT_THREAD_ID() <<
                    INTERNAL_ssLOG_GET_DATE_TIME() <<
                    Internal_ssLog_TabAdder(InternalUnsafe_ssLogGetTabSpace(), true) <<
                    levelApplier <<
                    INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_FUNC() <<
                    funcName <<
                    INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_FILE() <<
                    fileName <<
                    lineNum << 
                    INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_MESSAGE() <<
                    message
                );
                
                INTERNAL_UNSAFE_ssLOG_TO_FILE_LOCKED
                (
                    InternalUnsafe_ssLogGetThreadVSpace() <<
                    INTERNAL_UNSAFE_ssLOG_PRINT_THREAD_ID() <<
                    INTERNAL_ssLOG_GET_DATE_TIME() <<
                    Internal_ssLog_TabAdder(InternalUnsafe_ssLogGetTabSpace(), true) <<
                    Internal_ssLogLevelNoColor(level) <<
                    INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_FUNC() <<
                    funcName <<
                    INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_FILE() <<
                    fileName <<
                    lineNum << 
                    INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_MESSAGE() <<
                    message
                );
            }
        }
    }

    inline void InternalUnsafe_ssLogFuncImpl(   std::string fileName, 
                                                std::string lineNum, 
                                                int level, 
                                                std::string prependMsg)
    {
        Internal_ssLogLevelApplier levelApplier = Internal_ssLogLevelApplier(level);
        (void)levelApplier;
        INTERNAL_UNSAFE_ssLOG_TO_CONSOLE_LOCKED
        (
            InternalUnsafe_ssLogGetThreadVSpace() <<
            INTERNAL_UNSAFE_ssLOG_PRINT_THREAD_ID() <<
            INTERNAL_ssLOG_GET_DATE_TIME() <<
            Internal_ssLog_TabAdder(InternalUnsafe_ssLogGetTabSpace(), true) <<
            levelApplier <<
            INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_FUNC() <<
            prependMsg <<
            INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_FILE() <<
            fileName <<
            lineNum <<
            INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_MESSAGE()
        );
        
        INTERNAL_UNSAFE_ssLOG_TO_FILE_LOCKED
        (
            InternalUnsafe_ssLogGetThreadVSpace() <<
            INTERNAL_UNSAFE_ssLOG_PRINT_THREAD_ID() <<
            INTERNAL_ssLOG_GET_DATE_TIME() <<
            Internal_ssLog_TabAdder(InternalUnsafe_ssLogGetTabSpace(), true) <<
            Internal_ssLogLevelNoColor(level) <<
            INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_FUNC() <<
            prependMsg <<
            INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_FILE() <<
            fileName <<
            lineNum <<
            INTERNAL_UNSAFE_ssLOG_GET_PREPEND_BEFORE_MESSAGE()
        );
    }
    
    inline void Internal_ssLogFuncEntry(std::string expr,
                                        std::string fileName, 
                                        std::string lineNum,
                                        int level)
    {
        Internal_ssLogPreActionChecks();
        
        //Accessing map entry
        INTERNAL_ssLOG_MAP_READ_GUARD();
        
        InternalUnsafe_ssLogGetLogLevelStack().push(level);
        if(InternalUnsafe_ssLogGetTargetLogLevel() >= level)
        { 
            InternalUnsafe_ssLogEmptyLine();
            InternalUnsafe_ssLogFuncImpl
            (
                fileName, 
                lineNum, 
                level, 
                INTERNAL_ssLOG_GET_CONTENT_NAME(expr) + std::string("BEGINS ")
            );
        }

        InternalUnsafe_ssLogGetFuncNameStack().push(expr);
        InternalUnsafe_ssLogIncrementTabSpace();
    }

    inline void Internal_ssLogFuncExit( std::string expr,
                                        std::string fileName, 
                                        std::string lineNum)
    {
        Internal_ssLogPreActionChecks();
        
        //Accessing map entry
        INTERNAL_ssLOG_MAP_READ_GUARD();
        if( InternalUnsafe_ssLogGetFuncNameStack().empty() || 
            InternalUnsafe_ssLogGetFuncNameStack().top() != expr) 
        { 
            INTERNAL_UNSAFE_ssLOG_TO_CONSOLE_LOCKED
            (
                "ssLOG_FUNC_EXIT is missing somewhere. " << 
                expr << " is expected but" << 
                InternalUnsafe_ssLogGetFuncNameStack().top() << 
                " is found instead."
            );
            
            INTERNAL_UNSAFE_ssLOG_TO_FILE_LOCKED
            (
                "ssLOG_FUNC_EXIT is missing somewhere. " << 
                expr << " is expected but" << 
                InternalUnsafe_ssLogGetFuncNameStack().top() << 
                " is found instead."
            );
            
            ssLogReadCount--;
            return;
        }
        
        int currentLogLevel = InternalUnsafe_ssLogGetLogLevelStack().top();
        InternalUnsafe_ssLogGetLogLevelStack().pop();
        InternalUnsafe_ssLogGetFuncNameStack().pop();
        InternalUnsafe_ssLogDecrementTabSpace();
        if(InternalUnsafe_ssLogGetTargetLogLevel() >= currentLogLevel)
        {
            InternalUnsafe_ssLogFuncImpl
            (
                fileName, 
                lineNum, 
                currentLogLevel,
                INTERNAL_ssLOG_GET_CONTENT_NAME(expr) + std::string("EXITS ")
            );
            InternalUnsafe_ssLogEmptyLine();
        }
    }

    class Internal_ssLogFunctionScope
    {
        private:
            std::string FuncName;
            std::string FileName;
        
        public:
            inline Internal_ssLogFunctionScope( std::string funcName, 
                                                std::string fileName, 
                                                std::string lineNum,
                                                int level)
            {
                Internal_ssLogFuncEntry(funcName, fileName, lineNum, level);
                FuncName = funcName;
                FileName = fileName;
            }

            inline ~Internal_ssLogFunctionScope()
            {
                Internal_ssLogFuncExit( FuncName, 
                                        FileName, 
                                        #if ssLOG_SHOW_LINE_NUM
                                            FileName.empty() ? "" : "] ");
                                        #else
                                            "");
                                        #endif
            }
    };
#endif //#if ssLOG_CALL_STACK

inline void Internal_ssLogSetAllCacheOutput(bool cache)
{
    Internal_ssLogPreActionChecks();
    //Editing all map entries
    {
        std::unique_lock<std::mutex> lk(ssLogMapWriteMutex);
        while(ssLogReadCount > 0) {}    //Wait until all reads are done
        for(auto it = ssLogInfoMap.begin(); it != ssLogInfoMap.end(); ++it)
            it->second.CacheOutput = cache;
    }
}

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

inline void Internal_ssLogOutputAllCacheGrouped()
{
    Internal_ssLogPreActionChecks();
    //Reading all map entries
    {
        std::unique_lock<std::mutex> lk(ssLogMapWriteMutex);
        while(ssLogReadCount > 0) {}    //Wait until all reads are done
        
        INTERNAL_ssLOG_LOCK_OUTPUT();
        
        for(auto it = ssLogInfoMap.begin(); it != ssLogInfoMap.end(); ++it)
        {
            if(it->second.CurrentCachedOutput.empty())
                continue;

            for(int i = 0; i < (int)it->second.CurrentCachedOutput.size(); i++)
                ssLOG_BASE(it->second.CurrentCachedOutput[i].second);

            it->second.CurrentCachedOutput.clear();
        }
        
        ssLOG_FLUSH();
        
        INTERNAL_ssLOG_UNLOCK_OUTPUT();
    }
}

inline void Internal_ssLogOutputAllCache()
{
    Internal_ssLogPreActionChecks();
    //Reading all map entries
    {
        std::unique_lock<std::mutex> lk(ssLogMapWriteMutex);
        while(ssLogReadCount > 0) {}    //Wait until all reads are done
        
        std::vector<std::pair<std::chrono::system_clock::time_point, std::string>> allOutput;
        
        for(auto it = ssLogInfoMap.begin(); it != ssLogInfoMap.end(); ++it)
        {
            if(!it->second.CurrentCachedOutput.empty())
            {
                allOutput.insert(allOutput.end(), 
                               it->second.CurrentCachedOutput.begin(),
                               it->second.CurrentCachedOutput.end());
                it->second.CurrentCachedOutput.clear();
            }
        }

        std::sort(  allOutput.begin(), 
                    allOutput.end(),
                    []( const std::pair<std::chrono::system_clock::time_point, std::string>& a, 
                        const std::pair<std::chrono::system_clock::time_point, std::string>& b) 
                    { 
                        return a.first < b.first; 
                    });

        INTERNAL_ssLOG_LOCK_OUTPUT();
        
        std::stringstream ss;
        for(const auto& output : allOutput)
            ss << output.second << "\n";
            
        ssLOG_BASE(ss.str());
        ssLOG_FLUSH();
        
        INTERNAL_ssLOG_UNLOCK_OUTPUT();
    }
}

inline void Internal_ssLogResetAllThreadInfo()
{
    std::unique_lock<std::mutex> lk(ssLogMapWriteMutex);
    while(ssLogReadCount > 0) {}    //Wait until all reads are done
    
    //Remove only threads that have no remaining information
    int maxThreadID = 0;
    for(auto it = ssLogInfoMap.begin(); it != ssLogInfoMap.end();)
    {
        const ssLogThreadInfo& info = it->second;
        bool hasInfo =  !info.CurrentCachedOutput.empty() ||
                        !info.FuncNameStack.empty() ||
                        !info.LogLevelStack.empty() ||
                        info.TabSpace != 0;
                       
        if(!hasInfo)
            it = ssLogInfoMap.erase(it);
        else
        {
            maxThreadID = maxThreadID > info.ID ? maxThreadID : info.ID;
            ++it;
        }
    }
    
    ssLogNewThreadID = maxThreadID + 1;    
}

using Internal_ssLogHighResClock = std::chrono::time_point<std::chrono::high_resolution_clock>;
inline void Interna_ssLogBenchEnd(  const std::pair<std::string, 
                                                    Internal_ssLogHighResClock>& startVar,
                                    std::string funcName, 
                                    std::string fileName, 
                                    std::string lineNum,
                                    int level)
{
    double ssLogBenchUs = static_cast<double>
    (
        std::chrono::duration_cast<std::chrono::microseconds>
        (
            std::chrono::high_resolution_clock::now() - startVar.second
        ).count()
    );

    std::string benchmarkName = startVar.first.empty() ? "" :   std::string("\"") + 
                                                                startVar.first + "\" ";

    if(ssLogBenchUs > 1000000000)
    {
        Internal_ssLogLine( funcName, fileName, lineNum, 
                            std::string("Benchmark ") + benchmarkName + "took " +
                            std::to_string(ssLogBenchUs / 1000000000.0) + " minutes", level);
    }
    else if(ssLogBenchUs > 1000000)
    {
        Internal_ssLogLine( funcName, fileName, lineNum, 
                            std::string("Benchmark ") + benchmarkName + "took " +
                            std::to_string(ssLogBenchUs / 1000000.0) + " seconds", level);
    }
    else if(ssLogBenchUs > 1000)
    {
        Internal_ssLogLine( funcName, fileName, lineNum, 
                            std::string("Benchmark ") + benchmarkName + "took " +
                            std::to_string(ssLogBenchUs / 1000.0) + " milliseconds", level);
    }
    else
    {
        Internal_ssLogLine( funcName, fileName, lineNum, 
                            std::string("Benchmark ") + benchmarkName + "took " +
                            std::to_string(ssLogBenchUs) + " microseconds", level);
    }
}

inline void Internal_ssLogSetCurrentThreadTargetLevel(int targetLevel)
{
    Internal_ssLogPreActionChecks();
    //Reading map
    INTERNAL_ssLOG_MAP_READ_GUARD();
    ssLogInfoMap.at(std::this_thread::get_id()).ssTargetLogLevel = targetLevel;
}

inline int Internal_ssLogGetCurrentThreadTargetLevel()
{
    Internal_ssLogPreActionChecks();
    //Reading map
    INTERNAL_ssLOG_MAP_READ_GUARD();
    return InternalUnsafe_ssLogGetTargetLogLevel();
}

#include "ssLogLevels.hpp"

#endif //#ifndef ssLOG_HPP

