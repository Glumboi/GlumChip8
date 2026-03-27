# Define a library for raymoblib
set(CHIP8_CORE_DIR ${CMAKE_SOURCE_DIR}/deps/GlumChip8_C.Core)

add_library(GlumChip8_CCoreLib STATIC ${CHIP8_CORE_DIR}/Chip8CPU.c ${CHIP8_CORE_DIR}/Chip8Display.c ${CHIP8_CORE_DIR}/Chip8Keyboard.c ${CHIP8_CORE_DIR}/Chip8RAM.c ${CHIP8_CORE_DIR}/Chip8System.c)

# Include headers directory for android_native_app_glue.c
include_directories(${ANDROID_NDK}/sources/android/native_app_glue/)
include_directories(${CHIP8_CORE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/deps/raymob)


# Add android_native_app_glue.c to the source files
list(APPEND SOURCES ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

# Define compiler macros for the library
target_compile_definitions(raymoblib PRIVATE PLATFORM_ANDROID)

# Include raylib header files
target_include_directories(raymoblib PRIVATE "${CMAKE_SOURCE_DIR}/deps/raylib")

# Link required libraries to raylib
target_link_libraries(GlumChip8_CCoreLib raylib)
