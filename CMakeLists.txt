cmake_minimum_required(VERSION 3.16)

project(cpp_limit_order_book LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

add_library(orderbook
    src/OrderBook.cpp
)

target_include_directories(orderbook
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(orderbook_cli
    src/main.cpp
    src/Cli.cpp
)

target_link_libraries(orderbook_cli
    PRIVATE
        orderbook
)

add_executable(orderbook_tests
    tests/OrderBookTest.cpp
)

target_link_libraries(orderbook_tests
    PRIVATE
        orderbook
)

add_test(NAME orderbook_tests COMMAND orderbook_tests)