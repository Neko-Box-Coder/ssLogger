#ifndef ssLOG_LEVELS_HPP
#define ssLOG_LEVELS_HPP

//NOTE: This file should not be included by itself

// =======================================================================
// Macros for output level
// =======================================================================

#include <cstdint>
#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
#endif

#if ssLOG_USE_ESCAPE_SEQUENCES || !ssLOG_USE_WINDOWS_COLOR
    #if !defined(TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES)
        #define TERMCOLOR_USE_ANSI_ESCAPE_SEQUENCES
    #endif
#endif

#include "./termcolor.hpp"

#if ssLOG_ASCII != 1 && ssLOG_LOG_TO_FILE != 1
    template <typename CharT>
    //inline std::basic_ostream<CharT>& Internal_ssLogApplyLogUnsafe(std::basic_ostream<CharT>& stream, int level)
    std::basic_ostream<CharT>& operator<<(  std::basic_ostream<CharT>& stream, 
                                            Internal_ssLogLevelApplier& applier)
    {
        switch(applier.Level)
        {
            case ssLOG_LEVEL_FATAL:
                stream <<   termcolor::colorize << 
                            termcolor::grey << 
                            termcolor::on_red << 
                            "[FATAL]" << 
                            termcolor::reset << " ";
                return stream;
            
            case ssLOG_LEVEL_ERROR:
                stream <<   termcolor::colorize << 
                            termcolor::grey << 
                            termcolor::on_bright_red << 
                            "[ERROR]" << 
                            termcolor::reset << " ";
                return stream;
            
            case ssLOG_LEVEL_WARNING:
                stream <<   termcolor::colorize << 
                            termcolor::grey << 
                            termcolor::on_yellow << 
                            "[WARNING]" << 
                            termcolor::reset << " ";
                return stream;
            
            case ssLOG_LEVEL_INFO:
                stream <<   termcolor::colorize << 
                            termcolor::white << 
                            termcolor::on_grey << 
                            "[INFO]" << 
                            termcolor::reset << " ";
                return stream;
            
            case ssLOG_LEVEL_DEBUG:
                stream <<   termcolor::colorize << 
                            termcolor::grey << 
                            termcolor::on_green << 
                            "[DEBUG]" << 
                            termcolor::reset << " ";
                return stream;
            
            default:
                return stream;
        }
    }
#else
    template <typename CharT>
    std::basic_ostream<CharT>& operator<<(  std::basic_ostream<CharT>& stream, 
                                            Internal_ssLogLevelApplier& applier)
    {
        switch(applier.Level)
        {
            case ssLOG_LEVEL_FATAL:
                stream << "[FATAL] ";
                return stream;
            
            case ssLOG_LEVEL_ERROR:
                stream << "[ERROR] ";
                return stream;
            
            case ssLOG_LEVEL_WARNING:
                stream << "[WARNING] ";
                return stream;
            
            case ssLOG_LEVEL_INFO:
                stream << "[INFO] ";
                return stream;
            
            case ssLOG_LEVEL_DEBUG:
                stream << "[DEBUG] ";
                return stream;
            
            default:
                return stream;
        }
    }
#endif //#if ssLOG_ASCII != 1 && ssLOG_LOG_TO_FILE != 1

#define INTERNAL_ssLOG_EXECUTE_COMMAND_0()

#define INTERNAL_ssLOG_EXECUTE_COMMAND_1(command) command

#define INTERNAL_ssLOG_EXECUTE_COMMAND_2(a, command) command

