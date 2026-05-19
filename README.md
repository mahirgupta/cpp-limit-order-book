# C++ Limit Order Book - V2

Single-symbol limit order book and CLI written in C++17.

V2 refactors the original prototype into a small CMake project with a core order-book library, CLI executable, and regression tests.

## Features

- Buy and sell limit orders
- Price-time priority matching
- Resting-order-price trade execution
- Full and partial fills
- Multi-level sweeps
- Cancel active orders by order id
- Trade history
- Best bid, best ask, and spread
- Clear/reset command
- Core order validation
- CTest regression suite

## Project Layout

```text
include/
  Cli/
    Cli.hpp
  orderBook/
    Order.hpp
    OrderBook.hpp
    Trade.hpp
    Type.hpp
src/
  Cli.cpp
  OrderBook.cpp
  main.cpp
tests/
  OrderBookTest.cpp
CMakeLists.txt
```

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Test

```bash
ctest --test-dir build --output-on-failure
```

## Run CLI

```bash
./build/orderbook_cli
```

## CLI Commands

```text
help
buy <price> <quantity>
sell <price> <quantity>
cancel <order_id>
book
trades
best
clear
exit
quit
```

## Current V2 Scope

This version intentionally stays single-symbol. It does not yet include multi-symbol routing, users, portfolios, cash/share settlement, bots, benchmarks, or custom allocators.

## Next Steps

- Replace `-1` sentinel values with `std::optional`
- Add stronger randomized invariant testing
- Consider an explicit order-location index with side, price, and iterator
- Add GitHub Actions CI
- Start V3 multi-symbol exchange routing after V2 is stable
