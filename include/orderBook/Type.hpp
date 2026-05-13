#pragma once
#include<cstdint>

namespace Orderbook{
    using Price = std::int64_t;
    using Quantity = std::int64_t;
    using OrderId = std::int64_t;


    enum class Type{
        buy,
        sell
    };



}