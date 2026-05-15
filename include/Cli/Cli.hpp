#pragma once
#include<orderBook/OrderBook.hpp>
#include<vector>

class Cli{
public:
    void printBuyOrderList(Orderbook::OrderBook *bk);
    void printSellOrderList(Orderbook::OrderBook *bk);
    static void printTrade(std::vector<Orderbook::Trade>trade);
    void printBest(Orderbook::OrderBook *bk);

    void run(Orderbook::OrderBook *bk);
};