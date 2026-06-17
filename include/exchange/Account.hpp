#pragma once
#include<Type.hpp>
#include<orderBook/Trade.hpp>
#include<exchange/ExchangeType.hpp>
#include<map>
#include<set>
#include<list>



namespace Exch{
    struct account{
        Orderbook::UserId userid=0;
        Orderbook::Cash cash=0;
        Orderbook::Cash reservedCash=0;
        std::map<Orderbook::Symbol, Orderbook::Quantity>positions;
        std::map<Orderbook::Symbol, Orderbook::Quantity>reservedPositions;
        std::list<Exch::ExchangeTrade> trade;
        std::set<Orderbook::OrderId>orders;

    };
}
