cmake_minimum_required(VERSION 3.0)

cmake_policy(SET CMP0053 NEW)

# Make sure we aren't cluttering the source folder.
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Don't run cmake inside the source folder!\nMake a 'build' folder and run from there.")
endif()

# Project name.
project(ATextAdventure)

# Set up compiler environment
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set up flags for g++ and clang compilers.
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # Specify options that can be modified on each build.
    option(TARGET_X64 "Should we target 64-bit architecture? If not we taget 32-bit." Off)
    option(USING_GDB "Are we using gdb to debug?" On)
    option(EXTRA_DEBUG "Should we add extra debug symbols?" On)
    option(OPTIMISE_ON_DEBUG "Should we optimise a debug target?" On)

    # Add a whole bunch of warnings and specify the standard to compile against.
    # Further, ensure that warnings are treated as errors.
    set(CMAKE_C_FLAGS "-Wall -Wextra -Werror -Wshadow -Wformat=2 -std=c17")
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wold-style-cast -Wshadow -Wformat=2 -std=c++17")
    # Add some warnings that are only supported by one of g++ or clang.
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wduplicated-cond")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wduplicated-cond -Wuseless-cast")
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wused-but-marked-unused")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wused-but-marked-unused")
    endif()

    # Set the appropriate optimisation and debug symbol flags depending on options chosen and if we are in release or debug mode.
    set(CMAKE_C_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    if (${USING_GDB})
        if (${EXTRA_DEBUG})
            set(CMAKE_C_FLAGS_DEBUG "-ggdb3")
            set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3")
        else()
            set(CMAKE_C_FLAGS_DEBUG "-ggdb")
            set(CMAKE_CXX_FLAGS_DEBUG "-ggdb")
        endif()
    else()
        if (${EXTRA_DEBUG})
            set(CMAKE_C_FLAGS_DEBUG "-g3")
            set(CMAKE_CXX_FLAGS_DEBUG "-g3")
        else()
            set(CMAKE_C_FLAGS_DEBUG "-g")
            set(CMAKE_CXX_FLAGS_DEBUG "-g")
        endif()
    endif ()
    if (${OPTIMISE_ON_DEBUG})
        # This is supported by both Clang and GCC as long as we are up-to-date.
        # Should be fine going back as far as Ubuntu 17.10, and deffo fine on Arch.
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Og")
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og")
    endif()

    if (${TARGET_X64})
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    else()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
    endif()

# Set up flags for MSVC.
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # Warn on (nearly) everything, treat warnings as errors, use the most portable exception handling model.
    set(CMAKE_CXX_FLAGS "/W4 /WX /EHsc")

    # Set optimisation flags for release and debug modes.
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od")
endif()

# Include custom modules we need.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Add directories for compiler to search for included files.
include_directories("${CMAKE_SOURCE_DIR}/include")

# Define the headers and sources created for the project.
set(ATA_headers
)

set(ATA_sources
    src/main.cpp
)

# Add an executable to be compiled and linked.
add_executable(ATextAdventure
    ${ATA_headers}
    ${ATA_sources}
)

# Create launchers for the target.
include(CreateLaunchers)
create_target_launcher(ATextAdventure
    RUNTIME_LIBRARY_DIRS "${CMAKE_BINARY_DIR}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data"
)