#if !ssLOG_DISABLE_LOGS && ssLOG_LEVEL >= ssLOG_LEVEL_FATAL
    #if !ssLOG_CALL_STACK_ONLY
        #define ssLOG_FATAL(...) \
            do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FATAL, __VA_ARGS__ ) } while(0)
    #else
        #define ssLOG_FATAL(...) do{} while(false)
    #endif

    #define INTERNAL_ssLOG_FATAL_0() \
        Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), "", \
                            ssLOG_LEVEL_FATAL);
    
    #define INTERNAL_ssLOG_FATAL_1(debugText) \
        std::stringstream localssLogString; \
        localssLogString << debugText; \
        Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), \
                            localssLogString.str(), \
                            ssLOG_LEVEL_FATAL);
    
    #if !ssLOG_CALL_STACK_ONLY
        #define ssLOG_FUNC_CONTENT_FATAL(expr) \
            INTERNAL_ssLOG_FUNC_CONTENT_LEVELED(expr, ssLOG_LEVEL_FATAL)
    #else
        #define ssLOG_FUNC_CONTENT_FATAL(expr) expr
    #endif
    
    #define ssLOG_FUNC_ENTRY_FATAL(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_ENTRY_FATAL, __VA_ARGS__ ) } while(0)
    
    #define ssLOG_FUNC_EXIT_FATAL(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_EXIT_FATAL, __VA_ARGS__ ) } while(0)
    
    #define INTERNAL_ssLOG_FUNC_ENTRY_FATAL_0() \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(__func__, ssLOG_LEVEL_FATAL)
    
    #define INTERNAL_ssLOG_FUNC_EXIT_FATAL_0() \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(__func__, ssLOG_LEVEL_FATAL)
    
    #define INTERNAL_ssLOG_FUNC_ENTRY_FATAL_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_FATAL)
    
    #define INTERNAL_ssLOG_FUNC_EXIT_FATAL_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_FATAL)
    
    #define ssLOG_FUNC_FATAL(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_FATAL, __VA_ARGS__ )
    
    #define INTERNAL_ssLOG_FUNC_FATAL_0() \
        INTERNAL_ssLOG_FUNC_LEVELED_IMPL_1(ssLOG_LEVEL_FATAL)
    
    #define INTERNAL_ssLOG_FUNC_FATAL_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_FATAL)
    
    #if !ssLOG_CALL_STACK_ONLY
        #define ssLOG_BENCH_START_FATAL(...) \
            INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_START_FATAL, __VA_ARGS__ )
    #else
        #define ssLOG_BENCH_START_FATAL(...) INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__)
    #endif
    
    #define INTERNAL_ssLOG_BENCH_START_FATAL_0() \
        INTERNAL_ssLOG_BENCH_START_FATAL_1("")

    #define INTERNAL_ssLOG_BENCH_START_FATAL_1(benchName) \
        INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(benchName); \
        INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH_LEVELED(benchName, ssLOG_LEVEL_FATAL)
    
    #if !ssLOG_CALL_STACK_ONLY
        #define ssLOG_BENCH_END_FATAL(...) \
            do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_END_FATAL, __VA_ARGS__ ); } while(0)
    #else
        #define ssLOG_BENCH_END_FATAL(...) do{} while(false)
    #endif
    
    #define INTERNAL_ssLOG_BENCH_END_FATAL_0() \
        static_assert(false, "ssLOG_BENCH_END_FATAL must accept 1 argument")

    #define INTERNAL_ssLOG_BENCH_END_FATAL_1(startVar) \
        Interna_ssLogBenchEnd(  startVar, \
                                INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                                INTERNAL_ssLOG_GET_FILE_NAME(), \
                                INTERNAL_ssLOG_GET_LINE_NUM(), \
                                ssLOG_LEVEL_FATAL)
#else
    #define ssLOG_FATAL(...) do{}while(0)
    #define ssLOG_FUNC_CONTENT_FATAL(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_ENTRY_FATAL(...) do{}while(0)
    #define ssLOG_FUNC_EXIT_FATAL(...) do{}while(0)
    #define ssLOG_FUNC_FATAL(...) do{}while(0)
    #define ssLOG_BENCH_START_FATAL(...) INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__)
    #define ssLOG_BENCH_END_FATAL(...) do{}while(0)
#endif //#if !ssLOG_DISABLE_LOGS && ssLOG_LEVEL >= ssLOG_LEVEL_FATAL

