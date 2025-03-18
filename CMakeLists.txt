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
    include_directories(SYSTEM "/opt/homebrew/Cellar/curlpp/0.8.1_2/include")
    # Add compiler flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

# Find required packages with static preference
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-3.0)
pkg_check_modules(CURLPP REQUIRED IMPORTED_TARGET curlpp)
pkg_check_modules(JSONCPP REQUIRED IMPORTED_TARGET jsoncpp)

# Include directories
include_directories(
    ${GTKMM_INCLUDE_DIRS}
    ${CURLPP_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
)

# Link directories
link_directories(
    ${GTKMM_LIBRARY_DIRS}
    ${CURLPP_LIBRARY_DIRS}
    ${JSONCPP_LIBRARY_DIRS}
)

# Add compile options
add_compile_options(
    ${GTKMM_CFLAGS_OTHER}
    ${CURLPP_CFLAGS_OTHER}
    ${JSONCPP_CFLAGS_OTHER}
)

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
)

# Add executable
add_executable(gtkks ${SOURCES})

# Link libraries - different for macOS and Linux
if(APPLE)
    # macOS uses dynamic linking with library bundling
    target_link_libraries(gtkks
        ${GTKMM_LIBRARIES}
        ${CURLPP_LIBRARIES}
        ${JSONCPP_LIBRARIES}
    )
else()
    # Linux can use more static linking
    target_link_libraries(gtkks
        -Wl,-Bstatic
        ${GTKMM_STATIC_LIBRARIES}
        ${CURLPP_STATIC_LIBRARIES}
        ${JSONCPP_STATIC_LIBRARIES}
        -Wl,-Bdynamic
    )
endif()

# Install target
install(TARGETS gtkks DESTINATION bin)

# Print configuration
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "GTKMM include dirs: ${GTKMM_INCLUDE_DIRS}")
message(STATUS "GTKMM libraries: ${GTKMM_LIBRARIES}")
message(STATUS "GTKMM static libraries: ${GTKMM_STATIC_LIBRARIES}")
message(STATUS "CURLPP include dirs: ${CURLPP_INCLUDE_DIRS}")
message(STATUS "CURLPP libraries: ${CURLPP_LIBRARIES}")
message(STATUS "JSONCPP include dirs: ${JSONCPP_INCLUDE_DIRS}")
message(STATUS "JSONCPP libraries: ${JSONCPP_LIBRARIES}") 