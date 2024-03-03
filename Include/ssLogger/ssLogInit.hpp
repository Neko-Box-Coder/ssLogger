#ifndef ssLOG_INIT_HPP
#define ssLOG_INIT_HPP

#include "ssLogger/ssLogSwitches.hpp"

#include <sstream>
#include <string>
#include <stack>

#if ssLOG_THREAD_SAFE
    #include <unordered_map>
    #include <thread>
    #include <mutex>

    #ifndef INTERNAL_ssTHREAD_LOG_INFO_DECL
    #define INTERNAL_ssTHREAD_LOG_INFO_DECL
        //NOTE: Any changes to this struct needs to be updated in the ssLog file as well
        struct ssLogThreadLogInfo
        {
            int TabSpace = 0;
            std::stack<std::string> FuncNameStack = std::stack<std::string>();
            std::stack<int> LogLevelStack = std::stack<int>();
            std::stringstream CurrentPrepend;
        };
    #endif

    std::unordered_map<std::thread::id, ssLogThreadLogInfo> ssLogInfoMap = 
        std::unordered_map<std::thread::id, ssLogThreadLogInfo>();
    
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

int ssLogLevel = 0;

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

#if ssLOG_SHOW_TIME
    #include <chrono>
    #include <sstream>
    #include <iomanip>
    #include <ctime>

    std::string(*Internal_ssLogGetTime)(void) = []()
    {
        std::chrono::system_clock::time_point ssLog_tp = std::chrono::system_clock::now();
        std::stringstream ssLog_ss;
        auto ssLog_time_t = std::chrono::system_clock::to_time_t(ssLog_tp);
        
        ssLog_ss    << std::put_time(std::localtime(&ssLog_time_t), "%Y-%m-%d %T")
                    << "." << std::setfill('0') << std::setw(3)
                    << (std::chrono::duration_cast<std::chrono::milliseconds>(ssLog_tp.time_since_epoch()).count() % 1000);
        ssLog_ss    << " ";
        return ssLog_ss.str();
    };
#endif

#endif