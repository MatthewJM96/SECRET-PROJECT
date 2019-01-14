:: Disable command echoing.
@ECHO OFF

:: Set current working directory to the project directory.
PUSHD "%~dp0"

:: Make sure we don't polute global environment.
SETLOCAL

:: Some defaults we will override with settings chosen by caller.
SET "BUILD_CLEAN=false"
SET "BUILD_TYPE=Debug"
SET "BUILD_TARGET=x86"

:: The parameter lists for CMAKE and MAKE (or equivalent build tool).
SET "CMAKE_PARAMS="
SET "PRE_BUILD_PARAMS="
SET "BUILD_PARAMS="

:: Jump to the loop to process parameters passed to this script.
GOTO ParamLoop

:: Define a bunch of helper functions for parameter processing loop.
:Help
ECHO "\n"
ECHO "    /--------------\\ \n    |  Build Help  |\n    \\--------------/\n\n"
ECHO "        Build flags:\n"
ECHO "            --clean             | -c       ---   Clean build, removes all previous artefacts.\n"
ECHO "        CMake flags:\n"
ECHO "            --release           | -r       ---   Compile in release mode.\n"
ECHO "            --debug             | -d       ---   Compile in debug mode.\n"
ECHO "            --x64               | -64      ---   Compile for x64 architecture.\n"
ECHO "        Make flags:\n"
ECHO "            --verbose           | -v       ---   Run make with verbose set on.\n"
ECHO "\n"
GOTO ParamLoopContinue

:Clean
SET "BUILD_CLEAN=true"
GOTO ParamLoopContinue

:Release
SET "BUILD_TYPE=Release"
GOTO ParamLoopContinue

:Debug
SET "BUILD_TYPE=Debug"
GOTO ParamLoopContinue

:X64
SET "BUILD_TARGET=x86_64"
GOTO ParamLoopContinue

:Verbose
SET "BUILD_PARAMS=%BUILD_PARAMS% VERBOSE=1"
GOTO ParamLoopContinue

:: Loop over each parameter passed to this script, and appropriately update the earlier-defined variables.
:ParamLoop
    IF "%1"=="-h" (
        GOTO Help
    ) ELSE IF "%1"=="--help" (
        GOTO Help
    ) ELSE IF "%1"=="-c" (
        GOTO Clean
    ) ELSE IF "%1"=="--clean" (
        GOTO Clean
    ) ELSE IF "%1"=="-r" (
        GOTO Release
    ) ELSE IF "%1"=="--release" (
        GOTO Release
    ) ELSE IF "%1"=="-d" (
        GOTO Debug
    ) ELSE IF "%1"=="--debug" (
        GOTO Debug
    ) ELSE IF "%1"=="-64" (
        GOTO X64
    ) ELSE IF "%1"=="--x64" (
        GOTO X64
    ) ELSE IF "%1"=="-v" (
        GOTO Verbose
    ) ELSE IF "%1"=="--verbose" (
        GOTO Verbose
    ) ELSE IF NOT "%1"=="" (
        ECHO "Error: Do not recognise argument %1."
        EXIT /B 1
    ) ELSE (
        GOTO ParamLoopBreak
    )
    :ParamLoopContinue
    SHIFT
GOTO ParamLoop
:ParamLoopBreak

:: Set correct build type after processing parameters to this script.
IF "%BUILD_TYPE%"=="Debug" (
    SET "CMAKE_PARAMS=%CMAKE_PARAMS% -DCMAKE_BUILD_TYPE=Debug"
    SET "PRE_BUILD_PARAMS=%PRE_BUILD_PARAMS% --config Debug"
) ELSE (
    SET "CMAKE_PARAMS=%CMAKE_PARAMS% -DCMAKE_BUILD_TYPE=Release"
    SET "PRE_BUILD_PARAMS=%PRE_BUILD_PARAMS% --config Release"
)

:: Choose generator based on which architecture we are targetting.
IF "%BUILD_TARGET%"=="x86" (
    SET 'CMAKE_PARAMS=%CMAKE_PARAMS% -G "Visual Studio 14 2017"'
) ELSE (
    SET 'CMAKE_PARAMS=%CMAKE_PARAMS% -G "Visual Studio 14 2017 Win64"'
)

:: Define build directory based on which architecture we are targetting.
SET "BUILD=build_%BUILD_TARGET%"

:: If build directory exists and we want a clean build, delete all the contents of the build directory.
:: In any case, if it doesn't exist, create it.
IF EXIST "%BUILD%" (
    IF "%BUILD_CLEAN%"=="true" (
        RMDIR %BUILD% /S /Q
    )
) ELSE (
    MKDIR %BUILD%
)

:: Construct and call cmake command to generate build configuration.
SET "CMAKE_COMMAND=cmake -H. -B%BUILD% %CMAKE_PARAMS% -Wno-dev"
%CMAKE_COMMAND%

:: Construct and call build command.
SET "BUILD_COMMAND=cmake --build %BUILD% %PRE_BUILD_PARAMS% -- %BUILD_PARAMS%"
%BUILD_COMMAND%

:: Return to terminal's previous working directory.
POPD
