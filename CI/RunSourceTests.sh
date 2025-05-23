set -e

if [ -e ../Build/libssLogger_SRC.so ]; then
    cp -f ../Build/libssLogger_SRC.so ../Build/SourceTests
fi

../Build/SourceTests/LogFunctionTestSource
sleep 1
../Build/SourceTests/LogLevelTestSource
sleep 1
../Build/SourceTests/LogLineTestSource
sleep 1
../Build/SourceTests/LogMultiThreadTestSource