#if !ssLOG_DISABLE_LOGS && ssLOG_LEVEL >= ssLOG_LEVEL_ERROR
    #if !ssLOG_CALL_STACK_ONLY
        #define ssLOG_ERROR(...) \
            do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_ERROR, __VA_ARGS__ ) } while(0)
    #else
        #define ssLOG_ERROR(...) do{} while(false)
    #endif

    #define INTERNAL_ssLOG_ERROR_0() \
        Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), "", \
                            ssLOG_LEVEL_ERROR);
    
    #define INTERNAL_ssLOG_ERROR_1(debugText) \
        std::stringstream localssLogString; \
        localssLogString << debugText; \
        Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), \
                            localssLogString.str(), \
                            ssLOG_LEVEL_ERROR);

    #if !ssLOG_CALL_STACK_ONLY
        #define ssLOG_FUNC_CONTENT_ERROR(expr) \
            INTERNAL_ssLOG_FUNC_CONTENT_LEVELED(expr, ssLOG_LEVEL_ERROR)
    #else
        #define ssLOG_FUNC_CONTENT_ERROR(expr) expr
    #endif

    #define ssLOG_FUNC_ENTRY_ERROR(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_ENTRY_ERROR, __VA_ARGS__ ) } while(0)

    #define ssLOG_FUNC_EXIT_ERROR(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_EXIT_ERROR, __VA_ARGS__ ) } while(0)

    #define INTERNAL_ssLOG_FUNC_ENTRY_ERROR_0() \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(__func__, ssLOG_LEVEL_ERROR)

    #define INTERNAL_ssLOG_FUNC_EXIT_ERROR_0() \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(__func__, ssLOG_LEVEL_ERROR)

    #define INTERNAL_ssLOG_FUNC_ENTRY_ERROR_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_ERROR)

    #define INTERNAL_ssLOG_FUNC_EXIT_ERROR_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_ERROR)

    #define ssLOG_FUNC_ERROR(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_ERROR, __VA_ARGS__ )

    #define INTERNAL_ssLOG_FUNC_ERROR_0() \
        INTERNAL_ssLOG_FUNC_LEVELED_IMPL_1(ssLOG_LEVEL_ERROR)

    #define INTERNAL_ssLOG_FUNC_ERROR_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_ERROR)

    #if !ssLOG_CALL_STACK_ONLY
        #define ssLOG_BENCH_START_ERROR(...) \
            INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_START_ERROR, __VA_ARGS__ )
    #else
        #define ssLOG_BENCH_START_ERROR(...) INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__)
    #endif

    #define INTERNAL_ssLOG_BENCH_START_ERROR_0() \
        INTERNAL_ssLOG_BENCH_START_ERROR_1("")

    #define INTERNAL_ssLOG_BENCH_START_ERROR_1(benchName) \
        INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(benchName); \
        INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH_LEVELED(benchName, ssLOG_LEVEL_ERROR)

    #if !ssLOG_CALL_STACK_ONLY
        #define ssLOG_BENCH_END_ERROR(...) \
            do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_END_ERROR, __VA_ARGS__ ); } while(0)
    #else
        #define ssLOG_BENCH_END_ERROR(...) do{} while(false)
    #endif

    #define INTERNAL_ssLOG_BENCH_END_ERROR_0() \
        static_assert(false, "ssLOG_BENCH_END_ERROR must accept 1 argument")

    #define INTERNAL_ssLOG_BENCH_END_ERROR_1(startVar) \
        Interna_ssLogBenchEnd(  startVar, \
                                INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                                INTERNAL_ssLOG_GET_FILE_NAME(), \
                                INTERNAL_ssLOG_GET_LINE_NUM(), \
                                ssLOG_LEVEL_ERROR)
#else
    #define ssLOG_ERROR(...) do{}while(0)
    #define ssLOG_FUNC_CONTENT_ERROR(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_ENTRY_ERROR(...) do{}while(0)
    #define ssLOG_FUNC_EXIT_ERROR(...) do{}while(0)
    #define ssLOG_FUNC_ERROR(...) do{}while(0)
    #define ssLOG_BENCH_START_ERROR(...) INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__)
    #define ssLOG_BENCH_END_ERROR(...) do{}while(0)
#endif //#if !ssLOG_DISABLE_LOGS && ssLOG_LEVEL >= ssLOG_LEVEL_ERROR

