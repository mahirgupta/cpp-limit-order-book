# C++ Limit Order Book - V1

This is a simple single-symbol limit order book and matching engine written in C++.

The project supports basic buy/sell limit orders, automatic matching, trade generation, cancellations, trade history, and best bid/ask display.

## Features

- Buy limit orders
- Sell limit orders
- Automatic matching when best bid >= best ask
- Price-time priority
- Trade price uses resting order price
- Partial fills
- Full fills
- Cancel active orders by order id
- Trade history
- Best bid, best ask, and spread
- Clear book command
- Basic CLI input validation
- Memory cleanup on exit

## Commands

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