# CMake toolchain file for Kindle cross-compilation

# System information
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Specify the cross compiler
set(CMAKE_C_COMPILER arm-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabi-g++)

# Where to look for the target environment
set(CMAKE_FIND_ROOT_PATH /usr/arm-linux-gnueabi)

# Search for programs only in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers only in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Kindle-specific flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -march=armv7-a -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -march=armv7-a -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp")

# Set pkg-config for cross-compilation
set(ENV{PKG_CONFIG_PATH} "/opt/kindle-tc/arm-kindle-linux-gnueabi/sysroot/usr/lib/pkgconfig")
set(ENV{PKG_CONFIG_LIBDIR} "/opt/kindle-tc/arm-kindle-linux-gnueabi/sysroot/usr/lib/pkgconfig")
set(ENV{PKG_CONFIG_SYSROOT_DIR} "/opt/kindle-tc/arm-kindle-linux-gnueabi/sysroot") 