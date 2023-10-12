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

## 🔨 Usage:

### Logging a line:
```c++
    // Output:
    // 2022-08-14 16:49:53.802 [MethodName] in FileName.cpp on line 9
    ssLOG_LINE();

    // Output:
    // 2022-08-14 16:49:53.802 [MethodName] in FileName.cpp on line 9: [Here's some value: 42]
    int someValue = 42;
    ssLOG_LINE("Here's some value: "<<someValue);
```

### Logging with level:
![logLevel](./Resources/logLevels.png)
```c++
    // 2022-08-14 16:49:53.802 [FETAL] [MethodName] in FileName.cpp on line 9: [Test fetal]
    ssLOG_FETAL("Test fetal");

    // 2022-08-14 16:49:53.802 [ERROR] [MethodName] in FileName.cpp on line 9: [Test error]
    ssLOG_ERROR("Test error");

    // 2022-08-14 16:49:53.802 [WARNING] [MethodName] in FileName.cpp on line 9: [Test warning]
    ssLOG_WARNING("Test warning");

    // 2022-08-14 16:49:53.802 [INFO] [MethodName] in FileName.cpp on line 9: [Test info]
    ssLOG_INFO("Test info");

    // 2022-08-14 16:49:53.802 [DEBUG] [MethodName] in FileName.cpp on line 9: [Test debug]
    ssLOG_DEBUG("Test debug");

    // Output:
    // 2022-08-14 16:49:53.802 [WARNING] [MethodName] in FileName.cpp on line 9: [Here's some value: 42]
    int someValue = 42;
    ssLOG_WARNING("Here's some value: "<<someValue);
```

### Logging functions call stack (*With automatic macro*):

```c++
    //***Functions call stack are only logged when ssLOG_CALL_STACK is true***

    //Log function callstack with ssLOG_FUNC
    void B()
    {
        ssLOG_FUNC();

        //...
    }

    int C(bool b)
    {
        ssLOG_FUNC();
        
        //...

        if(b)
            return 42;

        //...

        return 43;
    }

    //You can also have custom names for functions as well, which is useful for lambda functions.
    auto lambda = []()
    {
        ssLOG_FUNC("My lambda function");

        //...
    };
    ```

    ### Logging functions call stack (*inline macro*):
    ```c++
    //***Functions call stack are only logged when ssLOG_CALL_STACK is true***

    void A()
    {
        //...
    }

    int B()
    {
        //...
        return 42;
    }

    int main()
    {
        // Or without space
        ssLOG_CONTENT( A() );
        
        ssLOG_CONTENT( int retVal = B() );

        // Or you can format it like this to log more than 1 statements!
        ssLOG_CONTENT
        (
            int retVal = B();
            // Some other statements...
        );

        // You can also add the log level suffix (_DEBUG, _ERROR, etc...) to any of these calls
        ssLOG_CONTENT_DEBUG
        (
            A();
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
| ssLOG_SHOW_TIME           | 1             | Show log time for all logged functions                                                                |
| ssLOG_THREAD_SAFE         | 1             | Use std::thread and ensure thread safety for all logged functions                                     |
| ssLOG_WRAP_WITH_BRACKET   | 1             | If true, contents will be wrapped square brackets                                                     |
| ssLOG_LOG_TO_FILE         | 0             | Log to file instead for all logged functions                                                          |
| ssLOG_LEVEL               | 3             | Log level (0: NONE, 1: FETAL, 2: ERROR, 3: WARNING, 4: INFO, 5: DEBUG)                                |
|                           |               | Recommended usage:                                                                                    |
|                           |               | NONE:     None of the levels will be printed, but will still print normal ssLOG_LINE or ssLOG_FUNC    |
|                           |               | FETAL:    Indicates program will crash                                                                |
|                           |               | ERROR:    Indicates program might crash and **likely** to not function correctly                      |
|                           |               | WARNING:  Indicates program won't crash but **might** not function correctly                          |
|                           |               | INFO:     Prints program state which **doesn't** spam the log                                         |
|                           |               | DEBUG:    Prints program state which **does** spam the log                                            |

----

### How to use:
1. Clone this repository **recursively**
    - `git submodule add https://github.com/Neko-Box-Coder/ssLogger.git <folder name>` and `git submodule update --init --recursive`
    - Or `git clone --recursive https://github.com/Neko-Box-Coder/ssLogger.git`
2. Decide if you want to use this with header-only or with source
    - Header only:
        1. Edit & include `include/ssLogSwitches.hpp` as you like
        2. Include `include/ssLogger/ssLog.hpp` to your header(s) below `ssLogSwitches.hpp`
        3. Include `include/ssLogger/ssLogInit.hpp` to your entry point **ONCE**
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
- Add option for only showing time instead of both date and time
- Add executable to merge thread logs together
