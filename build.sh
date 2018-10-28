#!/bin/bash

BUILD_CLEAN=false
BUILD_TYPE="Debug"
CMAKE_PARAMS=""
MAKE_PARAMS=""

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
            MAKE_PARAMS="$MAKE_PARAMS -j$2"
            shift
            ;;
        -v|--verbose)
            MAKE_PARAMS="$MAKE_PARAMS VERBOSE=1"
            ;;
        *)
            echo "Error: Do not recognise argument $1."
            exit 1
            ;;
    esac
    shift
done

if [ "$BUILD_TYPE" = "Debug" ]; then
    CMAKE_PARAMS="$CMAKE_PARAMS -DCMAKE_BUILD_TYPE=Debug"
else
    CMAKE_PARAMS="$CMAKE_PARAMS -DCMAKE_BUILD_TYPE=Release"
fi

if [ -d "build"  ] ; then
    if [ "$BUILD_CLEAN" = true ] ; then
        rm -rf build/*
    fi
else
    mkdir build
fi

printf -- "\n    /------------------------\\ \n    |  Generating Build Env  |\n    \\------------------------/\n\n\n"

eval "cmake -H. -Bbuild $CMAKE_PARAMS -Wno-deprecated"

printf -- "\n\n    /------------------\\ \n    |     Building     |\n    \\------------------/\n\n\n"

eval "cmake --build build -- $MAKE_PARAMS"
