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

    inline void Internal_ssLogBase(const std::stringstream& localss)
    {
        if(!ssLogFileStream.good())
            return;

        if(!ssLogFileStream.is_open())
        {
            time_t ssRawtime;
            struct tm* ssTimeinfo;
            char ssBuffer [80];
            time(&ssRawtime);
            ssTimeinfo = localtime(&ssRawtime);
            strftime(ssBuffer, 80, "%a %b %d %H_%M_%S %Y", ssTimeinfo);
            std::string nowString = std::string(ssBuffer)+"_log.txt";
            ssLogFileStream.open(nowString, std::ofstream::out);

            if(!ssLogFileStream.good())
                return;
        }
        ssLogFileStream << localss.rdbuf() << std::endl;
    }

    #define ssLOG_BASE(x) \
        do { \
            std::stringstream localss; \
            localss << x; \
            Internal_ssLogBase(localss); \
        } while(0)
    
#else
    #include <iostream>
    
    inline void Internal_ssLogBase(const std::stringstream& localss)
    {
        std::cout << localss.rdbuf() << std::endl;
    }
    
    #define ssLOG_BASE(x) \
        do{ \
            std::cout << x << std::endl; \
        } while(0)
#endif //ssLOG_LOG_TO_FILE

#define INTERNAL_UNSAFE_ssLOG_BASE(x) \
    do \
    { \
        std::stringstream localss; \
        localss << x; \
        if(InternalUnsafe_ssLogIsCacheOutput()) \
        { \
            InternalUnsafe_ssLogAppendCurrentCacheOutput(localss); \
            break; \
        } \
        \
        INTERNAL_ssLOG_LOCK_OUTPUT(); \
        Internal_ssLogBase(localss); \
        INTERNAL_ssLOG_UNLOCK_OUTPUT(); \
    } while(0)


//NOTE: ssLOG_BASE replaces ssLOG_SIMPLE for legacy reasons
#define ssLOG_SIMPLE(x) ssLOG_BASE(x)

#if ssLOG_SHOW_FILE_NAME
    inline std::string Internal_ssLogGetFileName(std::string fileName)
    {
        std::size_t ssLogfound = fileName.find_last_of("/\\");

        #if ssLOG_SHOW_LINE_NUM
            return "[" + fileName.substr(ssLogfound+1);
        #else
            return "[" + fileName.substr(ssLogfound+1) + "] ";
        #endif
    };

    #define INTERNAL_ssLOG_GET_FILE_NAME() Internal_ssLogGetFileName(__FILE__)
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

#define INTERNAL_ssLOG_MAP_READ_GUARD() \
    Internal_ssLogMapReadGuard mapGuard = Internal_ssLogMapReadGuard(); \
    (void)mapGuard;

inline void Internal_ssLogCheckNewThread()
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
        ssLogInfoMap[std::this_thread::get_id()].CacheOutput = ssLogNewThreadCacheByDefault.load();
    }
}

inline int InternalUnsafe_ssLogGetThreadId()
{
    return ssLogInfoMap.at(std::this_thread::get_id()).ID;
}

inline std::string InternalUnsafe_ssLogGetPrepend()
{
    std::string s;
    {
        auto& currentSS = ssLogInfoMap.at(std::this_thread::get_id()).CurrentPrepend;
        s = currentSS.str();
        currentSS.str("");
        currentSS.clear();
    }
    return s;
}

inline void InternalUnsafe_ssLogAppendPrepend(std::string appendMsg)
{
    ssLogInfoMap.at(std::this_thread::get_id()).CurrentPrepend << appendMsg;
}

