set -e

if [ -e ../Build/libssLogger_SRC.so ]; then
    cp -f ../Build/libssLogger_SRC.so ../Build/SourceTests
fi

../Build/SourceTests/LogFunctionTestSource
../Build/SourceTests/LogLevelTestSource
../Build/SourceTests/LogLineTestSource
../Build/SourceTests/LogMultiThreadTestSource
../Build/HeaderOnlyTests/LogFunctionTestHeader_Only
../Build/HeaderOnlyTests/LogLevelTestHeader_Only
../Build/HeaderOnlyTests/LogLineTestHeader_Only
../Build/HeaderOnlyTests/LogMultiThreadTestHeader_Only
