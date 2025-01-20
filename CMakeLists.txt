cmake_minimum_required(VERSION 3.28.1)
project(CryptoCurrencyPrices)

set(CMAKE_CXX_STANDARD 20)

# Find required packages
find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# Add the 'libraries' folder to the include path
include_directories(
        ${PROJECT_SOURCE_DIR}/libraries
        ${PROJECT_SOURCE_DIR}/libraries/imgui
)

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
        CryptoData.cpp
        CryptoClient.cpp
        ${IMGUI_SOURCES}
)

# Link libraries
target_link_libraries(CryptoCurrencyPrices
        PRIVATE
        glfw
        OpenGL::GL
        ws2_32
)

# Set the output directory
set_target_properties(CryptoCurrencyPrices PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)