inline void Internal_ssLogAppendPrepend(std::stringstream& ss)
{
    Internal_ssLogCheckNewThread();
    
    //Accessing map entry
    INTERNAL_ssLOG_MAP_READ_GUARD();
    ssLogInfoMap.at(std::this_thread::get_id()).CurrentPrepend << ss.rdbuf();
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
    Internal_ssLogCheckNewThread();
    
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
    Internal_ssLogCheckNewThread();
    
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

#if ssLOG_SHOW_THREADS
    #define INTERNAL_UNSAFE_ssLOG_PRINT_THREAD_ID() \
        "[Thread " << InternalUnsafe_ssLogGetThreadId() << "] "
#else
    #define INTERNAL_UNSAFE_ssLOG_PRINT_THREAD_ID()
#endif


// =======================================================================
// Macros for ssLOG_LINE, ssLOG_FUNC, ssLOG_FUNC_ENTRY, ssLOG_FUNC_EXIT and ssLOG_FUNC_CONTENT
// =======================================================================

#define ssLOG_LINE( ... ) do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_LINE, __VA_ARGS__ ) } while(0)

//NOTE: ssLOG_CONTENT replaces ssLOG_FUNC_CONTENT
#define ssLOG_CONTENT( ... ) ssLOG_FUNC_CONTENT( __VA_ARGS__ )

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

#if !ssLOG_CALL_STACK
    #define ssLOG_FUNC( ... ) do{}while(0)
    #define ssLOG_FUNC_ENTRY( ... ) do{}while(0)
    #define ssLOG_FUNC_EXIT( ... ) do{}while(0)
    #define ssLOG_FUNC_CONTENT(expr) expr; do{}while(0)

    inline void Internal_ssLogLine( std::string funcName, 
                                    std::string fileName, 
                                    std::string lineNum,
                                    std::string message,
                                    int level)
    {
        Internal_ssLogCheckNewThread();
        
        //Accessing map entry
        {
            INTERNAL_ssLOG_MAP_READ_GUARD();
            if(InternalUnsafe_ssLogGetTargetLogLevel() >= level)
            {
                Internal_ssLogLevelApplier levelApplier = Internal_ssLogLevelApplier(level);
                INTERNAL_UNSAFE_ssLOG_BASE( InternalUnsafe_ssLogGetThreadVSpace() <<
                                            INTERNAL_UNSAFE_ssLOG_PRINT_THREAD_ID() <<
                                            INTERNAL_ssLOG_GET_DATE_TIME() <<
                                            levelApplier <<
                                            InternalUnsafe_ssLogGetPrepend() <<
                                            funcName <<
                                            fileName <<
                                            lineNum << 
                                            message);
            }
        }
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
        INTERNAL_UNSAFE_ssLOG_BASE( InternalUnsafe_ssLogGetThreadVSpace() <<
                                    INTERNAL_UNSAFE_ssLOG_PRINT_THREAD_ID() << 
                                    INTERNAL_ssLOG_GET_DATE_TIME() << 
                                    Internal_ssLog_TabAdder(InternalUnsafe_ssLogGetTabSpace())); 
    }

    inline void Internal_ssLogLine( std::string funcName, 
                                    std::string fileName, 
                                    std::string lineNum,
                                    std::string message,
                                    int level)
    {
        Internal_ssLogCheckNewThread();
        
        //Accessing map entry
        {
            INTERNAL_ssLOG_MAP_READ_GUARD();
            if(InternalUnsafe_ssLogGetTargetLogLevel() >= level)
            {
                Internal_ssLogLevelApplier levelApplier = Internal_ssLogLevelApplier(level);
                INTERNAL_UNSAFE_ssLOG_BASE( InternalUnsafe_ssLogGetThreadVSpace() <<
                                            INTERNAL_UNSAFE_ssLOG_PRINT_THREAD_ID() <<
                                            INTERNAL_ssLOG_GET_DATE_TIME() <<
                                            Internal_ssLog_TabAdder(InternalUnsafe_ssLogGetTabSpace(), 
                                                                    true) <<
                                            levelApplier <<
                                            InternalUnsafe_ssLogGetPrepend() <<
                                            funcName <<
                                            fileName <<
                                            lineNum << 
                                            message);
            }
        }
    }

    inline void InternalUnsafe_ssLogFuncImpl(std::string fileName, std::string lineNum, int level)
    {
        Internal_ssLogLevelApplier levelApplier = Internal_ssLogLevelApplier(level);
        INTERNAL_UNSAFE_ssLOG_BASE( InternalUnsafe_ssLogGetThreadVSpace() <<
                                    INTERNAL_UNSAFE_ssLOG_PRINT_THREAD_ID() <<
                                    INTERNAL_ssLOG_GET_DATE_TIME() <<
                                    Internal_ssLog_TabAdder(InternalUnsafe_ssLogGetTabSpace(), 
                                                            true) <<
                                    levelApplier <<
                                    InternalUnsafe_ssLogGetPrepend() <<
                                    fileName <<
                                    lineNum);
    }
    
    inline void Internal_ssLogFuncEntry(std::string expr,
                                        std::string fileName, 
                                        std::string lineNum,
                                        int level)
    {
        Internal_ssLogCheckNewThread();
        
        //Accessing map entry
        INTERNAL_ssLOG_MAP_READ_GUARD();
        
        InternalUnsafe_ssLogGetLogLevelStack().push(level);
        if(InternalUnsafe_ssLogGetTargetLogLevel() >= level)
        { 
            InternalUnsafe_ssLogEmptyLine();
            InternalUnsafe_ssLogAppendPrepend(  INTERNAL_ssLOG_GET_CONTENT_NAME(expr) + 
                                                std::string("BEGINS "));
            InternalUnsafe_ssLogFuncImpl(fileName, lineNum, level);
        }

        InternalUnsafe_ssLogGetFuncNameStack().push(expr);
        InternalUnsafe_ssLogIncrementTabSpace();
    }

    inline void Internal_ssLogFuncExit( std::string expr,
                                        std::string fileName, 
                                        std::string lineNum)
    {
        Internal_ssLogCheckNewThread();
        
        //Accessing map entry
        INTERNAL_ssLOG_MAP_READ_GUARD();
        if( InternalUnsafe_ssLogGetFuncNameStack().empty() || 
            InternalUnsafe_ssLogGetFuncNameStack().top() != expr) 
        { 
            INTERNAL_UNSAFE_ssLOG_BASE( "ssLOG_FUNC_EXIT is missing somewhere. " << 
                                        expr << " is expected but" << 
                                        InternalUnsafe_ssLogGetFuncNameStack().top() << 
                                        " is found instead."); 
            
            ssLogReadCount--;
            return;
        }
        
        int currentLogLevel = InternalUnsafe_ssLogGetLogLevelStack().top();
        InternalUnsafe_ssLogGetLogLevelStack().pop();
        InternalUnsafe_ssLogGetFuncNameStack().pop();
        InternalUnsafe_ssLogDecrementTabSpace();
        if(InternalUnsafe_ssLogGetTargetLogLevel() >= currentLogLevel)
        {
            InternalUnsafe_ssLogAppendPrepend(  INTERNAL_ssLOG_GET_CONTENT_NAME(expr) + 
                                                std::string("EXITS "));
            InternalUnsafe_ssLogFuncImpl(fileName, lineNum, currentLogLevel);
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

    //INTERNAL_ssLOG_LIMIT_EXPR turns any expression into string limiting it to 50 characters
    #define INTERNAL_ssLOG_LIMIT_EXPR(x) \
        (std::string(#x).size() > 50 ? std::string(#x).substr(0, 50) + " ..." : #x)
    
    #define INTERNAL_ssLOG_LIMIT_STR(x) \
        (x.size() > 50 ? x.substr(0, 50) + " ..." : x)

    #define ssLOG_FUNC( ... ) INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC, __VA_ARGS__ )

    #define INERNAL_ssLOG_FUNC_LEVELED( ... ) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_LEVELED_IMPL, __VA_ARGS__ )

    #define ssLOG_FUNC_CONTENT(expr) INTERNAL_ssLOG_FUNC_CONTENT_LEVELED(expr, 0)

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
#endif //!ssLOG_CALL_STACK

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

#define ssLOG_PREPEND(x) \
    do{ \
        std::stringstream localssLogString; \
        localssLogString << x; \
        Internal_ssLogAppendPrepend(localssLogString); \
    } while(0)


// =======================================================================
// Macros for ssLOG_ENABLE_CACHE_OUTPUT, ssLOG_DISABLE_CACHE_OUTPUT, 
// ssLOG_ENABLE_CACHE_OUTPUT_FOR_CURRENT_THREAD, ssLOG_DISABLE_CACHE_OUTPUT_FOR_CURRENT_THREAD, 
// ssLOG_ENABLE_CACHE_OUTPUT_FOR_NEW_THREADS, ssLOG_DISABLE_CACHE_OUTPUT_FOR_NEW_THREADS, 
// ssLOG_CACHE_OUTPUT_FOR_SCOPE, ssLOG_OUTPUT_ALL_CACHE and ssLOG_OUTPUT_ALL_CACHE_GROUPED
// =======================================================================
inline void Internal_ssLogSetAllCacheOutput(bool cache)
{
    Internal_ssLogCheckNewThread();
    //Editing all map entries
    {
        std::unique_lock<std::mutex> lk(ssLogMapWriteMutex);
        while(ssLogReadCount > 0) {}    //Wait until all reads are done
        for(auto it = ssLogInfoMap.begin(); it != ssLogInfoMap.end(); ++it)
            it->second.CacheOutput = cache;
    }
}

#define ssLOG_ENABLE_CACHE_OUTPUT() Internal_ssLogSetAllCacheOutput(true)

#define ssLOG_DISABLE_CACHE_OUTPUT() Internal_ssLogSetAllCacheOutput(false)

#define ssLOG_ENABLE_CACHE_OUTPUT_FOR_CURRENT_THREAD() Internal_ssLogSetCacheOutput(true)

#define ssLOG_DISABLE_CACHE_OUTPUT_FOR_CURRENT_THREAD() Internal_ssLogSetCacheOutput(false)

#define ssLOG_IS_CACHE_OUTPUT_FOR_CURRENT_THREAD() Internal_ssLogIsCacheOutput()

#define ssLOG_ENABLE_CACHE_OUTPUT_FOR_NEW_THREADS() ssLogNewThreadCacheByDefault.store(true)

#define ssLOG_DISABLE_CACHE_OUTPUT_FOR_NEW_THREADS() ssLogNewThreadCacheByDefault.store(false)

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
    Internal_ssLogCacheScope \
    INTERNAL_ssLOG_SELECT(ssLogCacheScopeObj, __LINE__) = Internal_ssLogCacheScope()

inline void Internal_ssLogOutputAllCacheGrouped()
{
    Internal_ssLogCheckNewThread();
    //Reading all map entries
    {
        std::unique_lock<std::mutex> lk(ssLogMapWriteMutex);
        while(ssLogReadCount > 0) {}    //Wait until all reads are done
        
        INTERNAL_ssLOG_LOCK_OUTPUT();
        
        for(auto it = ssLogInfoMap.begin(); it != ssLogInfoMap.end(); ++it)
        {
            if(it->second.CurrentCachedOutput.empty())
                continue;

            for(int i = 0; i < it->second.CurrentCachedOutput.size(); i++)
                ssLOG_BASE(it->second.CurrentCachedOutput[i].second);

            it->second.CurrentCachedOutput.clear();
        }
        
        INTERNAL_ssLOG_UNLOCK_OUTPUT();
    }
}

inline void Internal_ssLogOutputAllCache()
{
    Internal_ssLogCheckNewThread();
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

#define ssLOG_OUTPUT_ALL_CACHE() Internal_ssLogOutputAllCache()

#define ssLOG_OUTPUT_ALL_CACHE_GROUPED() Internal_ssLogOutputAllCacheGrouped()

#define ssLOG_RESET_ALL_THREAD_INFO() Internal_ssLogResetAllThreadInfo()

// =======================================================================
// Macros for ssLOG_BENCH_START and ssLOG_BENCH_END
// =======================================================================

#define ssLOG_BENCH_START( ... ) \
    INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_START, __VA_ARGS__ )

#define ssLOG_BENCH_END( ... ) \
    do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_END, __VA_ARGS__ ); } while(0)

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

// =======================================================================
// Macros for ssLOG_SET_CURRENT_THREAD_TARGET_LEVEL
// =======================================================================

inline void Internal_ssLogSetCurrentThreadTargetLevel(int targetLevel)
{
    Internal_ssLogCheckNewThread();
    //Reading map
    INTERNAL_ssLOG_MAP_READ_GUARD();
    ssLogInfoMap.at(std::this_thread::get_id()).ssTargetLogLevel = targetLevel;
}

inline int Internal_ssLogGetCurrentThreadTargetLevel()
{
    Internal_ssLogCheckNewThread();
    //Reading map
    INTERNAL_ssLOG_MAP_READ_GUARD();
    return InternalUnsafe_ssLogGetTargetLogLevel();
}

#define ssLOG_SET_CURRENT_THREAD_TARGET_LEVEL(targetLevel) \
    Internal_ssLogSetCurrentThreadTargetLevel(targetLevel)

#define ssLOG_GET_CURRENT_THREAD_TARGET_LEVEL() Internal_ssLogGetCurrentThreadTargetLevel()

// =======================================================================
// Macros for output level
// =======================================================================

#define ssLOG_LEVEL_DEBUG 5
#define ssLOG_LEVEL_INFO 4
#define ssLOG_LEVEL_WARNING 3
#define ssLOG_LEVEL_ERROR 2
#define ssLOG_LEVEL_FATAL 1
#define ssLOG_LEVEL_NONE 0

#include <cstdint>
#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
#endif

#if ssLOG_USE_ESCAPE_SEQUENCES || !ssLOG_USE_WINDOWS_COLOR
    #if !defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
        #define TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES
    #endif
#endif

#include "./termcolor.hpp"

#if ssLOG_ASCII != 1 && ssLOG_LOG_TO_FILE != 1
    template <typename CharT>
    //inline std::basic_ostream<CharT>& Internal_ssLogApplyLogUnsafe(std::basic_ostream<CharT>& stream, int level)
    std::basic_ostream<CharT>& operator<<(  std::basic_ostream<CharT>& stream, 
                                            Internal_ssLogLevelApplier& applier)
    {
        switch(applier.Level)
        {
            case ssLOG_LEVEL_FATAL:
                stream <<   termcolor::colorize << 
                            termcolor::grey << 
                            termcolor::on_red << 
                            "[FATAL]" << 
                            termcolor::reset << " ";
                return stream;
            
            case ssLOG_LEVEL_ERROR:
                stream <<   termcolor::colorize << 
                            termcolor::grey << 
                            termcolor::on_bright_red << 
                            "[ERROR]" << 
                            termcolor::reset << " ";
                return stream;
            
            case ssLOG_LEVEL_WARNING:
                stream <<   termcolor::colorize << 
                            termcolor::grey << 
                            termcolor::on_yellow << 
                            "[WARNING]" << 
                            termcolor::reset << " ";
                return stream;
            
            case ssLOG_LEVEL_INFO:
                stream <<   termcolor::colorize << 
                            termcolor::white << 
                            termcolor::on_grey << 
                            "[INFO]" << 
                            termcolor::reset << " ";
                return stream;
            
            case ssLOG_LEVEL_DEBUG:
                stream <<   termcolor::colorize << 
                            termcolor::grey << 
                            termcolor::on_green << 
                            "[DEBUG]" << 
                            termcolor::reset << " ";
                return stream;
            
            default:
                return stream;
        }
    }
#else
    template <typename CharT>
    std::basic_ostream<CharT>& operator<<(  std::basic_ostream<CharT>& stream, 
                                            Internal_ssLogLevelApplier& applier)
    {
        switch(applier.Level)
        {
            case ssLOG_LEVEL_FATAL:
                stream << "[FATAL] ";
                return stream;
            
            case ssLOG_LEVEL_ERROR:
                stream << "[ERROR] ";
                return stream;
            
            case ssLOG_LEVEL_WARNING:
                stream << "[WARNING] ";
                return stream;
            
            case ssLOG_LEVEL_INFO:
                stream << "[INFO] ";
                return stream;
            
            case ssLOG_LEVEL_DEBUG:
                stream << "[DEBUG] ";
                return stream;
            
            default:
                return stream;
        }
    }
#endif //ssLOG_ASCII != 1 && ssLOG_LOG_TO_FILE != 1

#define INTERNAL_ssLOG_EXECUTE_COMMAND_0()

#define INTERNAL_ssLOG_EXECUTE_COMMAND_1(command) command

#define INTERNAL_ssLOG_EXECUTE_COMMAND_2(a, command) command

#if ssLOG_LEVEL >= ssLOG_LEVEL_FATAL
    #define ssLOG_FATAL(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FATAL, __VA_ARGS__ ) } while(0)

    #define INTERNAL_ssLOG_FATAL_0() \
        Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), "", \
                            ssLOG_LEVEL_FATAL);
    
    #define INTERNAL_ssLOG_FATAL_1(debugText) \
        std::stringstream localssLogString; \
        localssLogString << debugText; \
        Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), \
                            localssLogString.str(), \
                            ssLOG_LEVEL_FATAL);
    
    #define ssLOG_FUNC_CONTENT_FATAL(expr) \
        INTERNAL_ssLOG_FUNC_CONTENT_LEVELED(expr, ssLOG_LEVEL_FATAL)
    
    #define ssLOG_FUNC_ENTRY_FATAL(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_ENTRY_FATAL, __VA_ARGS__ ) } while(0)
    
    #define ssLOG_FUNC_EXIT_FATAL(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_EXIT_FATAL, __VA_ARGS__ ) } while(0)
    
    #define INTERNAL_ssLOG_FUNC_ENTRY_FATAL_0() \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(__func__, ssLOG_LEVEL_FATAL)
    
    #define INTERNAL_ssLOG_FUNC_EXIT_FATAL_0() \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(__func__, ssLOG_LEVEL_FATAL)
    
    #define INTERNAL_ssLOG_FUNC_ENTRY_FATAL_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_FATAL)
    
    #define INTERNAL_ssLOG_FUNC_EXIT_FATAL_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_FATAL)
    
    #define ssLOG_FUNC_FATAL(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_FATAL, __VA_ARGS__ )
    
    #define INTERNAL_ssLOG_FUNC_FATAL_0() \
        INTERNAL_ssLOG_FUNC_LEVELED_IMPL_1(ssLOG_LEVEL_FATAL)
    
    #define INTERNAL_ssLOG_FUNC_FATAL_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_FATAL)
    
    #define ssLOG_BENCH_START_FATAL(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_START_FATAL, __VA_ARGS__ )
    
    #define INTERNAL_ssLOG_BENCH_START_FATAL_0() \
        INTERNAL_ssLOG_BENCH_START_FATAL_1("")

    #define INTERNAL_ssLOG_BENCH_START_FATAL_1(benchName) \
        INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(benchName); \
        INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH_LEVELED(benchName, ssLOG_LEVEL_FATAL)
    
    #define ssLOG_BENCH_END_FATAL(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_END_FATAL, __VA_ARGS__ ); } while(0)
    
    #define INTERNAL_ssLOG_BENCH_END_FATAL_0() \
        static_assert(false, "ssLOG_BENCH_END_FATAL must accept 1 argument")

    #define INTERNAL_ssLOG_BENCH_END_FATAL_1(startVar) \
        Interna_ssLogBenchEnd(  startVar, \
                                INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                                INTERNAL_ssLOG_GET_FILE_NAME(), \
                                INTERNAL_ssLOG_GET_LINE_NUM(), \
                                ssLOG_LEVEL_FATAL)
