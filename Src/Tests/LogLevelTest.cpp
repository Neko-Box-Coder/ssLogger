#ifndef ssLOG_USE_SOURCE
    #include "ssLogger/ssLogInit.hpp"
#endif

#include "ssLogger/ssLog.hpp"

void TestLogLevels()
{
    ssLOG_LINE("Test");
    ssLOG_FETAL("Test fetal");
    ssLOG_ERROR("Test error");
    ssLOG_WARNING("Test warning");
    ssLOG_INFO("Test info");
    ssLOG_DEBUG("Test debug");
}

void TestFuncFetal()
{
    ssLOG_FUNC_FETAL();
    ssLOG_LINE("TestFuncFetal");
    ssLOG_FUNC_CONTENT_FETAL( TestLogLevels(); );
}

void TestFuncError()
{
    ssLOG_FUNC_ERROR();
    ssLOG_LINE("TestFuncError");
    ssLOG_FUNC_CONTENT_ERROR( TestLogLevels(); );
}

void TestFuncWarning()
{
    ssLOG_FUNC_WARNING();
    ssLOG_LINE("TestFuncWarning");
    ssLOG_FUNC_CONTENT_WARNING( TestLogLevels(); );
}

void TestFuncInfo()
{
    ssLOG_FUNC_INFO();
    ssLOG_LINE("TestFuncInfo");
    ssLOG_FUNC_CONTENT_INFO( TestLogLevels(); );
}

void TestFuncDebug()
{
    ssLOG_FUNC_DEBUG();
    ssLOG_LINE("TestFuncDebug");
    ssLOG_FUNC_CONTENT_DEBUG( TestLogLevels(); );
}


int main()
{
    TestLogLevels();
    
    TestFuncFetal();
    TestFuncError();
    TestFuncWarning();
    TestFuncInfo();
    TestFuncDebug();
    
    return 0;
}