#if ssLOG_LEVEL >= ssLOG_LEVEL_WARNING
    #if !ssLOG_CALL_STACK_ONLY
        #define ssLOG_WARNING(...) \
            do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_WARNING, __VA_ARGS__ ) } while(0)
    #else
        #define ssLOG_WARNING(...) do{} while(false)
    #endif

    #define INTERNAL_ssLOG_WARNING_0() \
        Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), "", \
                            ssLOG_LEVEL_WARNING);
    
    #define INTERNAL_ssLOG_WARNING_1(debugText) \
        std::stringstream localssLogString; \
        localssLogString << debugText; \
        Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), \
                            localssLogString.str(), \
                            ssLOG_LEVEL_WARNING);
    
    #if !ssLOG_CALL_STACK_ONLY
        #define ssLOG_FUNC_CONTENT_WARNING(expr) \
            INTERNAL_ssLOG_FUNC_CONTENT_LEVELED(expr, ssLOG_LEVEL_WARNING)
    #else
        #define ssLOG_FUNC_CONTENT_WARNING(expr) expr
    #endif
    
    #define ssLOG_FUNC_ENTRY_WARNING(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_ENTRY_WARNING, __VA_ARGS__ ) } while(0)

    #define ssLOG_FUNC_EXIT_WARNING(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_EXIT_WARNING, __VA_ARGS__ ) } while(0)

    #define INTERNAL_ssLOG_FUNC_ENTRY_WARNING_0() \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(__func__, ssLOG_LEVEL_WARNING)

    #define INTERNAL_ssLOG_FUNC_EXIT_WARNING_0() \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(__func__, ssLOG_LEVEL_WARNING)

    #define INTERNAL_ssLOG_FUNC_ENTRY_WARNING_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_WARNING)

    #define INTERNAL_ssLOG_FUNC_EXIT_WARNING_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_WARNING)

    #define ssLOG_FUNC_WARNING(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_WARNING, __VA_ARGS__ )

    #define INTERNAL_ssLOG_FUNC_WARNING_0() \
        INTERNAL_ssLOG_FUNC_LEVELED_IMPL_1(ssLOG_LEVEL_WARNING)

    #define INTERNAL_ssLOG_FUNC_WARNING_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_WARNING)

    #if !ssLOG_CALL_STACK_ONLY
        #define ssLOG_BENCH_START_WARNING(...) \
            INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_START_WARNING, __VA_ARGS__ )
    #else
        #define ssLOG_BENCH_START_WARNING(...) INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__)
    #endif

    #define INTERNAL_ssLOG_BENCH_START_WARNING_0() \
        INTERNAL_ssLOG_BENCH_START_WARNING_1("")

    #define INTERNAL_ssLOG_BENCH_START_WARNING_1(benchName) \
        INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(benchName); \
        INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH_LEVELED(benchName, ssLOG_LEVEL_WARNING)

    #if !ssLOG_CALL_STACK_ONLY
        #define ssLOG_BENCH_END_WARNING(...) \
            do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_END_WARNING, __VA_ARGS__ ); } while(0)
    #else
        #define ssLOG_BENCH_END_WARNING(...) do{} while(false)
    #endif

    #define INTERNAL_ssLOG_BENCH_END_WARNING_0() \
        static_assert(false, "ssLOG_BENCH_END_WARNING must accept 1 argument")

    #define INTERNAL_ssLOG_BENCH_END_WARNING_1(startVar) \
        Interna_ssLogBenchEnd(  startVar, \
                                INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                                INTERNAL_ssLOG_GET_FILE_NAME(), \
                                INTERNAL_ssLOG_GET_LINE_NUM(), \
                                ssLOG_LEVEL_WARNING)
#else
    #define ssLOG_WARNING(...) do{}while(0)
    #define ssLOG_FUNC_CONTENT_WARNING(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_ENTRY_WARNING(...) do{}while(0)
    #define ssLOG_FUNC_EXIT_WARNING(...) do{}while(0)
    #define ssLOG_FUNC_WARNING(...) do{}while(0)
    #define ssLOG_BENCH_START_WARNING(...) INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__)
    #define ssLOG_BENCH_END_WARNING(...) do{}while(0)
#endif //#if !ssLOG_DISABLE_LOGS && ssLOG_LEVEL >= ssLOG_LEVEL_WARNING