#else
    #define ssLOG_FATAL(...) do{}while(0)
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
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_ERROR, __VA_ARGS__ ) } while(0)

    #define INTERNAL_ssLOG_ERROR_0() \
        Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), "", \
                            ssLOG_LEVEL_ERROR);
    
    #define INTERNAL_ssLOG_ERROR_1(debugText) \
        std::stringstream localssLogString; \
        localssLogString << debugText; \
        Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), \
                            localssLogString.str(), \
                            ssLOG_LEVEL_ERROR);

    #define ssLOG_FUNC_CONTENT_ERROR(expr) \
        INTERNAL_ssLOG_FUNC_CONTENT_LEVELED(expr, ssLOG_LEVEL_ERROR)

    #define ssLOG_FUNC_ENTRY_ERROR(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_ENTRY_ERROR, __VA_ARGS__ ) } while(0)

    #define ssLOG_FUNC_EXIT_ERROR(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_EXIT_ERROR, __VA_ARGS__ ) } while(0)

    #define INTERNAL_ssLOG_FUNC_ENTRY_ERROR_0() \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(__func__, ssLOG_LEVEL_ERROR)

    #define INTERNAL_ssLOG_FUNC_EXIT_ERROR_0() \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(__func__, ssLOG_LEVEL_ERROR)

    #define INTERNAL_ssLOG_FUNC_ENTRY_ERROR_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_ERROR)

    #define INTERNAL_ssLOG_FUNC_EXIT_ERROR_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_ERROR)

    #define ssLOG_FUNC_ERROR(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_ERROR, __VA_ARGS__ )

    #define INTERNAL_ssLOG_FUNC_ERROR_0() \
        INTERNAL_ssLOG_FUNC_LEVELED_IMPL_1(ssLOG_LEVEL_ERROR)

    #define INTERNAL_ssLOG_FUNC_ERROR_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_ERROR)

    #define ssLOG_BENCH_START_ERROR(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_START_ERROR, __VA_ARGS__ )

    #define INTERNAL_ssLOG_BENCH_START_ERROR_0() \
        INTERNAL_ssLOG_BENCH_START_ERROR_1("")

    #define INTERNAL_ssLOG_BENCH_START_ERROR_1(benchName) \
        INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(benchName); \
        INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH_LEVELED(benchName, ssLOG_LEVEL_ERROR)

    #define ssLOG_BENCH_END_ERROR(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_END_ERROR, __VA_ARGS__ ); } while(0)

    #define INTERNAL_ssLOG_BENCH_END_ERROR_0() \
        static_assert(false, "ssLOG_BENCH_END_ERROR must accept 1 argument")

    #define INTERNAL_ssLOG_BENCH_END_ERROR_1(startVar) \
        Interna_ssLogBenchEnd(  startVar, \
                                INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                                INTERNAL_ssLOG_GET_FILE_NAME(), \
                                INTERNAL_ssLOG_GET_LINE_NUM(), \
                                ssLOG_LEVEL_ERROR)
