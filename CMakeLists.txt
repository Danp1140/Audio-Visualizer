cmake_minimum_required(VERSION 3.13)
project(Audio_Visualizer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -framework CoreAudio -framework AudioToolbox -framework AudioUnit -framework CoreServices -framework Carbon")

include_directories(include)
link_directories(libs)

add_executable(Audio_Visualizer src/main.cpp
        src/Viewport.cpp
        src/Viewport.h
        src/Drawable2D.cpp
        src/Drawable2D.h)

target_link_libraries(Audio_Visualizer GLEW glfw3 portaudio -m64)