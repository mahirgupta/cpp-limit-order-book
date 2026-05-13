#pragma once
#include<Type.hpp>


namespace Orderbook{
    struct Trade{
        Orderbook::OrderId buyerOrderId;
        Orderbook::OrderId sellerOrderId;
        Orderbook::Price price;
        Orderbook::Quantity quantity;
    };
}