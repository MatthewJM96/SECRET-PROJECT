#!/bin/bash

# Some defaults we will override with settings chosen by caller.
BUILD_CLEAN=false
BUILD_TYPE="Debug"

# The parameter lists for CMAKE and the build tool of the system.
CMAKE_PARAMS=""
BUILD_PARAMS=""

# Loop over each parameter passed to this script, and appropriately update the above variables.
while test $# -gt 0
do
    case "$1" in
        -h|--help)
            printf -- "\n"
            printf -- "    /--------------\\ \n    |  Build Help  |\n    \\--------------/\n\n"
            printf -- "        Build flags:\n"
            printf -- "            --clean             | -c       ---   Clean build, removes all previous artefacts.\n"
            printf -- "        CMake flags:\n"
            printf -- "            --release           | -r       ---   Compile in release mode.\n"
            printf -- "            --debug             | -d       ---   Compile in debug mode.\n"
            printf -- "            --x64               | -64      ---   Compile for x64 architecture.\n"
            printf -- "            --clang             | -cl      ---   Compiles using clang rather than gcc.\n"
            printf -- "            --no-gdb            | -ng      ---   Add OS specific debug symbols rather than GDB's.\n"
            printf -- "            --no-extra-debug    | -ned     ---   Don't add extra debug symbols.\n"
            printf -- "            --no-optimise-debug | -nod     ---   Don't optimise debug mode builds.\n"
            printf -- "        Make flags:\n"
            printf -- "            --jobs X            | -j X     ---   Run compilation on X number of threads.\n"
            printf -- "            --verbose           | -v       ---   Run make with verbose set on.\n"
            printf -- "\n"
            exit 0
            ;;
        -c|--clean)
            BUILD_CLEAN=true
            ;;
        -r|--release)
            BUILD_TYPE="Release"
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
            ;;
        -64|--x64)
            CMAKE_PARAMS="$CMAKE_PARAMS -DTARGET_X64=On"
        -cl|--clang)
            CMAKE_PARAMS="$CMAKE_PARAMS -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -DCMAKE_C_COMPILER=/usr/bin/clang"
            ;;
        -ng|--no-gdb)
            CMAKE_PARAMS="$CMAKE_PARAMS -DUSING_GDB=Off"
            ;;
        -ned|--no-extra-debug)
            CMAKE_PARAMS="$CMAKE_PARAMS -DEXTRA_DEBUG=Off"
            ;;
        -nod|--no-optimise-debug)
            CMAKE_PARAMS="$CMAKE_PARAMS -DOPTIMISE_ON_DEBUG=Off"
            ;;
        -j|--jobs)
            if ! [[ $2 =~ ^[0-9]+$ ]] ; then
                echo "Error: Saw argument --jobs (-j) but it was not followed by a number of jobs to run."
                exit 1
            fi
            BUILD_PARAMS="$BUILD_PARAMS -j$2"
            shift
            ;;
        -v|--verbose)
            BUILD_PARAMS="$BUILD_PARAMS VERBOSE=1"
            ;;
        *)
            echo "Error: argument $1 not recognised."
            exit 1
            ;;
    esac
    shift
done

# Set correct build type after processing parameters to this script.
if [ "$BUILD_TYPE" = "Debug" ]; then
    CMAKE_PARAMS="$CMAKE_PARAMS -DCMAKE_BUILD_TYPE=Debug"
else
    CMAKE_PARAMS="$CMAKE_PARAMS -DCMAKE_BUILD_TYPE=Release"
fi

# If build directory exists and we want a clean build, delete all the contents of the build directory.
# In any case, if it doesn't exist, create it.
if [ -d "build"  ] ; then
    if [ "$BUILD_CLEAN" = true ] ; then
        rm -rf build/*
    fi
else
    mkdir build
fi

printf -- "\n    /----------------------------------\\ \n    |  Generating Build Configuration  |\n    \\----------------------------------/\n\n\n"

# Generate the build configuration. 
eval "cmake -H. -Bbuild $CMAKE_PARAMS -Wno-deprecated"

printf -- "\n\n    /------------------\\ \n    |     Building     |\n    \\------------------/\n\n\n"

# Build application using build configuration.
eval "cmake --build build -- $BUILD_PARAMS"
