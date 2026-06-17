#pragma once
#include<Type.hpp>
#include<vector>


namespace Exch{
    enum class ExchangeOrderStatus{
        AcceptedResting,
        AcceptedFilled,
        AcceptedPartiallyFilled,
        RejectedInvalidInput,
        RejectedDuplicateOrderId,
        RejectedUnknownSymbol,
        RejectedUnknownUser,
        RejectedInsufficientCash,
        RejectedInsufficientShares,
        RejectedCashOverflow
    };

    enum class CancelStatus{
        Cancelled,
        UnknownOrder,
        UnknownSymbol,
        UnknownUser,
        Unauthorized,
        NotCancelled
    };

    struct ExchangeTrade{
        Orderbook::Symbol symbol;
        Orderbook::OrderId buyerOrderId;
        Orderbook::OrderId sellerOrderId;
        Orderbook::UserId buyerUserId;
        Orderbook::UserId sellerUserId;
        Orderbook::Price price;
        Orderbook::Quantity quantity;
    };


    struct ExchangeOrderResult{
        bool accepted;
        Exch::ExchangeOrderStatus status;
        std::string message;
        Orderbook::Symbol symbol;
        Orderbook::OrderId orderId;
        std::vector<Exch::ExchangeTrade>trades;
        Orderbook::Quantity remainQuantity;
        Orderbook::UserId UserID;
    };

    struct CancelResult{
        bool cancelled;
        Exch::CancelStatus status;
        Orderbook::Symbol symbol;
        std::string message;
    };

}
