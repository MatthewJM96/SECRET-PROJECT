cmake_minimum_required(VERSION 3.1)

cmake_policy(SET CMP0053 NEW)
cmake_policy(SET CMP0054 NEW)

# Make sure we aren't cluttering the source folder.
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Don't run cmake inside the source folder!\nMake a 'build' folder and run from there.")
endif()


include(${CMAKE_CURRENT_LIST_DIR}/cmake/hunter/HunterGate.cmake)

HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.57.tar.gz"
    SHA1 "8befbbaea51cb54681f6e690212ed753ae807529"
)

# Project name.
project(SECRET_PROJECT)

# Add depedencies
hunter_add_package(glm)
find_package(glm CONFIG REQUIRED)
hunter_add_package(glew)
find_package(glew CONFIG REQUIRED)
hunter_add_package(SDL2)
find_package(SDL2 CONFIG REQUIRED)

# Set up compiler environment
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    option(TARGET_X64 "Should we target 64-bit architecture? If not we taget 32-bit." Off)
    option(USING_GDB "Are we using gdb to debug?" On)
    option(EXTRA_DEBUG "Should we add extra debug symbols?" On)
    option(OPTIMISE_ON_DEBUG "Should we optimise a debug target?" On)

    set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")

    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    if (${USING_GDB})
        if (${EXTRA_DEBUG})
            set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3")
        else()
            set(CMAKE_CXX_FLAGS_DEBUG "-ggdb")
        endif()
    else()
        if (${EXTRA_DEBUG})
            set(CMAKE_CXX_FLAGS_DEBUG "-g3")
        else()
            set(CMAKE_CXX_FLAGS_DEBUG "-g")
        endif()
    endif ()
    if (${OPTIMISE_ON_DEBUG})
        # This is supported by both Clang and GCC as long as we are up-to-date.
        # Should be fine going back as far as Ubuntu 17.10, and deffo fine on Arch.
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
    set(CMAKE_CXX_FLAGS "/Wall /WX /EHsc")

    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od")
endif()

# Include custom modules we need.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Add directories for compiler to search for included files.
include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/deps/include")

# Define the headers and sources created for the project.
set(SP_headers
)

set(SP_sources
    src/main.cpp
)

# As we make them, create groupings by namespace - e.g. graphics, IO, UI.
# Use source_group for these to improve Visual Studio project file creation.

# Add an executable to be compiled and linked.
add_executable(SECRET_PROJECT
    ${SP_headers}
    ${SP_sources}
)

# Target the libraries we want to link.
target_link_libraries(
    SECRET_PROJECT
    SDL2::SDL2main
    SDL2::SDL2
    glew::glew
    glm
)

include(CreateLaunchers)

# Create launchers for the target.
create_target_launcher(SECRET_PROJECT
    RUNTIME_LIBRARY_DIRS "${CMAKE_BINARY_DIR}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data"
)
