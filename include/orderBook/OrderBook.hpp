#pragma once
#include<cstdint>
#include<map>
#include<list>
#include<orderBook/Order.hpp>
#include<orderBook/Trade.hpp>




namespace Orderbook{
    class OrderBook{
    private:
        std::map<Price, std::list<Orderbook::Order>>buyOrders;
        std::map<Price, std::list<Orderbook::Order>>sellOrders;
        std::map<OrderId, std::list<Order>::iterator>orders;
        Orderbook::OrderId curOrderId;
        std::vector<Orderbook::Trade>trade;
        


    public:
        OrderBook();
        ~OrderBook();

        OrderId getCurOrderId();

        std::int64_t makeBuyOrder(Price price, Quantity quantity);
        
        std::int64_t makeSellOrder(Price price, Quantity quantity);

        bool cancelOrder(OrderId id);

        Price getBestBid();

        Price getBestAsk();

        Price getSpread();

        std::int64_t match(Type type);

        void makeTrade(OrderId buyId, OrderId sellId, Quantity quantity, Price price);

        void clear();

        std::vector<Order> getBuyOrders();

        std::vector<Order> getSellOrders();

        std::vector<Trade> getTrade();

        std::vector<Trade> getTrade(std::int64_t s);
    };
}