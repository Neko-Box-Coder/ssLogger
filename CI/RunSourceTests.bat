@ECHO OFF

SETLOCAL ENABLEEXTENSIONS

GOTO :FINAL

:RUN_TEST <testFile>
    @REM Setlocal EnableDelayedExpansion
    PUSHD "%~dp1"
    CALL "%~1"
    IF NOT %errorlevel% == 0 (
        ECHO "Failed: %errorlevel%"
        GOTO :FAILED
    )
    POPD
    EXIT /b

:FINAL
CALL :RUN_TEST "%~dp0\..\Build\SourceTests\Debug\LogFunctionTestSource.exe"
timeout /t 3
CALL :RUN_TEST "%~dp0\..\Build\SourceTests\Debug\LogLevelTestSource.exe"
timeout /t 3
CALL :RUN_TEST "%~dp0\..\Build\SourceTests\Debug\LogLineTestSource.exe"
timeout /t 3
CALL :RUN_TEST "%~dp0\..\Build\SourceTests\Debug\LogMultiThreadTestSource.exe"

EXIT 0

:FAILED
EXIT 1
