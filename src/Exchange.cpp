#include<exchange/Exchange.hpp>

namespace Exch{

    Exchange::Exchange(){
        Exchange::curOrderId=0;
        Exchange::orderToSymbol.clear();
        Exchange::symbolToBook.clear();
        Exchange::curUserId = 0;
    }

    std::optional<Orderbook::Symbol> Exchange::getSymbolFromId(const Orderbook::OrderId id)const{
            auto it = Exchange::orderToSymbol.find(id);
            if(it == Exchange::orderToSymbol.end()){
                return std::nullopt;
            }
            return it->second;
        }

    bool Exch::Exchange::addSymbol(const Orderbook::Symbol& symbol){
        if(Exchange::symbolToBook.find(symbol)!=Exchange::symbolToBook.end()) return false;
        auto result = Exchange::symbolToBook.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(symbol),
            std::forward_as_tuple()
        );
        return result.second;
    }

    bool Exch::Exchange::hasSymbol(const Orderbook::Symbol& symbol)const{
        if(Exchange::symbolToBook.find(symbol)!=Exchange::symbolToBook.end())return true;
        return false;
    }

    std::vector<Orderbook::Symbol> Exchange::getSymbol()const{
        std::vector<Orderbook::Symbol> ans;
        for(const auto &i: Exchange::symbolToBook){
            ans.push_back(i.first);
        }
        return ans;
    }

    Orderbook::UserId Exch::Exchange::addUser(){
        Exch::account a;
        a.userid = ++Exchange::curUserId;
        Exchange::accounts[curUserId] = a;
        return curUserId;
    }

    bool Exch::Exchange::hasUser(Orderbook::UserId userid){
        return(Exchange::accounts.find(userid)!=Exchange::accounts.end());
    }

    std::vector<Orderbook::UserId> Exchange::getUsers()const{
        std::vector<Orderbook::UserId>ans;
        for(auto i:Exchange::accounts) ans.push_back(i.first);
        return ans;
    }

    bool Exchange::deleteUser(const Orderbook::UserId userid){
        if(!Exchange::hasUser(userid)) return true; // already deleted no user
        auto hasNonZeroBalance = [](const auto& balances){
            for(const auto& i:balances){
                if(i.second!=0) return true;
            }
            return false;
        };

        const auto& account = Exchange::accounts[userid];
        if(account.cash!=0 || account.reservedCash!=0 || hasNonZeroBalance(account.positions) || hasNonZeroBalance(account.reservedPositions) || !account.trade.empty()) return false;

        std::vector<Orderbook::OrderId>temp;
        for(auto i:Exchange::accounts[userid].orders) temp.push_back(i);

        bool ok = true;

        for(auto i:temp) ok &= Exchange::cancelOrder(i,userid).cancelled;

        if(ok) Exchange::accounts.erase(userid);
        return ok;

    }

    std::optional<Exch::account> Exchange::getAccount(const Orderbook::UserId userid) const{
        auto it = Exchange::accounts.find(userid);
        if(it==Exchange::accounts.end()) return std::nullopt;
        return it->second;
    }

    std::optional<Orderbook::Cash> Exchange::getAvailableCash(const Orderbook::UserId userid){
        if(!Exchange::hasUser(userid)) return std::nullopt;
        return Exchange::accounts[userid].cash;
    }

    std::optional<Orderbook::Quantity>Exchange::getAvailablePositions(const Orderbook::UserId userid, const Orderbook::Symbol sym){
        if(!Exchange::hasUser(userid)) return std::nullopt;
        if(Exchange::accounts[userid].positions.find(sym)==Exchange::accounts[userid].positions.end()) return std::nullopt;
        return Exchange::accounts[userid].positions[sym];
    }

    bool Exchange::depositCash(const Orderbook::Cash ammount, const Orderbook::UserId userid){
        if(!Exchange::hasUser(userid) || ammount<=0)return false;

        Exchange::accounts[userid].cash+=ammount;
        return true;
    }

    bool Exchange::withdrawCash(const Orderbook::Cash ammount, const Orderbook::UserId userid){
        if(!Exchange::hasUser(userid) || Exchange::getAvailableCash(userid)<ammount || ammount<=0) return false;
        Exchange::accounts[userid].cash-=ammount;
        return true;
    }

    Orderbook::Quantity Exchange::depositPosition(const Orderbook::Quantity quantity, const Orderbook::Symbol sym , const Orderbook::UserId userid){
        if(!Exchange::hasUser(userid)|| !Exchange::hasSymbol(sym) || quantity<=0) return 0;
        return Exchange::accounts[userid].positions[sym]+=quantity;
    }


    Exch::ExchangeOrderResult Exchange::buy(const Orderbook::Symbol& symbol, Orderbook::Price price, Orderbook::Quantity quantity, Orderbook::UserId userid){
        if(Exchange::symbolToBook.find(symbol)==Exchange::symbolToBook.end() || price<=0 || quantity<=0 || !Exchange::hasUser(userid)){
            return {0, "Invalid Input", symbol,0, {}, 0, userid};
        }

        if(Exchange::getAvailableCash(userid) < price*quantity) return {0,"Insufficient Cash", symbol,0,{},0, userid};


        Orderbook::OrderBook &book = Exchange::symbolToBook[symbol];

        Orderbook::OrderResult res = book.makeBuyOrder(price,quantity, ++curOrderId, userid);


        std::string mes;
        if(res.accepted) {
            mes = "Succesfully Executed";
            Exchange::orderToSymbol[curOrderId] = symbol;
            Exchange::accounts[userid].orders.insert(curOrderId);
            Exchange::accounts[userid].cash-= price*quantity;
            Exchange::accounts[userid].reservedCash+=price*quantity;
        }
        else mes = "Error can't placed order";


        std::vector<Exch::ExchangeTrade>trd;

        for(const auto &i:res.trade){
            Exch::ExchangeTrade temp;
            temp.buyerOrderId = i.buyerOrderId;
            temp.sellerOrderId = i.sellerOrderId;
            temp.price = i.price;
            temp.quantity = i.quantity;
            temp.symbol = symbol;
            temp.buyerUserId= i.buyerUserId;
            temp.sellerUserId = i.sellerUserId;
            Exchange::accounts[temp.buyerUserId].reservedCash-=i.buyerPrice*temp.quantity;
            Exchange::accounts[temp.buyerUserId].cash+=(i.buyerPrice-temp.price)*temp.quantity;
            Exchange::accounts[temp.buyerUserId].positions[temp.symbol]+=temp.quantity;
            Exchange::accounts[temp.sellerUserId].cash+=temp.price*temp.quantity;
            Exchange::accounts[temp.sellerUserId].reservedPositions[temp.symbol]-=temp.quantity;
            Exchange::accounts[temp.buyerUserId].trade.push_back(temp);
            Exchange::accounts[temp.sellerUserId].trade.push_back(temp);

            trd.push_back(temp);
            if(!Exchange::symbolToBook[symbol].checkOrder(i.buyerOrderId)) Exchange::orderToSymbol.erase(i.buyerOrderId), Exchange::accounts[i.buyerUserId].orders.erase(i.buyerOrderId);
            if(!Exchange::symbolToBook[symbol].checkOrder(i.sellerOrderId)) Exchange::orderToSymbol.erase(i.sellerOrderId), Exchange::accounts[i.sellerUserId].orders.erase(i.sellerOrderId);
        }
        return {res.accepted, mes, symbol, res.orderId, trd, res.remainQuantity, res.UserId };

    }


    Exch::ExchangeOrderResult Exchange::sell(const Orderbook::Symbol& symbol, Orderbook::Price price, Orderbook::Quantity quantity, Orderbook::UserId userid ){
        if(Exchange::symbolToBook.find(symbol)==Exchange::symbolToBook.end() || price<=0 || quantity<=0 || !Exchange::hasUser(userid)){
            return {0, "Invalid Input", symbol,0, {}, 0, userid};
        }

        if(Exchange::getAvailablePositions(userid,symbol)<quantity) return {0,"Insufficient Quantity",symbol,0,{},0, userid};

        Orderbook::OrderBook &book = Exchange::symbolToBook[symbol];

        Orderbook::OrderResult res = book.makeSellOrder(price,quantity, ++curOrderId, userid);


        std::string mes;
        if(res.accepted) {
            mes = "Succesfully Executed";
            Exchange::orderToSymbol[curOrderId] = symbol;
            Exchange::accounts[userid].orders.insert(curOrderId);
            Exchange::accounts[userid].positions[symbol]-=quantity;
            Exchange::accounts[userid].reservedPositions[symbol]+=quantity;
        }
        else mes = "Error can't placed order";


        std::vector<Exch::ExchangeTrade>trd;

        for(const auto &i:res.trade){
            Exch::ExchangeTrade temp;
            temp.buyerOrderId = i.buyerOrderId;
            temp.sellerOrderId = i.sellerOrderId;
            temp.buyerUserId = i.buyerUserId;
            temp.sellerUserId = i.sellerUserId;
            temp.price = i.price;
            temp.quantity = i.quantity;
            temp.symbol = symbol;
            Exchange::accounts[temp.buyerUserId].reservedCash-=i.buyerPrice*temp.quantity;
            Exchange::accounts[temp.buyerUserId].cash+=(i.buyerPrice-temp.price)*temp.quantity;
            Exchange::accounts[temp.buyerUserId].positions[temp.symbol]+=temp.quantity;
            Exchange::accounts[temp.sellerUserId].cash+=temp.price*temp.quantity;
            Exchange::accounts[temp.sellerUserId].reservedPositions[temp.symbol]-=temp.quantity;
            Exchange::accounts[temp.buyerUserId].trade.push_back(temp);
            Exchange::accounts[temp.sellerUserId].trade.push_back(temp);


            trd.push_back(temp);

            if(!Exchange::symbolToBook[symbol].checkOrder(i.buyerOrderId)) Exchange::orderToSymbol.erase(i.buyerOrderId), Exchange::accounts[i.buyerUserId].orders.erase(i.buyerOrderId);
            if(!Exchange::symbolToBook[symbol].checkOrder(i.sellerOrderId)) Exchange::orderToSymbol.erase(i.sellerOrderId), Exchange::accounts[i.sellerUserId].orders.erase(i.sellerOrderId);
        }



        return {res.accepted, mes, symbol, res.orderId, trd, res.remainQuantity, res.UserId };
    }



    Exch::CancelResult Exchange::cancelOrder(Orderbook::OrderId orderId, Orderbook::UserId userid){
        auto it1 = Exchange::orderToSymbol.find(orderId);
        if(it1==Exchange::orderToSymbol.end()) return {0,"","None"};

        auto it2 = Exchange::symbolToBook.find(it1->second);
        if(it2 == Exchange::symbolToBook.end()) return {0,it1->second, "Unknown Symbol"};


        auto &book = it2->second;
        if(book.getUserIdFromOrder(orderId)!=userid) return {0,it1->second,"Order Id Not match with User Id"};
        Orderbook::CancelOrderResult ok = book.cancelOrder(orderId);
        Exch::CancelResult ans;
        ans.cancelled = ok.cancelled;
        ans.symbol = it1->second;
        if(ok.cancelled){
            Exchange::orderToSymbol.erase(orderId);
            if(ok.type==Orderbook::Type::buy){
                Exchange::accounts[userid].cash+=ok.orderPrice*ok.orderQuantity;
                Exchange::accounts[userid].reservedCash-=ok.orderPrice*ok.orderQuantity;
            }
            else{
                Exchange::accounts[userid].positions[ans.symbol]+=ok.orderQuantity;
                Exchange::accounts[userid].reservedPositions[ans.symbol]-=ok.orderQuantity;
            }
            Exchange::accounts[userid].orders.erase(orderId);


        }
        return ans;
    }


    std::vector<Orderbook::Order> Exchange::getBuyOrders(const Orderbook::Symbol& symbol) const{
        auto it = Exchange::symbolToBook.find(symbol);
        if(it == Exchange::symbolToBook.end()){
            return {};
        }
        const auto &book = it->second;
        return book.getBuyOrders();
    }


    std::vector<Orderbook::Order> Exchange::getSellOrders(const Orderbook::Symbol& symbol) const{
        auto it = Exchange::symbolToBook.find(symbol);
        if(it == Exchange::symbolToBook.end()){
            return {};
        }
        const auto &book = it->second;
        return book.getSellOrders();
    }


    std::optional<Orderbook::Price> Exchange::getBestBid(const Orderbook::Symbol& symbol) const{
        auto it = Exchange::symbolToBook.find(symbol);
        if(it == Exchange::symbolToBook.end()){
            return std::nullopt;
        }

        auto &book = it->second;
        return book.getBestBid();
    }


    std::optional<Orderbook::Price> Exchange::getBestAsk(const Orderbook::Symbol& symbol) const{
        auto it = Exchange::symbolToBook.find(symbol);
        if(it == Exchange::symbolToBook.end()){
            return std::nullopt;
        }

        auto &book = it->second;
        return book.getBestAsk();
    }


    std::optional<Orderbook::Price> Exchange::getSpread(const Orderbook::Symbol& symbol) const{
        auto it = Exchange::symbolToBook.find(symbol);
        if(it == Exchange::symbolToBook.end()){
            return std::nullopt;
        }

        auto &book = it->second;
        return book.getSpread();
    }


    std::vector<ExchangeTrade> Exchange::getTrades(const Orderbook::Symbol& symbol) const{
        auto it = Exchange::symbolToBook.find(symbol);
        if(it == Exchange::symbolToBook.end()){
            return {};
        }

        auto &book = it->second;
        auto trd = book.getTrade();

        std::vector<Exch::ExchangeTrade> ans;
        for(auto &i:trd){
            Exch::ExchangeTrade temp;
            temp.buyerOrderId = i.buyerOrderId;
            temp.sellerOrderId = i.sellerOrderId;
            temp.price = i.price;
            temp.quantity = i.quantity;
            temp.symbol = symbol;
            temp.buyerUserId = i.buyerUserId;
            temp.sellerUserId = i.sellerUserId;
            ans.push_back(temp);
        }
        return ans;

    }


    void Exchange::clearSymbol(const Orderbook::Symbol& symbol){
         auto it = Exchange::symbolToBook.find(symbol);
        if(it == Exchange::symbolToBook.end()){
            return ;
        }

        auto &book = it->second;
        auto v1 = book.getBuyOrders();
        auto v2 = book.getSellOrders();

        for(auto i:v1){
            Exchange::cancelOrder(i.orderId,i.UserId);
        }

        for(auto i:v2){
            Exchange::cancelOrder(i.orderId, i.UserId);
        }

        book.clear();
        std::vector<Orderbook::OrderId>temp;
        for(auto &i:Exchange::orderToSymbol){
            if(i.second==symbol) temp.push_back(i.first);
        }

        for(auto &i:temp) Exchange::orderToSymbol.erase(i);

    }


    void Exchange::clearAll(){
        for(auto &symbolEntry:Exchange::symbolToBook){
            auto v1 = symbolEntry.second.getBuyOrders();
            auto v2 = symbolEntry.second.getSellOrders();
            for(auto order:v1){
                Exchange::cancelOrder(order.orderId,order.UserId);
            }

            for(auto order:v2){
                Exchange::cancelOrder(order.orderId, order.UserId);
            }
            symbolEntry.second.clear();

        }
        orderToSymbol.clear();
        curOrderId=0;
    }

    bool Exchange::checkInvariant()const{
        std::map<Orderbook::UserId, Orderbook::Cash> expectedReservedCash;
        std::map<Orderbook::UserId, std::map<Orderbook::Symbol, Orderbook::Quantity>> expectedReservedPositions;
        std::map<Orderbook::UserId, std::set<Orderbook::OrderId>> expectedOrders;
        std::set<Orderbook::OrderId> activeOrderIds;

        for(const auto& accountEntry : Exchange::accounts){
            const auto userId = accountEntry.first;
            const auto& account = accountEntry.second;

            if(account.userid != userId)return false;
            if(account.cash < 0)return false;
            if(account.reservedCash < 0)return false;

            for(const auto& position : account.positions){
                if(position.second < 0)return false;
                if(!Exchange::hasSymbol(position.first))return false;
            }

            for(const auto& reservedPosition : account.reservedPositions){
                if(reservedPosition.second < 0)return false;
                if(!Exchange::hasSymbol(reservedPosition.first))return false;
            }
        }

        for(const auto &i:Exchange::symbolToBook){
            const auto& sym = i.first;
            const auto& book = i.second;
            if(!book.checkInvariants())return false;

            for(const auto& order : book.getBuyOrders()){
                auto symbolIt = Exchange::orderToSymbol.find(order.orderId);
                if(symbolIt==Exchange::orderToSymbol.end())return false;
                if(symbolIt->second != sym)return false;

                auto accountIt = Exchange::accounts.find(order.UserId);
                if(accountIt==Exchange::accounts.end())return false;
                if(accountIt->second.orders.find(order.orderId) == accountIt->second.orders.end())return false;
                if(!activeOrderIds.insert(order.orderId).second)return false;
                expectedOrders[order.UserId].insert(order.orderId);
                expectedReservedCash[order.UserId] += order.price * order.quantity;
            }

            for(const auto& order : book.getSellOrders()){
                auto symbolIt = Exchange::orderToSymbol.find(order.orderId);
                if(symbolIt==Exchange::orderToSymbol.end())return false;
                if(symbolIt->second != sym)return false;

                auto accountIt = Exchange::accounts.find(order.UserId);
                if(accountIt==Exchange::accounts.end())return false;
                if(accountIt->second.orders.find(order.orderId) == accountIt->second.orders.end())return false;
                if(!activeOrderIds.insert(order.orderId).second)return false;
                expectedOrders[order.UserId].insert(order.orderId);
                expectedReservedPositions[order.UserId][sym] += order.quantity;
            }
        }



        for(const auto &i: Exchange::orderToSymbol){
            if(!Exchange::hasSymbol(i.second))return false;
            const auto bookIt = Exchange::symbolToBook.find(i.second);
            if(bookIt==Exchange::symbolToBook.end()) return false;
            const Orderbook::OrderBook& bk = bookIt->second;
            if(!bk.checkOrder(i.first))return false;
            if(activeOrderIds.find(i.first) == activeOrderIds.end())return false;
        }

        for(const auto& accountEntry : Exchange::accounts){
            const auto userId = accountEntry.first;
            const auto& account = accountEntry.second;

            if(account.orders != expectedOrders[userId])return false;
            if(account.reservedCash != expectedReservedCash[userId])return false;

            for(const auto& reservedPosition : account.reservedPositions){
                const auto expected = expectedReservedPositions[userId][reservedPosition.first];
                if(reservedPosition.second != expected)return false;
            }

            for(const auto& expectedPosition : expectedReservedPositions[userId]){
                const auto reservedIt = account.reservedPositions.find(expectedPosition.first);
                const auto actual = reservedIt == account.reservedPositions.end() ? 0 : reservedIt->second;
                if(actual != expectedPosition.second)return false;
            }
        }

        return true;

    }



    Exchange::~Exchange(){
        Exchange::orderToSymbol.clear();
        Exchange::symbolToBook.clear();
    }
}
