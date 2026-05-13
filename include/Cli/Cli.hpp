#pragma once
#include<orderBook/OrderBook.hpp>

class Cli{
private:
    Orderbook::OrderBook bk;
public:
    void printBuyOrderList();
    void printSellOrderList();
    void printTrade(std::vector<Orderbook::Trade>trade);
    void printBest();

    void run();
};