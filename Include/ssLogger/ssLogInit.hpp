#ifndef ssLOG_INIT_HPP
#define ssLOG_INIT_HPP

#include "./ssLogSwitches.hpp"

#include <sstream>
#include <string>
#include <stack>

#if ssLOG_THREAD_SAFE
    #include <unordered_map>
    #include <thread>
    #include <mutex>

    #include "./ssLogThreadInfo.hpp"

    std::unordered_map<std::thread::id, ssLogThreadInfo> ssLogInfoMap = 
        std::unordered_map<std::thread::id, ssLogThreadInfo>();
    
    int ssNewThreadID = 0;
    int ssCurrentThreadID = 0;
    std::thread::id ssLastThreadID = std::thread::id();
    std::mutex ssLogMutex;
    
    std::string(*Internal_ssLogGetPrepend)(void) = []()
    {
        auto& currentSS = ssLogInfoMap[std::this_thread::get_id()].CurrentPrepend;
        std::string s = currentSS.str();
        currentSS.str("");
        currentSS.clear();
        return s;
    };
#else
    int ssTabSpace = 0;
    std::stack<std::string> ssFuncNameStack = std::stack<std::string>();
    std::stringstream ssCurrentPrepend;
    std::stack<int> ssLogLevelStack = std::stack<int>();

    std::string(*Internal_ssLogGetPrepend)(void) = []()
    {
        std::string s = ssCurrentPrepend.str();
        ssCurrentPrepend.str("");
        ssCurrentPrepend.clear();
        return s;
    };

#endif

int ssCurrentLogLevel = 0;

#if ssLOG_LOG_TO_FILE
    #include <fstream>
    std::ofstream ssLogFileStream = std::ofstream();
#else
    #ifdef _WIN32
        #include <windows.h>
        #undef max
        #undef DELETE
        bool ssLogInitLambdaStatus = []()
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

    std::string (*Internal_ssLogGetDateTime)(void) = []()
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
