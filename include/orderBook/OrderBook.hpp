#pragma once
#include<cstdint>
#include<map>
#include<vector>
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

        Orderbook::OrderResult makeBuyOrder(Price price, Quantity quantity);
        
        Orderbook::OrderResult makeSellOrder(Price price, Quantity quantity);

        bool cancelOrder(OrderId id);

        Price getBestBid();

        Price getBestAsk();

        Price getSpread();

        std::vector<Orderbook::Trade> match(Type type);

        Orderbook::Trade makeTrade(OrderId buyId, OrderId sellId, Quantity quantity, Price price);

        void clear();

        std::vector<Order> getBuyOrders();

        std::vector<Order> getSellOrders();

        std::vector<Trade> getTrade();

        std::vector<Trade> getTrade(std::int64_t s);
    };
}