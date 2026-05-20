#include<exchange/Exchange.hpp>

namespace Exch{

    Orderbook::Symbol Exchange::getSymbolfromid(const Orderbook::OrderId id){
            auto it = Exchange::orderToSymbol.find(id);
            if(it == Exchange::orderToSymbol.end()){
                return "None";
            }
            return it->second;
        }

    bool Exch::Exchange::addSymbol(const Orderbook::Symbol& symbol){
        if(Exchange::symbolToBook.find(symbol)!=Exchange::symbolToBook.end()) return false;
        Orderbook::OrderBook book;
        Exchange::symbolToBook[symbol] = book;
        return true;
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
        if(res.accepted) mes = "Succesfully Executed";
        else mes = "Error can't placed order";


        std::vector<Exch::ExchangeTrade>trd;

        for(const auto &i:res.trade){
            Exch::ExchangeTrade temp;
            temp.buyerOrderId = i.buyerOrderId;
            temp.sellerOrderId = i.sellerOrderId;
            temp.price = i.price;
            temp.quantity = i.quantity;
            temp.symbol = symbol;
        }
        if(res.accepted) 

        return {res.accepted, mes, symbol, res.orderId, trd, res.remainQuantity };

    }


    Exch::ExchangeOrderResult Exchange::sell(const Orderbook::Symbol& symbol, Orderbook::Price price, Orderbook::Quantity quantity ){
        if(Exchange::symbolToBook.find(symbol)==Exchange::symbolToBook.end() || price<=0 || quantity<=0){
            return {0, "Invalid Input", symbol,0, {}, 0};
        }

        Orderbook::OrderBook &book = Exchange::symbolToBook[symbol];

        Orderbook::OrderResult res = book.makeSellOrder(price,quantity, ++curOrderId);

        std::string mes;
        if(res.accepted) mes = "Succesfully Executed";
        else mes = "Error can't placed order";


        std::vector<Exch::ExchangeTrade>trd;

        for(const auto &i:res.trade){
            Exch::ExchangeTrade temp;
            temp.buyerOrderId = i.buyerOrderId;
            temp.sellerOrderId = i.sellerOrderId;
            temp.price = i.price;
            temp.quantity = i.quantity;
            temp.symbol = symbol;
        }

        return {res.accepted, mes, symbol, res.orderId, trd, res.remainQuantity };
    }



    bool Exchange::cancelOrder(Orderbook::OrderId orderId){
        auto it1 = Exchange::orderToSymbol.find(orderId);
        if(it1==Exchange::orderToSymbol.end()) return false;
        
        auto it2 = Exchange::symbolToBook.find(it1->second);
        if(it2 == Exchange::symbolToBook.end()) return false;

        auto &book = it2->second;
        book.cancelOrder(orderId);
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
    }


    void Exchange::clearAll(){
        for(auto &i:Exchange::symbolToBook){
            i.second.clear();
        }
        curOrderId=0;
    }

    Exchange::~Exchange(){
        Exchange::orderToSymbol.clear();
        Exchange::symbolToBook.clear();
    }
}