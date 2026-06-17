#pragma once
#include<Type.hpp>


namespace Orderbook{
    struct Trade{
        Orderbook::OrderId buyerOrderId;
        Orderbook::OrderId sellerOrderId;
        Orderbook::UserId buyerUserId;
        Orderbook::UserId sellerUserId;
        Orderbook::Price buyerPrice;
        Orderbook::Price sellerPrice;
        Orderbook::Price price;
        Orderbook::Quantity quantity;
    };
}