# ssLogger 📔
![](./logo.png)
##### Logs incoming....

---

#### Super simple macro based Logger for call stack and quick debug logging, with minimum dependencies, high flexiblity and works with C++ 11 or above.

#### Both header only or CMake option available.

#### 🗒️ Fully verbose with call stack?
![demo](./Resources/demo.gif)

#### 👟 Simple logging with just function name and line number?
![demo2](./Resources/demo2.gif)

#### 🧵 Thread-safety for multithreading? (Can be disabled for performance)
![demo2](./Resources/demo3.gif)

#### 🐞 Logging with different levels?
![logLevel](./Resources/logLevels.png)

## 🔨 Usage:

### Logging a line:
```c++
    ssLOG_LINE();

    int someValue = 42;
    ssLOG_LINE("Here's some value: " << someValue);
    
    //[Thread 0] 2022-08-14 16:49:53.802 MethodName() in FileName.cpp on line 9
    //[Thread 0] 2022-08-14 16:49:53.802 MethodName() in FileName.cpp on line 9: Here's some value: 42
```

### Logging with level:
```c++
    ssLOG_FATAL("Test fatal");
    ssLOG_ERROR("Test error");
    ssLOG_WARNING("Test warning");
    ssLOG_INFO("Test info");
    ssLOG_DEBUG("Test debug");

    int someValue = 42;
    ssLOG_WARNING("Here's some value: "<<someValue);
    
    //[Thread 0] 2022-08-14 16:49:53.802 [FATAL] MethodName() in FileName.cpp on line 9: Test fatal
    //[Thread 0] 2022-08-14 16:49:53.802 [ERROR] MethodName() in FileName.cpp on line 9: Test error
    //[Thread 0] 2022-08-14 16:49:53.802 [WARNING] MethodName() in FileName.cpp on line 9: Test warning
    //[Thread 0] 2022-08-14 16:49:53.802 [INFO] MethodName() in FileName.cpp on line 9: Test info
    //[Thread 0] 2022-08-14 16:49:53.802 [DEBUG] MethodName() in FileName.cpp on line 9: Test debug
    //[Thread 0] 2022-08-14 16:49:53.802 [WARNING] MethodName() in FileName.cpp on line 9: Here's some value: 42
```

### Logging functions call stack:

```c++
    //***Functions call stack are only logged when ssLOG_CALL_STACK is true***

    //Log function callstack with ssLOG_FUNC
    void InitializeApp()
    {
        ssLOG_FUNC();
        ssLOG_LINE("Initializing MyApp...");
    }

    void SanitizeData()
    {
        ssLOG_FUNC();
        ssLOG_LINE("Sanitizing data...");
    }

    void ProcessData()
    {
        ssLOG_FUNC();
        ssLOG_LINE("Processing data...");
        SanitizeData();
    }

    //You can also have custom names for functions as well, which is useful for lambda functions.
    auto userDataHandler = []()
    {
        ssLOG_FUNC("userDataHandler");
        ssLOG_LINE("Handling user data...");
    };
    
    int main()
    {
        ssLOG_FUNC();
        InitializeApp();
        ProcessData();
        
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
```

### Logging multiple lines as content:

```c++
    //***Functions call stack are only logged when ssLOG_CALL_STACK is true***

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
        
        ssLOG_LINE("Processing data...");
        return 42;
    }

    int main()
    {
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

        return 0;
    }
```
----

#### 🔧 Easy Customization:

