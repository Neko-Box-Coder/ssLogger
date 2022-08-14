#ifndef ssLOG_DEBUG_INIT
#define ssLOG_DEBUG_INIT

#include "ssLogger/ssLogSwitches.hpp"

#include <string>
#include <stack>

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

    std::unordered_map<std::thread::id, ThreadLogInfo> ssLogInfoMap = std::unordered_map<std::thread::id, ThreadLogInfo>();
    std::thread::id ssLastThreadID = std::thread::id();
    std::mutex ssLogMutex;

#else

    int ssTabSpace = 0;
    std::stack<std::string> ssFuncNameStack = std::stack<std::string>();

#endif
#endif