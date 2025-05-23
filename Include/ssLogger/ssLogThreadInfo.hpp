#ifndef ssLOG_THREAD_INFO_HPP
#define ssLOG_THREAD_INFO_HPP

#include "./ssLogAPIHelper.hpp"

#include <stack>
#include <string>
#include <sstream>
#include <chrono>
#include <vector>

struct ssLogThreadInfo
{
    int TabSpace = 0;
    std::stack<std::string> FuncNameStack = std::stack<std::string>();
    std::stack<int> LogLevelStack = std::stack<int>();
    std::string CurrentPrepend;
    bool CacheOutput = false;
    std::vector<std::pair<std::chrono::system_clock::time_point, std::string>> CurrentCachedOutput;
    int ID;
    #ifdef ssLOG_LEVEL
        int ssTargetLogLevel = ssLOG_LEVEL;
    #else
        int ssTargetLogLevel = 3;
    #endif
    bool outputLocked = false;
};


#endif
