#pragma once 
#include<cstdint>
#include<Type.hpp>
#include<orderBook/Trade.hpp>
#include<vector>

namespace Orderbook{
    

    
    struct Order{
        Orderbook::Price price;
        Orderbook::Quantity quantity;
        Orderbook::OrderId orderId;
        Orderbook::Type type;  
    };

    struct OrderResult{
        bool accepted;
        Orderbook::OrderId orderId;
        std::vector<Orderbook::Trade>trade;
        Orderbook::Quantity remainQuantity;
    };


}