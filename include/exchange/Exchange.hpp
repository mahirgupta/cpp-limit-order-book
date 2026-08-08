#pragma once
#include<cstdint>
#include<string>
#include<vector>
#include<map>
#include<exchange/ExchangeType.hpp>
#include<orderBook/OrderBook.hpp>
#include<exchange/Account.hpp>
#include<tuple>
#include<set>

namespace Exch{


    class Exchange{

    private:
        Orderbook::OrderId curOrderId;
        std::map<Orderbook::OrderId, Orderbook::Symbol>orderToSymbol;
        std::map<Orderbook::Symbol, Orderbook::OrderBook>symbolToBook;
        Orderbook::UserId curUserId;
        std::map<Orderbook::UserId, Exch::account>accounts;


    public:

        Exchange();

        Exchange(const Exchange&) = delete;

        Exchange& operator=(const Exchange&) = delete;

        std::optional<Orderbook::Symbol> getSymbolFromId(const Orderbook::OrderId id) const;

        bool addSymbol(const Orderbook::Symbol& symbol);

        bool hasSymbol(const Orderbook::Symbol& symbol) const;

        std::vector<Orderbook::Symbol>getSymbol()const;

        Orderbook::UserId addUser();

        bool hasUser(const Orderbook::UserId userid);

        std::vector<Orderbook::UserId> getUsers()const;

        bool depositCash(const Orderbook::Cash ammount, const Orderbook::UserId userid);

        bool withdrawCash(const Orderbook::Cash ammount, const Orderbook::UserId userid);

        Orderbook::Quantity depositPosition(const Orderbook::Quantity, const Orderbook::Symbol, const Orderbook::UserId);

        bool deleteUser(const Orderbook::UserId userid);

        std::optional<Exch::account> getAccount(const Orderbook::UserId userid) const;

        std::optional<Orderbook::Cash> getAvailableCash(const Orderbook::UserId userid) ;

        std::optional<Orderbook::Quantity>getAvailablePositions(const Orderbook::UserId userid, const Orderbook::Symbol sym);

        Exch::ExchangeOrderResult buy(const Orderbook::Symbol& symbol, Orderbook::Price price, Orderbook::Quantity quantity, Orderbook::UserId userid );

        Exch::ExchangeOrderResult sell(const Orderbook::Symbol& symbol, Orderbook::Price price, Orderbook::Quantity quantity, Orderbook::UserId userid );

        Exch::CancelResult cancelOrder(Orderbook::OrderId orderId, Orderbook::UserId userid);

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
