#pragma once 
#include<cstdint>
#include<Type.hpp>

namespace Orderbook{
    

    
    struct Order{
        Orderbook::Price price;
        Orderbook::Quantity quantity;
        Orderbook::OrderId orderId;
        Orderbook::Type type;  
    };

}