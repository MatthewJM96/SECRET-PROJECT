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
hunter_add_package(SDL_ttf)
find_package(SDL_ttf CONFIG REQUIRED)
hunter_add_package(PNG)
find_package(PNG CONFIG REQUIRED)

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

#    if (${TARGET_X64})
#        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
#        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
#    else()
#        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
#        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
#    endif()
# Set up flags for MSVC.
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # Warn on (nearly) everything, treat warnings as errors, use the most portable exception handling model.
    set(CMAKE_CXX_FLAGS "/W4 /WX /wd4201 /EHsc")

    # Set optimisation flags for release and debug modes.
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
    set(CMAKE_CXX_FLAGS_DEBUG "/Od /Zi")
endif()

# Include custom modules we need.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Add directories for compiler to search for included files.
include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/deps/include")

# Define the headers and sources created for the project.
set(SP_include
    include/stdafx.h
    include/types.h
)

set(SP_src
    src/main.cpp
)

set(SP_graphics_include
    include/graphics/Clipping.hpp
    include/graphics/Font.h
    include/graphics/GLSLProgram.h
    include/graphics/Gradients.hpp
    include/graphics/SpriteBatcher.h
    include/graphics/TextAlign.h
    include/graphics/WordWrap.hpp
)

set(SP_graphics_src
    src/graphics/Font.cpp
    src/graphics/GLSLProgram.cpp
    src/graphics/SpriteBatcher.cpp
    src/graphics/TextAlign.cpp
)

set(SP_io_include
    include/io/FileLoader.h
    include/io/ImageIO.h
)
set(SP_io_src
    src/io/FileLoader.cpp
    src/io/ImageIO.cpp
)

# As we make them, create groupings by namespace - e.g. graphics, IO, UI to improve Visual Studio project file creation.
source_group("include" FILES ${SP_include})
source_group("src" FILES ${SP_src})
source_group("include/graphics" FILES ${SP_graphics_include})
source_group("src/graphics" FILES ${SP_graphics_src})
source_group("include/io" FILES ${SP_io_include})
source_group("src/io" FILES ${SP_io_src})

# Add an executable to be compiled and linked.
add_executable(SECRET_PROJECT
    ${SP_src}
    ${SP_graphics_src}
    ${SP_io_src}
)

# Target the libraries we want to link.
target_link_libraries(
    SECRET_PROJECT
    SDL2::SDL2main
    SDL2::SDL2
    SDL_ttf::SDL_ttf
    glew::glew
    glm
    PNG::png
)

# Create launchers for the target.
include(CreateLaunchers)
create_target_launcher(SECRET_PROJECT
    RUNTIME_LIBRARY_DIRS "${CMAKE_BINARY_DIR}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data"
)
