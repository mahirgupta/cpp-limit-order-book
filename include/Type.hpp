#pragma once
#include<cstdint>
#include<string>

namespace Orderbook{
    using Price = std::int64_t;
    using Quantity = std::int64_t;
    using OrderId = std::int64_t;
    using Symbol = std::string;

    enum class Type{
        buy,
        sell
    };



}