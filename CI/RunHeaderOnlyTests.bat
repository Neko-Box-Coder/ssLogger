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
CALL :RUN_TEST "%~dp0\..\Build\HeaderOnlyTests\Debug\LogFunctionTestHeader_Only.exe"
timeout /t 3
CALL :RUN_TEST "%~dp0\..\Build\HeaderOnlyTests\Debug\LogLevelTestHeader_Only.exe"
timeout /t 3
CALL :RUN_TEST "%~dp0\..\Build\HeaderOnlyTests\Debug\LogLineTestHeader_Only.exe"
timeout /t 3
CALL :RUN_TEST "%~dp0\..\Build\HeaderOnlyTests\Debug\LogMultiThreadTestHeader_Only.exe"

EXIT 0

:FAILED
EXIT 1