#else
    #define ssLOG_ERROR(...) do{}while(0)
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
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_WARNING, __VA_ARGS__ ) } while(0)

    #define INTERNAL_ssLOG_WARNING_0() \
        Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), "", \
                            ssLOG_LEVEL_WARNING);
    
    #define INTERNAL_ssLOG_WARNING_1(debugText) \
        std::stringstream localssLogString; \
        localssLogString << debugText; \
        Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), \
                            localssLogString.str(), \
                            ssLOG_LEVEL_WARNING);
    
    #define ssLOG_FUNC_CONTENT_WARNING(expr) \
        INTERNAL_ssLOG_FUNC_CONTENT_LEVELED(expr, ssLOG_LEVEL_WARNING)
    
    #define ssLOG_FUNC_ENTRY_WARNING(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_ENTRY_WARNING, __VA_ARGS__ ) } while(0)

    #define ssLOG_FUNC_EXIT_WARNING(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_EXIT_WARNING, __VA_ARGS__ ) } while(0)

    #define INTERNAL_ssLOG_FUNC_ENTRY_WARNING_0() \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(__func__, ssLOG_LEVEL_WARNING)

    #define INTERNAL_ssLOG_FUNC_EXIT_WARNING_0() \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(__func__, ssLOG_LEVEL_WARNING)

    #define INTERNAL_ssLOG_FUNC_ENTRY_WARNING_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_WARNING)

    #define INTERNAL_ssLOG_FUNC_EXIT_WARNING_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_WARNING)

    #define ssLOG_FUNC_WARNING(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_WARNING, __VA_ARGS__ )

    #define INTERNAL_ssLOG_FUNC_WARNING_0() \
        INTERNAL_ssLOG_FUNC_LEVELED_IMPL_1(ssLOG_LEVEL_WARNING)

    #define INTERNAL_ssLOG_FUNC_WARNING_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_WARNING)

    #define ssLOG_BENCH_START_WARNING(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_START_WARNING, __VA_ARGS__ )

    #define INTERNAL_ssLOG_BENCH_START_WARNING_0() \
        INTERNAL_ssLOG_BENCH_START_WARNING_1("", ssLOG_LEVEL_WARNING)

    #define INTERNAL_ssLOG_BENCH_START_WARNING_1(benchName) \
        INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(benchName); \
        INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH_LEVELED(benchName, ssLOG_LEVEL_WARNING)

    #define ssLOG_BENCH_END_WARNING(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_END_WARNING, __VA_ARGS__ ); } while(0)

    #define INTERNAL_ssLOG_BENCH_END_WARNING_0() \
        static_assert(false, "ssLOG_BENCH_END_WARNING must accept 1 argument")

    #define INTERNAL_ssLOG_BENCH_END_WARNING_1(startVar) \
        Interna_ssLogBenchEnd(  startVar, \
                                INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                                INTERNAL_ssLOG_GET_FILE_NAME(), \
                                INTERNAL_ssLOG_GET_LINE_NUM(), \
                                ssLOG_LEVEL_WARNING)
