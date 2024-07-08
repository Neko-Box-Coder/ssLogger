#ifndef ssLOG_THREAD_INFO_HPP
#define ssLOG_THREAD_INFO_HPP

#include <stack>
#include <string>
#include <sstream>

struct ssLogThreadInfo
{
    int TabSpace = 0;
    std::stack<std::string> FuncNameStack = std::stack<std::string>();
    std::stack<int> LogLevelStack = std::stack<int>();
    std::stringstream CurrentPrepend;
    bool CacheOutput = false;
    std::stringstream CurrentCachedOutput;
    int ID;
    int ssCurrentLogLevel = 0;
    #ifdef ssLOG_LEVEL
        int ssTargetLogLevel = ssLOG_LEVEL;
    #else
        int ssTargetLogLevel = 3;
    #endif
    bool outputLocked = false;
};


#endif
