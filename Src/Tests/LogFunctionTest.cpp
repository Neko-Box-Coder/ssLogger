#ifndef ssLOG_USE_SOURCE
    #include "ssLogger/ssLogInit.hpp"
#endif

#include "ssLogger/ssLog.hpp"

#include <chrono>
#include <thread>


void InitializeApp()
{
    ssLOG_FUNC_ENTRY();
    
    //Initialize the application
    ssLOG_LINE("Initializing MyApp...");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    ssLOG_FUNC_EXIT();
}

void SanitizeData()
{
    ssLOG_FUNC();
    
    ssLOG_LINE("Sanitizing data...");
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void ProcessData()
{
    ssLOG_FUNC();
    
    //Process data
    ssLOG_LINE("Processing data...");
    SanitizeData();
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

int ProcessLotsOfData(  int userID,
                        std::string username, 
                        std::string password, 
                        int health,
                        int mana)
{
    (void)userID;
    (void)username;
    (void)password;
    (void)health;
    (void)mana;
    
    // Process data
    ssLOG_LINE("Processing data...");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    return 42;
}

void CleanupApp()
{
    ssLOG_FUNC();
    
    // Cleanup the application
    ssLOG_LINE("Cleaning up MyApp...");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

int main()
{
    ssLOG_FUNC();
    
    InitializeApp();

    ProcessData();
    
    ssLOG_CONTENT
    (
        ProcessLotsOfData
        (
            69, 
            "Bob", 
            "Very Secure Password", 
            9000, 
            9000
        );
    );
    
    auto userDataHandler = []()
    {
        ssLOG_FUNC("userDataHandler");
        ssLOG_LINE("Handling user data...");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    };

    userDataHandler();

    CleanupApp();

    return 0;
}
