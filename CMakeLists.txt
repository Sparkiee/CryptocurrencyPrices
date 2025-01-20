cmake_minimum_required(VERSION 3.28.1)
project(CryptoCurrencyPrices)

set(CMAKE_CXX_STANDARD 20)

# Add the 'libraries' folder to the include path
include_directories(${PROJECT_SOURCE_DIR}/libraries)

add_executable(CryptoCurrencyPrices main.cpp)

# Add the necessary Winsock library for networking
target_link_libraries(CryptoCurrencyPrices ws2_32)
