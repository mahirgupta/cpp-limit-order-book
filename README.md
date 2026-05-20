# C++ Limit Order Book - V3

Multi-symbol limit order book and exchange style CLI written in C++17.

V3 adds an `Exchange` layer on top of the V2 single-symbol `OrderBook`.
Each symbol has its own independent book, and the exchange routes orders,
cancels, best bid/ask, book views, and trades by symbol.

## Features

- Multiple symbols
- One independent `OrderBook` per symbol
- Global order ids across all symbols
- Buy and sell limit orders
- Price-time priority matching
- Resting-order-price trade execution
- Full and partial fills
- Multi-level sweeps
- Cancel active orders by global order id
- Symbol specific trade history
- Best bid, best ask, and spread per symbol
- Clear one symbol or clear all books
- OrderBook and Exchange regression tests
- Randomized invariant style tests

## Project Layout

```text
include/
  Type.hpp
  Cli/
    Cli.hpp
  exchange/
    Exchange.hpp
    ExchangeType.hpp
  orderBook/
    Order.hpp
    OrderBook.hpp
    Trade.hpp
src/
  Cli.cpp
  Exchange.cpp
  OrderBook.cpp
  main.cpp
tests/
  ExchangeTest.cpp
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

Or run directly:

```bash
./build/orderbook_tests
./build/exchange_tests
```

## Run CLI

```bash
./build/orderbook_cli
```

## CLI Commands

```text
help
add_symbol <symbol>
symbols
buy <symbol> <price> <quantity>
sell <symbol> <price> <quantity>
cancel <order_id>
book <symbol>
trades <symbol>
best <symbol>
clear <symbol>
clear_all
exit
quit
```

## Example

```text
add_symbol AAPL
add_symbol MSFT

buy AAPL 100 10
sell AAPL 100 4

book AAPL
trades AAPL
best AAPL
```

Orders only match inside the same symbol. So a buy on `AAPL` will not match a sell on `MSFT`.

## Current V3 Scope

V3 is still not doing users, portfolios, cash, holdings, settlement, bots,
benchmarks, or multithreading.

Current focus is:

- correct single-symbol matching
- clean multi-symbol routing
- global order id cancel routing
- symbol level book/trade queries
- deterministic tests for exchange behavior

## Next Steps

- Add `Exchange::checkInvariants()`
- Replace `-1` sentinel values with `std::optional`
- Add GitHub Actions CI
- Update naming/style later if needed
- Start V4 users, portfolios, cash reservation, and settlement after V3 is stable
