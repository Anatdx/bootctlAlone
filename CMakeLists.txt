cmake_minimum_required(VERSION 3.14)
project(bootctlAlone LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(bootctl src/main.cpp)
target_compile_definitions(bootctl PRIVATE BOOTCTL_STANDALONE=1)
install(TARGETS bootctl DESTINATION bin)