#else
    #define ssLOG_WARNING(...) do{}while(0)
    #define ssLOG_FUNC_CONTENT_WARNING(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_ENTRY_WARNING(...) do{}while(0)
    #define ssLOG_FUNC_EXIT_WARNING(...) do{}while(0)
    #define ssLOG_FUNC_WARNING(...) do{}while(0)
    #define ssLOG_BENCH_START_WARNING(...) INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__)
    #define ssLOG_BENCH_END_WARNING(...) do{}while(0)
#endif //ssLOG_LEVEL >= ssLOG_LEVEL_WARNING

#if ssLOG_LEVEL >= ssLOG_LEVEL_INFO
    #define ssLOG_INFO(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_INFO, __VA_ARGS__ ) } while(0)

    #define INTERNAL_ssLOG_INFO_0() \
        Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), "", \
                            ssLOG_LEVEL_INFO);
    
    #define INTERNAL_ssLOG_INFO_1(debugText) \
        std::stringstream localssLogString; \
        localssLogString << debugText; \
        Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), \
                            localssLogString.str(), \
                            ssLOG_LEVEL_INFO);
    
    #define ssLOG_FUNC_CONTENT_INFO(expr) \
        INTERNAL_ssLOG_FUNC_CONTENT_LEVELED(expr, ssLOG_LEVEL_INFO)
    
    #define ssLOG_FUNC_ENTRY_INFO(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_ENTRY_INFO, __VA_ARGS__ ) } while(0)

    #define ssLOG_FUNC_EXIT_INFO(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_EXIT_INFO, __VA_ARGS__ ) } while(0)

    #define INTERNAL_ssLOG_FUNC_ENTRY_INFO_0() \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(__func__, ssLOG_LEVEL_INFO)

    #define INTERNAL_ssLOG_FUNC_EXIT_INFO_0() \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(__func__, ssLOG_LEVEL_INFO)

    #define INTERNAL_ssLOG_FUNC_ENTRY_INFO_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_INFO)

    #define INTERNAL_ssLOG_FUNC_EXIT_INFO_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_INFO)

    #define ssLOG_FUNC_INFO(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_INFO, __VA_ARGS__ )

    #define INTERNAL_ssLOG_FUNC_INFO_0() \
        INTERNAL_ssLOG_FUNC_LEVELED_IMPL_1(ssLOG_LEVEL_INFO)

    #define INTERNAL_ssLOG_FUNC_INFO_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_INFO)

    #define ssLOG_BENCH_START_INFO(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_START_INFO, __VA_ARGS__ )

    #define INTERNAL_ssLOG_BENCH_START_INFO_0() \
        INTERNAL_ssLOG_BENCH_START_INFO_1("", ssLOG_LEVEL_INFO)

    #define INTERNAL_ssLOG_BENCH_START_INFO_1(benchName) \
        INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(benchName); \
        INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH_LEVELED(benchName, ssLOG_LEVEL_INFO)

    #define ssLOG_BENCH_END_INFO(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_END_INFO, __VA_ARGS__ ); } while(0)

    #define INTERNAL_ssLOG_BENCH_END_INFO_0() \
        static_assert(false, "ssLOG_BENCH_END_INFO must accept 1 argument")

    #define INTERNAL_ssLOG_BENCH_END_INFO_1(startVar) \
        Interna_ssLogBenchEnd(  startVar, \
                                INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                                INTERNAL_ssLOG_GET_FILE_NAME(), \
                                INTERNAL_ssLOG_GET_LINE_NUM(), \
                                ssLOG_LEVEL_INFO)
