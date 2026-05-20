#include<exchange/Exchange.hpp>

namespace Exch{

    Exchange::Exchange(){
        Exchange::curOrderId=0;
        Exchange::orderToSymbol.clear();
        Exchange::symbolToBook.clear();
    }

    Orderbook::Symbol Exchange::getSymbolfromid(const Orderbook::OrderId id)const{
            auto it = Exchange::orderToSymbol.find(id);
            if(it == Exchange::orderToSymbol.end()){
                return "None";
            }
            return it->second;
        }

    bool Exch::Exchange::addSymbol(const Orderbook::Symbol& symbol){
        if(Exchange::symbolToBook.find(symbol)!=Exchange::symbolToBook.end()) return false;
        // Exchange::symbolToBook.emplace(symbol,Orderbook::OrderBook{});
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

    Exch::ExchangeOrderResult Exchange::buy(const Orderbook::Symbol& symbol, Orderbook::Price price, Orderbook::Quantity quantity){
        if(Exchange::symbolToBook.find(symbol)==Exchange::symbolToBook.end() || price<=0 || quantity<=0){
            return {0, "Invalid Input", symbol,0, {}, 0};
        }

        Orderbook::OrderBook &book = Exchange::symbolToBook[symbol];

        Orderbook::OrderResult res = book.makeBuyOrder(price,quantity, ++curOrderId);

        std::string mes;
        if(res.accepted) {
            mes = "Succesfully Executed";
            Exchange::orderToSymbol[curOrderId] = symbol;
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
            trd.push_back(temp);
            if(!Exchange::symbolToBook[symbol].checkOrder(i.buyerOrderId)) Exchange::orderToSymbol.erase(i.buyerOrderId);
            if(!Exchange::symbolToBook[symbol].checkOrder(i.sellerOrderId)) Exchange::orderToSymbol.erase(i.sellerOrderId);
        }
        return {res.accepted, mes, symbol, res.orderId, trd, res.remainQuantity };
        
    }
    
    
    Exch::ExchangeOrderResult Exchange::sell(const Orderbook::Symbol& symbol, Orderbook::Price price, Orderbook::Quantity quantity ){
        if(Exchange::symbolToBook.find(symbol)==Exchange::symbolToBook.end() || price<=0 || quantity<=0){
            return {0, "Invalid Input", symbol,0, {}, 0};
        }
        
        Orderbook::OrderBook &book = Exchange::symbolToBook[symbol];
        
        Orderbook::OrderResult res = book.makeSellOrder(price,quantity, ++curOrderId);
        
        std::string mes;
        if(res.accepted) {
            mes = "Succesfully Executed";
            Exchange::orderToSymbol[curOrderId] = symbol;
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
            trd.push_back(temp);
            if(!Exchange::symbolToBook[symbol].checkOrder(i.buyerOrderId)) Exchange::orderToSymbol.erase(i.buyerOrderId);
            if(!Exchange::symbolToBook[symbol].checkOrder(i.sellerOrderId)) Exchange::orderToSymbol.erase(i.sellerOrderId);
        }

        

        return {res.accepted, mes, symbol, res.orderId, trd, res.remainQuantity };
    }



    Exch::CancelResult Exchange::cancelOrder(Orderbook::OrderId orderId){
        auto it1 = Exchange::orderToSymbol.find(orderId);
        if(it1==Exchange::orderToSymbol.end()) return {0,"None",""};
        
        auto it2 = Exchange::symbolToBook.find(it1->second);
        if(it2 == Exchange::symbolToBook.end()) return {0,it1->second, ""};

        auto &book = it2->second;
        bool ok = book.cancelOrder(orderId);
        Exch::CancelResult ans;
        ans.cancelled = ok;
        ans.symbol = it1->second;
        if(ok){
            Exchange::orderToSymbol.erase(orderId);
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


    Orderbook::Price Exchange::getBestBid(const Orderbook::Symbol& symbol) const{
        auto it = Exchange::symbolToBook.find(symbol);
        if(it == Exchange::symbolToBook.end()){
            return -1;
        }

        auto &book = it->second;
        return book.getBestBid();
    }
    
    
    Orderbook::Price Exchange::getBestAsk(const Orderbook::Symbol& symbol) const{
        auto it = Exchange::symbolToBook.find(symbol);
        if(it == Exchange::symbolToBook.end()){
            return -1;
        }

        auto &book = it->second;
        return book.getBestAsk();
    }
    

    Orderbook::Price Exchange::getSpread(const Orderbook::Symbol& symbol) const{
        auto it = Exchange::symbolToBook.find(symbol);
        if(it == Exchange::symbolToBook.end()){
            return -1;
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

        book.clear();
        std::vector<Orderbook::OrderId>temp;
        for(auto &i:Exchange::orderToSymbol){
            if(i.second==symbol) temp.push_back(i.first);
        }
        
        for(auto &i:temp) Exchange::orderToSymbol.erase(i);

    }


    void Exchange::clearAll(){
        for(auto &i:Exchange::symbolToBook){
            i.second.clear();
        }
        orderToSymbol.clear();
        curOrderId=0;
    }

    Exchange::~Exchange(){
        Exchange::orderToSymbol.clear();
        Exchange::symbolToBook.clear();
    }
}