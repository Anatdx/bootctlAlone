cmake_minimum_required(VERSION 3.20)

project(bootctl_alone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# bootctl uses Android property (and optionally HAL). Meaningful on Android (NDK) only.
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
  add_executable(bootctl src/bootctl_main.cpp)
  target_compile_definitions(bootctl PRIVATE BOOTCTL_ANDROID=1)
  target_include_directories(bootctl PRIVATE src)
  target_link_libraries(bootctl PRIVATE log)
else()
  add_executable(bootctl src/bootctl_main.cpp)
  target_compile_definitions(bootctl PRIVATE BOOTCTL_STANDALONE=1)
  target_include_directories(bootctl PRIVATE src)
endif()