#else
    #define ssLOG_INFO(...) do{}while(0)
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
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_DEBUG, __VA_ARGS__ ) } while(0)

    #define INTERNAL_ssLOG_DEBUG_0() \
        Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), "", \
                            ssLOG_LEVEL_DEBUG);
    
    #define INTERNAL_ssLOG_DEBUG_1(debugText) \
        std::stringstream localssLogString; \
        localssLogString << debugText; \
        Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), \
                            localssLogString.str(), \
                            ssLOG_LEVEL_DEBUG);
    
    #define ssLOG_FUNC_CONTENT_DEBUG(expr) \
        INTERNAL_ssLOG_FUNC_CONTENT_LEVELED(expr, ssLOG_LEVEL_DEBUG)
    
    #define ssLOG_FUNC_ENTRY_DEBUG(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_ENTRY_DEBUG, __VA_ARGS__ ) } while(0)

    #define ssLOG_FUNC_EXIT_DEBUG(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_EXIT_DEBUG, __VA_ARGS__ ) } while(0)

    #define INTERNAL_ssLOG_FUNC_ENTRY_DEBUG_0() \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(__func__, ssLOG_LEVEL_DEBUG)

    #define INTERNAL_ssLOG_FUNC_EXIT_DEBUG_0() \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(__func__, ssLOG_LEVEL_DEBUG)

    #define INTERNAL_ssLOG_FUNC_ENTRY_DEBUG_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_DEBUG)

    #define INTERNAL_ssLOG_FUNC_EXIT_DEBUG_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_DEBUG)

    #define ssLOG_FUNC_DEBUG(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_DEBUG, __VA_ARGS__ )

    #define INTERNAL_ssLOG_FUNC_DEBUG_0() \
        INTERNAL_ssLOG_FUNC_LEVELED_IMPL_1(ssLOG_LEVEL_DEBUG)

    #define INTERNAL_ssLOG_FUNC_DEBUG_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_DEBUG)

    #define ssLOG_BENCH_START_DEBUG(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_START_DEBUG, __VA_ARGS__ )

    #define INTERNAL_ssLOG_BENCH_START_DEBUG_0() \
        INTERNAL_ssLOG_BENCH_START_DEBUG_1("", ssLOG_LEVEL_DEBUG)

    #define INTERNAL_ssLOG_BENCH_START_DEBUG_1(benchName) \
        INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(benchName); \
        INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH_LEVELED(benchName, ssLOG_LEVEL_DEBUG)

    #define ssLOG_BENCH_END_DEBUG(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_END_DEBUG, __VA_ARGS__ ); } while(0)

    #define INTERNAL_ssLOG_BENCH_END_DEBUG_0() \
        static_assert(false, "ssLOG_BENCH_END_DEBUG must accept 1 argument")

    #define INTERNAL_ssLOG_BENCH_END_DEBUG_1(startVar) \
        Interna_ssLogBenchEnd(  startVar, \
                                INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                                INTERNAL_ssLOG_GET_FILE_NAME(), \
                                INTERNAL_ssLOG_GET_LINE_NUM(), \
                                ssLOG_LEVEL_DEBUG)
#else
    #define ssLOG_DEBUG(...) do{}while(0)
    #define ssLOG_FUNC_CONTENT_DEBUG(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_ENTRY_DEBUG(...) do{}while(0)
    #define ssLOG_FUNC_EXIT_DEBUG(...) do{}while(0)
    #define ssLOG_FUNC_DEBUG(...) do{}while(0)
    #define ssLOG_BENCH_START_DEBUG(...) INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__)
    #define ssLOG_BENCH_END_DEBUG(...) do{}while(0)
#endif //ssLOG_LEVEL >= ssLOG_LEVEL_DEBUG

#endif //ssLOG_HPP

