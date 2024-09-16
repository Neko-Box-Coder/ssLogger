#ifndef ssLOG_INIT_HPP
#define ssLOG_INIT_HPP

#include "./ssLogSwitches.hpp"
#include "./ssLogAPIHelper.hpp"

#include <sstream>
#include <string>
#include <stack>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>

#include "./ssLogThreadInfo.hpp"

ssLOG_API std::unordered_map<std::thread::id, ssLogThreadInfo> ssLogInfoMap = 
    std::unordered_map<std::thread::id, ssLogThreadInfo>();

ssLOG_API int ssLogNewThreadID = 0;
ssLOG_API std::atomic<bool> ssLogNewThreadCacheByDefault(false);
ssLOG_API std::atomic<int> ssLogReadCount(0);
ssLOG_API std::mutex ssLogMapWriteMutex;

#if ssLOG_THREAD_SAFE_OUTPUT
    ssLOG_API std::mutex ssLogOutputMutex;
#endif

#if ssLOG_LOG_TO_FILE
    #include <fstream>
    ssLOG_API std::ofstream ssLogFileStream = std::ofstream();
#else
    #ifdef _WIN32
        #ifndef NOMINMAX
            #define NOMINMAX
        #endif
        
        #include <windows.h>
        #undef DELETE
        ssLOG_API bool ssLogInitLambdaStatus = []()
        {
            return static_cast<bool>(SetConsoleOutputCP(CP_UTF8));
        }();
    #endif
    
#endif

#if ssLOG_SHOW_TIME || ssLOG_SHOW_DATE
    #include <chrono>
    #include <sstream>
    #include <iomanip>
    #include <ctime>

    ssLOG_API std::string (*Internal_ssLogGetDateTime)(void) = []()
    {
        using namespace std::chrono;
        
        system_clock::time_point ssLogTimePoint = system_clock::now();
        std::stringstream ssLogStringStream;
        std::time_t ssLogDateTime = system_clock::to_time_t(ssLogTimePoint);
        
        ssLogStringStream << "" <<
            #if ssLOG_SHOW_DATE
                std::put_time(std::localtime(&ssLogDateTime), "%Y-%m-%d ") <<
            #endif
            #if ssLOG_SHOW_TIME
                std::put_time(std::localtime(&ssLogDateTime), "%T") <<
                "." << std::setfill('0') << std::setw(3) << 
                (duration_cast<microseconds>(ssLogTimePoint.time_since_epoch()).count() / 1000) % 1000 << " ";
            #endif
        
        return ssLogStringStream.str();
    };
#endif

#endif
