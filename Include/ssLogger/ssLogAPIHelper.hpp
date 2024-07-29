#ifndef ssLOG_API_HELPER_HPP
#define ssLOG_API_HELPER_HPP

#ifndef ssLOG_API
    #if _MSC_VER
        #if ssLOG_DLL
            #if ssLOG_SHARED_EXPORT
                #define ssLOG_API __declspec(dllexport)
            #else
                #define ssLOG_API __declspec(dllimport)
            #endif
        #else
            #define ssLOG_API
        #endif
    #else
        #if ssLOG_DLL
            #define ssLOG_API __attribute__ ((visibility ("default")))
        #else
            #define ssLOG_API
        #endif
    #endif
#endif

#endif
