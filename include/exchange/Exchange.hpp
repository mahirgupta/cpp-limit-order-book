#pragma once
#include<cstdint>
#include<string>
#include<vector>
#include<map>
#include<exchange/ExchangeType.hpp>
#include<orderBook/OrderBook.hpp>

namespace Exch{
    
    
    class Exchange{
    
    private:
        Orderbook::OrderId curOrderId;
        std::map<Orderbook::OrderId, Orderbook::Symbol>orderToSymbol;
        std::map<Orderbook::Symbol, Orderbook::OrderBook>symbolToBook;


    public:
        
        Orderbook::Symbol getSymbolfromid(const Orderbook::OrderId id);

        bool addSymbol(const Orderbook::Symbol& symbol);

        bool hasSymbol(const Orderbook::Symbol& symbol) const;

        std::vector<Orderbook::Symbol>getSymbol()const;

        Exch::ExchangeOrderResult buy(const Orderbook::Symbol& symbol, Orderbook::Price price, Orderbook::Quantity quantity );
        
        Exch::ExchangeOrderResult sell(const Orderbook::Symbol& symbol, Orderbook::Price price, Orderbook::Quantity quantity );
       
        bool cancelOrder(Orderbook::OrderId orderId);

        std::vector<Orderbook::Order> getBuyOrders(const Orderbook::Symbol& symbol) const;
        
        std::vector<Orderbook::Order> getSellOrders(const Orderbook::Symbol& symbol) const;
        
        Orderbook::Price getBestBid(const Orderbook::Symbol& symbol) const;

        Orderbook::Price getBestAsk(const Orderbook::Symbol& symbol) const;
        
        Orderbook::Price getSpread(const Orderbook::Symbol& symbol) const;

        std::vector<ExchangeTrade> getTrades(const Orderbook::Symbol& symbol) const;

        void clearSymbol(const Orderbook::Symbol& symbol) ;

        void clearAll();

        ~Exchange();

    };
}