cmake_minimum_required(VERSION 3.26)
project(Emu_GameBoy)

set(CMAKE_CXX_STANDARD 23)
set(PROJECT_VERSION 1.0.0)

set(SOURCE_FILES
        hardware/Bus.cpp
        logging/Logger.cpp
        hardware/Screen.cpp
        main.cpp
)

set(HEADER_FILES
        hardware/Bus.h
        logging/Logger.h
        hardware/Screen.h
)

find_package(glfw3 REQUIRED)

add_compile_options(-Wall -Wextra)
add_executable(Emu_GameBoy ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(Emu_GameBoy glfw)