#if !ssLOG_DISABLE_LOGS && ssLOG_LEVEL >= ssLOG_LEVEL_INFO
    #if !ssLOG_CALL_STACK_ONLY
        #define ssLOG_INFO(...) \
            do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_INFO, __VA_ARGS__ ) } while(0)
    #else
        #define ssLOG_INFO(...) do{} while(false)
    #endif

    #define INTERNAL_ssLOG_INFO_0() \
        Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), "", \
                            ssLOG_LEVEL_INFO);
    
    #define INTERNAL_ssLOG_INFO_1(debugText) \
        std::stringstream localssLogString; \
        localssLogString << debugText; \
        Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), \
                            localssLogString.str(), \
                            ssLOG_LEVEL_INFO);
    
    #if !ssLOG_CALL_STACK_ONLY
        #define ssLOG_FUNC_CONTENT_INFO(expr) \
            INTERNAL_ssLOG_FUNC_CONTENT_LEVELED(expr, ssLOG_LEVEL_INFO)
    #else
        #define ssLOG_FUNC_CONTENT_INFO(expr) expr
    #endif
    
    #define ssLOG_FUNC_ENTRY_INFO(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_ENTRY_INFO, __VA_ARGS__ ) } while(0)

    #define ssLOG_FUNC_EXIT_INFO(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_EXIT_INFO, __VA_ARGS__ ) } while(0)

    #define INTERNAL_ssLOG_FUNC_ENTRY_INFO_0() \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(__func__, ssLOG_LEVEL_INFO)

    #define INTERNAL_ssLOG_FUNC_EXIT_INFO_0() \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(__func__, ssLOG_LEVEL_INFO)

    #define INTERNAL_ssLOG_FUNC_ENTRY_INFO_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_INFO)

    #define INTERNAL_ssLOG_FUNC_EXIT_INFO_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_INFO)

    #define ssLOG_FUNC_INFO(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_INFO, __VA_ARGS__ )

    #define INTERNAL_ssLOG_FUNC_INFO_0() \
        INTERNAL_ssLOG_FUNC_LEVELED_IMPL_1(ssLOG_LEVEL_INFO)

    #define INTERNAL_ssLOG_FUNC_INFO_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_INFO)

    #if !ssLOG_CALL_STACK_ONLY
        #define ssLOG_BENCH_START_INFO(...) \
            INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_START_INFO, __VA_ARGS__ )
    #else
        #define ssLOG_BENCH_START_INFO(...) INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__)
    #endif

    #define INTERNAL_ssLOG_BENCH_START_INFO_0() \
        INTERNAL_ssLOG_BENCH_START_INFO_1("")

    #define INTERNAL_ssLOG_BENCH_START_INFO_1(benchName) \
        INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(benchName); \
        INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH_LEVELED(benchName, ssLOG_LEVEL_INFO)

    #if !ssLOG_CALL_STACK_ONLY
        #define ssLOG_BENCH_END_INFO(...) \
            do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_END_INFO, __VA_ARGS__ ); } while(0)
    #else
        #define ssLOG_BENCH_END_INFO(...) do{} while(false)
    #endif

    #define INTERNAL_ssLOG_BENCH_END_INFO_0() \
        static_assert(false, "ssLOG_BENCH_END_INFO must accept 1 argument")

    #define INTERNAL_ssLOG_BENCH_END_INFO_1(startVar) \
        Interna_ssLogBenchEnd(  startVar, \
                                INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                                INTERNAL_ssLOG_GET_FILE_NAME(), \
                                INTERNAL_ssLOG_GET_LINE_NUM(), \
                                ssLOG_LEVEL_INFO)
#else
    #define ssLOG_INFO(...) do{}while(0)
    #define ssLOG_FUNC_CONTENT_INFO(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_ENTRY_INFO(...) do{}while(0)
    #define ssLOG_FUNC_EXIT_INFO(...) do{}while(0)
    #define ssLOG_FUNC_INFO(...) do{}while(0)
    #define ssLOG_BENCH_START_INFO(...) INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__)
    #define ssLOG_BENCH_END_INFO(...) do{}while(0)
#endif //#if !ssLOG_DISABLE_LOGS && ssLOG_LEVEL >= ssLOG_LEVEL_INFO