##### CMake / Header Defines
| Define Macro Name         | Default Value | Explaination                                                                                          |
| ---                       | ---           | ---                                                                                                   |
| ssLOG_CALL_STACK          | 1             | Show call stack for all logged functions                                                              |
| ssLOG_LOG_WITH_ASCII      | 0             | Logging will only use ASCII characters,                                                               |
|                           |               | which changes the call stack tree characters and disables log level text highlights                   |
| ssLOG_SHOW_FILE_NAME      | 1             | Show file name for all logged functions                                                               |
|                           |               | ⚠️ **Warning:** This extracts the file name from the `__FILE__` macro in runtime,                     |
|                           |               | which contains the **full path** to the file, and will contain sensitive information                  |
|                           |               | such as **your username** or **system file structure**.                                               |
|                           |               | It is recommended to turn it **off** in any production build                                          |
| ssLOG_SHOW_LINE_NUM       | 1             | Show line number for all logged functions                                                             |
| ssLOG_SHOW_FUNC_NAME      | 1             | Show function name for all logged functions                                                           |
| ssLOG_SHOW_DATE           | 1             | Show log date for all logged functions                                                                |
| ssLOG_SHOW_TIME           | 1             | Show log time for all logged functions                                                                |
| ssLOG_THREAD_SAFE         | 1             | Use std::thread and ensure thread safety for all logged functions                                     |
| ssLOG_LOG_TO_FILE         | 0             | Log to file instead for all logged functions                                                          |
| ssLOG_LEVEL               | 3             | Log level (0: NONE, 1: FATAL, 2: ERROR, 3: WARNING, 4: INFO, 5: DEBUG)                                |
|                           |               | Recommended usage:                                                                                    |
|                           |               | NONE:     None of the levels will be printed, but will still print normal ssLOG_LINE or ssLOG_FUNC    |
|                           |               | FATAL:    Indicates program will crash                                                                |
|                           |               | ERROR:    Indicates program might crash and **likely** to not function correctly                      |
|                           |               | WARNING:  Indicates program won't crash but **might** not function correctly                          |
|                           |               | INFO:     Prints program state which **doesn't** spam the log                                         |
|                           |               | DEBUG:    Prints program state which **does** spam the log                                            |

----

### How to use:
1. Clone this repository **recursively**
    - `git submodule add https://github.com/Neko-Box-Coder/ssLogger.git <folder name>` and `git submodule update --init --recursive`
    - Or `git clone --recursive https://github.com/Neko-Box-Coder/ssLogger.git`
2. There are two ways to use ssLogger:
    - Header only:
        1. Edit (or redefine macros) specified in `include/ssLogSwitches.hpp` as you like
        2. Include `include/ssLogger/ssLog.hpp`
        3. Include `include/ssLogger/ssLogInit.hpp` to your entry point **ONCE** (above `include/ssLogger/ssLog.hpp`)
    - Source with CMake:
        1. Add `add_subdirectory(<path to ssLogger> <optional binary directory>)` to your `CMakeLists.txt`
        2. Link ssLogger with your target. `target_link_libraries(<Your Target> PUBLIC ssLogger)`
        3. Add `#include "ssLogger/ssLog.hpp"` to your header(s)
        4. Edit properties via CMake GUI or command line

> <font size="4">⚠️ **Warning:** Using ssLogger before main (i.e. inside static class initialization) will result undefined behaviour (as ssLogger uses global static variable).</font>

----

### Dependencies:

- [termcolor](https://github.com/ikalnytskyi/termcolor) with [license distributed](https://github.com/ikalnytskyi/termcolor/blob/master/LICENSE) 

- Common dependencies
    - `#include <sstream>`
    - `#include <string>`
    - `#include <stack>`
    - `#include <iostream>`
- ssLOG_THREAD_SAFE
    - `#include <unordered_map>`
    - `#include <thread>`
    - `#include <mutex>`
- ssLOG_LOG_TO_FILE
    - On: `#include <fstream>`, `#include <ctime>`
    - Off: 
        - `#include <cstdint>`
        - POSIX
            - `#include <unistd.h>`
        - Windows
            - `#include <io.h>`
            - `#include <windows.h>`
- ssLOG_SHOW_TIME
    - `#include <chrono>`
    - `#include <sstream>`
    - `#include <iomanip>`
    - `#include <ctime>`
- ssLOG_ASCII
    - Off:
        - `#include <cstdint>`
        - POSIX
            - `#include <unistd.h>`
        - Windows
            - `#include <io.h>`
            - `#include <windows.h>`
----

### 🔜 TODOs:
- Add script for running tests in different configurations
