cmake_minimum_required(VERSION 3.16)

project(cpp_limit_order_book LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

add_library(orderbook
    src/OrderBook.cpp
    src/Exchange.cpp
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

add_executable(exchange_tests
    tests/ExchangeTest.cpp
)

target_link_libraries(exchange_tests
    PRIVATE
        orderbook
)

add_test(NAME exchange_tests COMMAND exchange_tests)