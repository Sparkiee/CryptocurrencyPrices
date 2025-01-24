cmake_minimum_required(VERSION 3.28.1)
project(CryptoCurrencyPrices)

set(CMAKE_CXX_STANDARD 20)

# Set the path to GLFW source if not using package managers
set(GLFW_SOURCE_DIR "${PROJECT_SOURCE_DIR}/libraries/glfw")
set(GLFW_BUILD_DIR "${GLFW_SOURCE_DIR}/build")

# Add the GLFW include directory
include_directories(
        ${PROJECT_SOURCE_DIR}/libraries
        ${PROJECT_SOURCE_DIR}/libraries/imgui
        ${GLFW_SOURCE_DIR}/include
)

# Add GLFW as a subdirectory to handle its build
add_subdirectory(${GLFW_SOURCE_DIR} ${GLFW_BUILD_DIR})

add_subdirectory(libraries/fmt)


# Find OpenGL package
find_package(OpenGL REQUIRED)

# ImGui source files
set(IMGUI_SOURCES
        libraries/imgui/imgui.cpp
        libraries/imgui/imgui_demo.cpp
        libraries/imgui/imgui_draw.cpp
        libraries/imgui/imgui_tables.cpp
        libraries/imgui/imgui_widgets.cpp
        libraries/imgui/backends/imgui_impl_glfw.cpp
        libraries/imgui/backends/imgui_impl_opengl3.cpp
        )

# Add source files
add_executable(CryptoCurrencyPrices
        main.cpp
        CryptoClient.cpp
        ${IMGUI_SOURCES}
        CryptoData.cpp
        )

# Link libraries
target_link_libraries(CryptoCurrencyPrices
        PRIVATE
        glfw
        OpenGL::GL   # For Unix-like systems, this should work
        fmt::fmt
        )

# Add macOS-specific frameworks
if(APPLE)
    target_link_libraries(CryptoCurrencyPrices PRIVATE
            "-framework Cocoa"
            "-framework IOKit"
            "-framework QuartzCore"
            )
endif()

# On Windows, if OpenGL::GL is not found, link manually
if(WIN32)
    target_link_libraries(CryptoCurrencyPrices PRIVATE ws2_32 opengl32)
endif()

# Set the output directory
set_target_properties(CryptoCurrencyPrices PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )
