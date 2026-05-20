#pragma once
#include<Type.hpp>
#include<vector>


namespace Exch{
    struct ExchangeTrade{
        Orderbook::Symbol symbol;
        Orderbook::OrderId buyerOrderId;
        Orderbook::OrderId sellerOrderId;
        Orderbook::Price price;
        Orderbook::Quantity quantity;
    };

    
    struct ExchangeOrderResult{
        bool accepted;
        std::string message;
        Orderbook::Symbol symbol;
        Orderbook::OrderId orderId;
        std::vector<Exch::ExchangeTrade>trades;
        Orderbook::Quantity remainQuantity;
    };

}

