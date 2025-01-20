cmake_minimum_required(VERSION 3.28.1)
project(CryptoCurrencyPrices)

set(CMAKE_CXX_STANDARD 20)

# Add the 'libraries' folder to the include path
include_directories(${PROJECT_SOURCE_DIR}/libraries)

add_executable(CryptoCurrencyPrices main.cpp)

if (APPLE)
    target_link_libraries(CryptoCurrencyPrices PRIVATE
            )
elseif (WIN32)
    target_link_libraries(CryptoCurrencyPrices PRIVATE ws2_32)
endif()

