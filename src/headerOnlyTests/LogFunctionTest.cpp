
# include "ssLogger/ssLogInit.hpp"
# include "ssLogger/ssLog.hpp"


void A()
{
    ssLOG_LINE("Function A content logged");
}

void B()
{
    ssLOG_FUNC_ENTRY();

    ssLOG_LINE("Function B content logged");
    
    ssLOG_FUNC_EXIT();
}

int C()
{
    ssLOG_LINE("Function C content logged");
    
    ssLOG_FUNC(A());

    return 42;
}

void FunctionWithALotOfArgs(int arg, int arg2, int arg3, int arg4, int arg5, int arg6)
{

}

int main()
{
    ssLOG_FUNC_ENTRY();
    
    B();

    ssLOG_FUNC( int cValue = C(); );

    ssLOG_FUNC
    (
        FunctionWithALotOfArgs
        (
            12312321, 
            1321514521, 
            21321, 
            2321321, 
            12321, 
            21321
        );
    );

    auto someLambda = []()
    {
        ssLOG_FUNC_ENTRY("Custom Lambda Function");
        
        ssLOG_LINE("Custom Lambda Function content logged");
        
        ssLOG_FUNC_EXIT("Custom Lambda Function");
    };

    someLambda();

    ssLOG_FUNC_EXIT();
    return 0;
}