cmake_minimum_required(VERSION 3.10)
project(gtkks VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable static linking
if(NOT APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
else()
    # macOS doesn't support fully static linking
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-search_paths_first")
endif()

set(BUILD_SHARED_LIBS OFF)

# Set compiler flags for macOS
if(APPLE)
    # Use clang++ explicitly on macOS
    set(CMAKE_CXX_COMPILER "clang++")
    # Add macOS SDK path to include directories
    include_directories(SYSTEM "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include")
    include_directories(SYSTEM "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/c++/v1")
    # Add homebrew include directories
    include_directories(SYSTEM "/opt/homebrew/include")
    # Add compiler flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-2.4)

# Print library information for debugging
message(STATUS "GTKMM_LIBRARIES: ${GTKMM_LIBRARIES}")
message(STATUS "GTKMM_LIBRARY_DIRS: ${GTKMM_LIBRARY_DIRS}")
message(STATUS "GTKMM_INCLUDE_DIRS: ${GTKMM_INCLUDE_DIRS}")

# Create a list of all possible config include directories
set(POSSIBLE_CONFIG_DIRS
    # x86_64 paths
    /usr/lib/x86_64-linux-gnu/atkmm-1.6/include
    /usr/lib/x86_64-linux-gnu/gdkmm-2.4/include
    /usr/lib/x86_64-linux-gnu/gtk-2.0/include
    /usr/lib/x86_64-linux-gnu/gtkmm-2.4/include
    # ARM paths
    /usr/lib/arm-linux-gnueabihf/atkmm-1.6/include
    /usr/lib/arm-linux-gnueabihf/gdkmm-2.4/include
    /usr/lib/arm-linux-gnueabihf/gtk-2.0/include
    /usr/lib/arm-linux-gnueabihf/gtkmm-2.4/include
    # More standard paths
    /usr/include/atkmm-1.6
    /usr/include/gdkmm-2.4
    /usr/include/gtkmm-2.4
)

# Find all *config.h files in the system
foreach(dir ${POSSIBLE_CONFIG_DIRS})
    if(EXISTS ${dir})
        message(STATUS "Including directory: ${dir}")
        include_directories(${dir})
    endif()
endforeach()

# Add the project include directories
include_directories(
    ${GTKMM_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
)

# Link directories
link_directories(
    ${GTKMM_LIBRARY_DIRS}
)

# Add compile options
add_compile_options(
    ${GTKMM_CFLAGS_OTHER}
)

# Define a macro to let code know we're using SimpleJson instead of jsoncpp
add_definitions(-DUSE_SIMPLE_JSON)

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/ChatView.cpp
    src/ModelSelector.cpp
    src/ApiManager.cpp
    src/Config.cpp
    src/LLMApi.cpp
    src/OllamaApi.cpp
    src/OpenAIApi.cpp
    src/GeminiApi.cpp
    src/DeepseekApi.cpp
    src/OpenRouterApi.cpp
    src/HttpClient.cpp
)

# Add executable
add_executable(gtkks ${SOURCES})

# Link libraries
target_link_libraries(gtkks
    ${GTKMM_LIBRARIES}
)

# Install target
install(TARGETS gtkks DESTINATION bin)

# Print configuration
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "GTKMM libraries: ${GTKMM_LIBRARIES}") 