#if !ssLOG_DISABLE_LOGS && ssLOG_LEVEL >= ssLOG_LEVEL_DEBUG
    #if !ssLOG_CALL_STACK_ONLY
        #define ssLOG_DEBUG(...) \
            do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_DEBUG, __VA_ARGS__ ) } while(0)
    #else
        #define ssLOG_DEBUG(...) do{} while(false)
    #endif

    #define INTERNAL_ssLOG_DEBUG_0() \
        Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), "", \
                            ssLOG_LEVEL_DEBUG);
    
    #define INTERNAL_ssLOG_DEBUG_1(debugText) \
        std::stringstream localssLogString; \
        localssLogString << debugText; \
        Internal_ssLogLine( INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                            INTERNAL_ssLOG_GET_FILE_NAME(), \
                            INTERNAL_ssLOG_GET_LINE_NUM(), \
                            localssLogString.str(), \
                            ssLOG_LEVEL_DEBUG);
    
    #if !ssLOG_CALL_STACK_ONLY
        #define ssLOG_FUNC_CONTENT_DEBUG(expr) \
            INTERNAL_ssLOG_FUNC_CONTENT_LEVELED(expr, ssLOG_LEVEL_DEBUG)
    #else
        #define ssLOG_FUNC_CONTENT_DEBUG(expr) expr
    #endif
    
    #define ssLOG_FUNC_ENTRY_DEBUG(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_ENTRY_DEBUG, __VA_ARGS__ ) } while(0)

    #define ssLOG_FUNC_EXIT_DEBUG(...) \
        do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_EXIT_DEBUG, __VA_ARGS__ ) } while(0)

    #define INTERNAL_ssLOG_FUNC_ENTRY_DEBUG_0() \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(__func__, ssLOG_LEVEL_DEBUG)

    #define INTERNAL_ssLOG_FUNC_EXIT_DEBUG_0() \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(__func__, ssLOG_LEVEL_DEBUG)

    #define INTERNAL_ssLOG_FUNC_ENTRY_DEBUG_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_ENTRY_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_DEBUG)

    #define INTERNAL_ssLOG_FUNC_EXIT_DEBUG_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_EXIT_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_DEBUG)

    #define ssLOG_FUNC_DEBUG(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_FUNC_DEBUG, __VA_ARGS__ )

    #define INTERNAL_ssLOG_FUNC_DEBUG_0() \
        INTERNAL_ssLOG_FUNC_LEVELED_IMPL_1(ssLOG_LEVEL_DEBUG)

    #define INTERNAL_ssLOG_FUNC_DEBUG_1(customFuncName) \
        INTERNAL_ssLOG_FUNC_LEVELED_IMPL_2(customFuncName, ssLOG_LEVEL_DEBUG)

    #if !ssLOG_CALL_STACK_ONLY
        #define ssLOG_BENCH_START_DEBUG(...) \
            INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_START_DEBUG, __VA_ARGS__ )
    #else
        #define ssLOG_BENCH_START_DEBUG(...) INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__)
    #endif

    #define INTERNAL_ssLOG_BENCH_START_DEBUG_0() \
        INTERNAL_ssLOG_BENCH_START_DEBUG_1("")

    #define INTERNAL_ssLOG_BENCH_START_DEBUG_1(benchName) \
        INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(benchName); \
        INTERNAL_ssLOG_BENCH_START_INNER_PRINT_BENCH_LEVELED(benchName, ssLOG_LEVEL_DEBUG)

    #if !ssLOG_CALL_STACK_ONLY
        #define ssLOG_BENCH_END_DEBUG(...) \
            do{ INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_BENCH_END_DEBUG, __VA_ARGS__ ); } while(0)
    #else
        #define ssLOG_BENCH_END_DEBUG(...) do{} while(false)
    #endif

    #define INTERNAL_ssLOG_BENCH_END_DEBUG_0() \
        static_assert(false, "ssLOG_BENCH_END_DEBUG must accept 1 argument")

    #define INTERNAL_ssLOG_BENCH_END_DEBUG_1(startVar) \
        Interna_ssLogBenchEnd(  startVar, \
                                INTERNAL_ssLOG_GET_FUNCTION_NAME_0(), \
                                INTERNAL_ssLOG_GET_FILE_NAME(), \
                                INTERNAL_ssLOG_GET_LINE_NUM(), \
                                ssLOG_LEVEL_DEBUG)
#else
    #define ssLOG_DEBUG(...) do{}while(0)
    #define ssLOG_FUNC_CONTENT_DEBUG(...) \
        INTERNAL_ssLOG_VA_SELECT( INTERNAL_ssLOG_EXECUTE_COMMAND, __VA_ARGS__ )
    
    #define ssLOG_FUNC_ENTRY_DEBUG(...) do{}while(0)
    #define ssLOG_FUNC_EXIT_DEBUG(...) do{}while(0)
    #define ssLOG_FUNC_DEBUG(...) do{}while(0)
    #define ssLOG_BENCH_START_DEBUG(...) INTERNAL_ssLOG_BENCH_START_INNER_CREATE_BENCH(__VA_ARGS__)
    #define ssLOG_BENCH_END_DEBUG(...) do{}while(0)
#endif //#if !ssLOG_DISABLE_LOGS && ssLOG_LEVEL >= ssLOG_LEVEL_DEBUG

#endif //#ifndef ssLOG_LEVELS_HPP
