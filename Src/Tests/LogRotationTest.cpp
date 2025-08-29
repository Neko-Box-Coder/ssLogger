#ifndef ssLOG_USE_SOURCE
    #include "ssLogger/ssLogInit.hpp"
#endif

#include "ssLogger/ssLog.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include <vector>

bool FileExists(const std::string& filename)
{
    std::ifstream file(filename);
    return file.good();
}

size_t GetFileSize(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if(!file.good()) return 0;
    return static_cast<size_t>(file.tellg());
}

void CleanupFiles(const std::vector<std::string>& files, const std::vector<bool>& shouldExist)
{
    for(int i = 0; i < files.size(); ++i)
    {
        if(FileExists(files[i]))
        {
            if(!shouldExist[i])
                std::cout << "ERROR: We expect " << files[i] << " to not exist but it exists\n";
            
            std::remove(files[i].c_str());
            std::cout << "Cleaned up: " << files[i] << std::endl;
        }
        else
        {
            if(shouldExist[i])
                std::cout << "ERROR: We expect " << files[i] << " to exist but it doesn't exist\n";
        }
    }
}

int main()
{
    std::cout << "=== Log Rotation Test ===" << std::endl;
    
    //Check if file logging is supported
    if(!ssLOG_IS_LOG_TO_FILE_ENABLED())
    {
        std::cout << "SKIPPED: File logging is not enabled in this build" << std::endl;
        return 0;
    }
    
    std::string logFileName = "test_rotation.txt";
    std::vector<std::string> createdFiles;
    
    //Enable file logging
    ssLOG_ENABLE_LOG_TO_FILE(true);
    ssLOG_SET_LOG_FILENAME(logFileName);
    createdFiles.push_back(logFileName);
    
    int testResult = 0;
    
    try
    {
        //Configure rotation settings - use small file size to trigger rotation
        ssLOG_ENABLE_ROTATION(true);
        ssLOG_SET_MAX_LOG_SIZE_MB(0.05); //0.05 MB (50KB) - lightweight test
        ssLOG_SET_MAX_ROTATED_FILES(3);
        
        std::cout << "Rotation enabled: " << (ssLOG_IS_ROTATION_ENABLED() ? "Yes" : "No") << std::endl;
        std::cout << "Max log size: " << ssLOG_GET_MAX_LOG_SIZE_MB() << " MB" << std::endl;
        std::cout << "Max rotated files: " << ssLOG_GET_MAX_ROTATED_FILES() << std::endl;
        std::cout << "Log file: " << ssLOG_GET_LOG_FILENAME() << std::endl;
        
        std::cout << "\nGenerating log entries to trigger rotation..." << std::endl;
        
        //Generate enough log data to trigger rotation - need ~500 messages to reach 50KB  
        for(int i = 0; i < 500; ++i)
            ssLOG_LINE("Test log entry " << i << " - Content for rotation testing.");
        
        //Check if main log file exists
        if(!FileExists(logFileName))
        {
            std::cerr << "ERROR: Main log file was not created!" << std::endl;
            testResult = 1;
            goto cleanup;
        }
        else
        {
            std::cout << "Main log file created successfully" << std::endl;
            size_t mainSize = GetFileSize(logFileName);
            std::cout << "    Current log file size: " << mainSize << " bytes" << std::endl;
        }
        
        //Generate more content to definitely trigger rotation
        std::cout << "\nGenerating more logs to test rotation..." << std::endl;
        size_t sizeBefore = ssLOG_GET_CURRENT_FILE_SIZE();
        std::cout << "Current file size before more logs: " << sizeBefore << " bytes" << std::endl;
        
        int rotationCount = 0;
        size_t maxSizeReached = 0;
        //Rotate 4 times
        for(int i = 500; i < 2100; ++i)
        {
            ssLOG_LINE("Additional test log " << i << " - More content for rotation test.");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            
            size_t currentSize = ssLOG_GET_CURRENT_FILE_SIZE();
            if(currentSize > maxSizeReached)
                maxSizeReached = currentSize;
            
            //Check if size suddenly dropped (indicating rotation occurred)
            if(maxSizeReached > 25000 && currentSize < 5000)
            {
                ++rotationCount;
                std::cout <<    "Rotation detected! Size dropped from " << maxSizeReached << " to " << 
                                currentSize << " bytes" << std::endl;
                
                //Add current log filename to cleanup list since rotation created a new file
                std::string newLogFileName = ssLOG_GET_LOG_FILENAME();
                createdFiles.push_back(newLogFileName);
                maxSizeReached = currentSize;
            }
            
            //Stop if we've clearly exceeded the limit and rotation should have occurred
            if(currentSize > 60000) //60KB - if we get here without rotation, something's wrong
            {
                std::cout << "ERROR: Generated over 60KB without rotation occurring" << std::endl;
                break;
            }
        }
        
        if(rotationCount == 3)
            std::cout << "Log rotation verified successfully!" << std::endl;
        else
        {
            std::cout <<    "ERROR: No rotation detected - either limit not reached or "
                            "rotation not working" << std::endl;
        }
        
        //Test disabling rotation
        std::cout << "\nTesting rotation disable..." << std::endl;
        ssLOG_ENABLE_ROTATION(false);
        
        if(!ssLOG_IS_ROTATION_ENABLED())
            std::cout << "Rotation successfully disabled" << std::endl;
        else
        {
            std::cerr << "ERROR: Failed to disable rotation!" << std::endl;
            testResult = 1;
        }
        
        std::cout << "\nTest completed successfully!" << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << "ERROR: Test failed with exception: " << e.what() << std::endl;
        testResult = 1;
    }
    
    
    cleanup:
    //Cleanup created files
    std::cout << "\nCleaning up test files..." << std::endl;
    std::cout << "Files to cleanup: " << std::endl;
    for(const std::string& file : createdFiles)
        std::cout << "    - " << file << std::endl;
    std::cout << std::endl;
    CleanupFiles(createdFiles, {false, true, true, true});
    
    return testResult;
}
