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
    int ID;
};


#endif