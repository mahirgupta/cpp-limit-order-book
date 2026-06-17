# C++ Limit Order Book - V4

Multi-symbol limit order book and exchange-style CLI written in C++17.

V4 adds users, accounts, available cash, reserved cash, available positions,
reserved positions, account trade history, and settlement on top of the V3
multi-symbol exchange layer.

## Features

- Multiple symbols
- One independent `OrderBook` per symbol
- Global order ids across all symbols
- User accounts with cash and positions
- Available and reserved balances for open orders
- Buy and sell limit orders
- Price-time priority matching
- Resting-order-price trade execution
- Full and partial fills
- Multi-level sweeps
- Cash/position settlement on trades
- Price improvement refund for buyers
- Cancel active orders by user id and global order id
- Symbol-specific trade history
- Account-level trade history
- Best bid, best ask, and spread per symbol using `std::optional`
- Clear one symbol or clear all books while releasing reservations
- `OrderBook` and `Exchange` invariant checks
- OrderBook and Exchange regression tests
- Randomized invariant-style tests

## Project Layout

```text
include/
  Type.hpp
  Cli/
    Cli.hpp
  exchange/
    Account.hpp
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
add_user
users
account <UserId>
deposit_cash <UserId> <amount>
withdraw_cash <UserId> <amount>
deposit_position <UserId> <symbol> <quantity>
buy <UserId> <symbol> <price> <quantity>
sell <UserId> <symbol> <price> <quantity>
cancel <UserId> <order_id>
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
add_user
add_user

deposit_cash 1 1000
deposit_position 2 AAPL 5

buy 1 AAPL 100 5
sell 2 AAPL 90 3

book AAPL
trades AAPL
account 1
account 2
```

Orders only match inside the same symbol. So a buy on `AAPL` will not match a
sell on `MSFT`.

## V4 Account Model

- `cash` is available cash that can be withdrawn or reserved for new buy orders.
- `reservedCash` is cash locked by active buy orders.
- `positions[symbol]` is available inventory that can be reserved for sell orders.
- `reservedPositions[symbol]` is inventory locked by active sell orders.

When a buy order is accepted, `price * quantity` moves from `cash` to
`reservedCash`. When a sell order is accepted, quantity moves from available
position to reserved position. Fills settle both sides and release the consumed
reserved balances. Cancel and clear operations release remaining reservations.

Self-trades are currently registered as normal trades. The global symbol trade
history stores one execution, while the account history stores both sides for
the same user. This keeps the model ready for future side-based fees or taxes.

## Current V4 Scope

Current focus is:

- correct single-symbol matching
- clean multi-symbol routing
- global order id cancel routing
- user-aware order placement and cancel
- cash and position reservation
- trade settlement between accounts
- self-trade registration
- symbol-level and account-level trade queries
- invariant checks after order, cancel, clear, and settlement flows
- deterministic tests for exchange and account behavior

## Next Steps

- Add richer cancel/order status enums
- Add account trade side metadata for clearer buy/sell-side history
- Add exchange fees or taxes
- Add GitHub Actions CI
- Add benchmarks
- Explore bots or strategy simulation
- Explore multithreading after correctness is locked down
