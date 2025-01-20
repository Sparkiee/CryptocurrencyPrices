cmake_minimum_required(VERSION 3.28.1)
project(CryptoDashboard)

set(CMAKE_CXX_STANDARD 20)

# Find packages
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

# Add ImGui source files
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/libs/imgui)
set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Add executable
add_executable(CryptoDashboard
        main.cpp
        ${IMGUI_SOURCES}
)

# Include directories
target_include_directories(CryptoDashboard PRIVATE
        ${CMAKE_SOURCE_DIR}/libs
        ${CMAKE_SOURCE_DIR}/libs/imgui
        ${CMAKE_SOURCE_DIR}/libs/imgui/backends
        ${OPENGL_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(CryptoDashboard PRIVATE
        OpenGL::GL
        glfw
)