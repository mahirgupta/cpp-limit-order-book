#pragma once
#include<cstdint>
#include<string>
#include<vector>
#include<map>
#include<exchange/ExchangeType.hpp>
#include<orderBook/OrderBook.hpp>
#include<tuple>
#include<set>

namespace Exch{
    
    
    class Exchange{
    
    private:
        Orderbook::OrderId curOrderId;
        std::map<Orderbook::OrderId, Orderbook::Symbol>orderToSymbol;
        std::map<Orderbook::Symbol, Orderbook::OrderBook>symbolToBook;


    public:

        Exchange();

        Exchange(const Exchange&) = delete;
        
        Exchange& operator=(const Exchange&) = delete;
        
        std::optional<Orderbook::Symbol> getSymbolFromId(const Orderbook::OrderId id) const;

        bool addSymbol(const Orderbook::Symbol& symbol);

        bool hasSymbol(const Orderbook::Symbol& symbol) const;

        std::vector<Orderbook::Symbol>getSymbol()const;

        Exch::ExchangeOrderResult buy(const Orderbook::Symbol& symbol, Orderbook::Price price, Orderbook::Quantity quantity );
        
        Exch::ExchangeOrderResult sell(const Orderbook::Symbol& symbol, Orderbook::Price price, Orderbook::Quantity quantity );
       
        Exch::CancelResult cancelOrder(Orderbook::OrderId orderId);

        std::vector<Orderbook::Order> getBuyOrders(const Orderbook::Symbol& symbol) const;
        
        std::vector<Orderbook::Order> getSellOrders(const Orderbook::Symbol& symbol) const;
        
        std::optional<Orderbook::Price> getBestBid(const Orderbook::Symbol& symbol) const;

        std::optional<Orderbook::Price> getBestAsk(const Orderbook::Symbol& symbol) const;
        
        std::optional<Orderbook::Price> getSpread(const Orderbook::Symbol& symbol) const;

        std::vector<ExchangeTrade> getTrades(const Orderbook::Symbol& symbol) const;

        void clearSymbol(const Orderbook::Symbol& symbol) ;

        void clearAll();

        bool checkInvariant() const;

        ~Exchange();

    };
}