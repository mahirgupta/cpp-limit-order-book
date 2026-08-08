#pragma once
#include<orderBook/OrderBook.hpp>
#include<exchange/Exchange.hpp>
#include<vector>

class Cli{
public:
    void printBuyOrderList(Exch::Exchange &ex, const Orderbook::Symbol& sym);
    void printSellOrderList(Exch::Exchange &ex,  const Orderbook::Symbol& sym);
    static void printTrade(const std::vector<Exch::ExchangeTrade>& trade);
    void printBest(const Exch::Exchange& ex, const Orderbook::Symbol& sym);
    void printSymbol(const std::vector<std::string>& v);
    void printUsers(const std::vector<Orderbook::UserId> &v);
    void printAccount(const Exch::account& account);

    void run(Exch::Exchange &ex);
};
