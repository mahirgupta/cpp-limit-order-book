#pragma once
#include<cstdint>
#include<map>
#include<vector>
#include<list>
#include<orderBook/Order.hpp>
#include<orderBook/Trade.hpp>
#include<optional>




namespace Orderbook{
    class OrderBook{
    private:
        std::map<Price, std::list<Orderbook::Order>>buyOrders;
        std::map<Price, std::list<Orderbook::Order>>sellOrders;
        struct OrderLocation{
            Type side;
            Price price;
            std::list<Order>::iterator iterator;
        };
        std::map<OrderId, OrderLocation>orders;
        // Orderbook::OrderId curOrderId;
        std::vector<Orderbook::Trade>trade;

        std::vector<Orderbook::Trade> match(Type type);

        Orderbook::Trade makeTrade(OrderId buyId, OrderId sellId, Quantity quantity, Price price);


    public:
        OrderBook();
        ~OrderBook();
        OrderBook(const OrderBook&) = delete;
        OrderBook& operator=(const OrderBook&) = delete;

        // OrderId getCurOrderId() const;

        Orderbook::OrderResult makeBuyOrder(Price price, Quantity quantity, OrderId curOrderId);
        
        Orderbook::OrderResult makeSellOrder(Price price, Quantity quantity, OrderId curOrderId);

        bool cancelOrder(OrderId id);

        std::optional<Price> getBestBid() const;

        std::optional<Price> getBestAsk() const;

        std::optional<Price> getSpread() const;

        void clear();

        std::vector<Order> getBuyOrders() const;

        bool checkOrder(Orderbook::OrderId id)const;

        std::vector<Order> getSellOrders() const;

        std::vector<Trade> getTrade() const;

        std::vector<Trade> getTrade(std::int64_t s) const;

        bool checkInvariants() const;
    };
}
