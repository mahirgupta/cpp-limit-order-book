#include<orderBook/OrderBook.hpp>
#include<algorithm>

namespace Orderbook{

    OrderBook::OrderBook(){
        OrderBook::buyOrders.clear();
        OrderBook::sellOrders.clear();
        OrderBook::curOrderId = 0;
        OrderBook::orders.clear();
        trade.clear();
    }

    OrderId OrderBook::getCurOrderId(){
        return OrderBook::curOrderId;
    }

    Price OrderBook::getBestBid(){
        Price ans{-1};
        if(!OrderBook::buyOrders.empty()) {
            ans = (--buyOrders.end())->first;
        }
        return ans;
    }
    
    Price OrderBook::getBestAsk(){
        Price ans{-1};
        if(!OrderBook::sellOrders.empty()) {
            ans = (sellOrders.begin())->first;
        }
        return ans;
    }

    Price OrderBook::getSpread(){
        Price bid = OrderBook::getBestBid();
        Price ask = OrderBook::getBestAsk();
        Price ans{-1};

        if(bid!=-1 && ask!=-1 ){
            ans = ask-bid;
        }

        return ans;
    }

    Orderbook::OrderResult OrderBook::makeBuyOrder(Price price, Quantity quantity){
        if(price<=0 || quantity<=0) return {0,0,{},0};
        Order b;
        b.orderId = ++curOrderId;
        b.price = price;
        b.quantity = quantity;
        b.type = Type::buy;

        OrderBook::buyOrders[price].push_back(b);
        OrderBook::orders[curOrderId] = std::prev(OrderBook::buyOrders[price].end());
        Orderbook::OrderResult ans;
        ans.accepted = 1;
        ans.orderId = curOrderId;
        ans.trade = match(Type::buy);
        ans.remainQuantity = quantity;
        for(auto &i:ans.trade){
            ans.remainQuantity-=i.quantity;
        }
        return ans;
        
    }
    
    Orderbook::OrderResult OrderBook::makeSellOrder(Price price, Quantity quantity){
        if(price<=0 || quantity<=0) return {0,0,{},0};
        Order b;
        b.orderId = ++curOrderId;
        b.price = price;
        b.quantity = quantity;
        b.type = Type::sell;

        OrderBook::sellOrders[price].push_back(b);
        OrderBook::orders[curOrderId] = std::prev(OrderBook::sellOrders[price].end());
        Orderbook::OrderResult ans;
        ans.accepted = 1;
        ans.orderId = curOrderId;
        ans.trade = match(Type::sell);
        ans.remainQuantity = quantity;
        for(auto &i:ans.trade){
            ans.remainQuantity-=i.quantity;
        }
        return ans;

    }

    bool OrderBook::cancelOrder(OrderId id){
        if(OrderBook::orders.find(id)==OrderBook::orders.end()){
            return 0;
        }
        Price price = OrderBook::orders[id]->price;
        Type type = OrderBook::orders[id]->type;

        if(type==Type::buy){
            OrderBook::buyOrders[price].erase(OrderBook::orders[id]);
            if(OrderBook::buyOrders[price].empty()) OrderBook::buyOrders.erase(price);
        }
        else{
            OrderBook::sellOrders[price].erase(OrderBook::orders[id]);
            if(OrderBook::sellOrders[price].empty()) OrderBook::sellOrders.erase(price);
        }
        OrderBook::orders.erase(id);
        return 1;
    }

    std::vector<Orderbook::Trade> OrderBook::match(Type type){

        std::vector<Orderbook::Trade>ans;

        while((!OrderBook::buyOrders.empty()) && (!OrderBook::sellOrders.empty()) &&  (OrderBook::getBestBid() >= OrderBook::getBestAsk())){
            
            Orderbook::Price buySidePrice = OrderBook::getBestBid();
            Orderbook::Price sellSidePrice = OrderBook::getBestAsk();

            auto buyLevelIt = std::prev(buyOrders.end());
            auto sellLevelIt = sellOrders.begin();
            
            auto &buyOrder = buyLevelIt->second.front();
            auto &sellOrder = sellLevelIt->second.front();

            auto buyOrderid = buyOrder.orderId;
            auto sellOrderid = sellOrder.orderId;
            
            Orderbook::Price price = (type==Type::buy)?sellSidePrice : buySidePrice;
            Orderbook::Quantity quantity = std::min(buyOrder.quantity, sellOrder.quantity);

            ans.push_back(OrderBook::makeTrade(buyOrderid,sellOrderid, quantity, price));
            
            buyOrder.quantity-=quantity;
            sellOrder.quantity-=quantity;

            if(buyOrder.quantity==0){
                buyLevelIt->second.erase(OrderBook::orders[buyOrderid]);
                OrderBook::orders.erase(buyOrderid);
                if(buyLevelIt->second.empty()) OrderBook::buyOrders.erase(buyLevelIt);   
            }
            
            if(sellOrder.quantity==0){
                sellLevelIt->second.erase(OrderBook::orders[sellOrderid]);
                OrderBook::orders.erase(sellOrderid);
                if(sellLevelIt->second.empty()) OrderBook::sellOrders.erase(sellLevelIt);   
            }
        }

        return ans;

    }

    Orderbook::Trade OrderBook::makeTrade(OrderId buyId, OrderId sellId, Quantity quantity, Price price){
        Trade t;
        t.buyerOrderId = buyId;
        t.sellerOrderId = sellId;
        t.price= price;
        t.quantity = quantity;
        OrderBook::trade.push_back(t);
        return t;
    }

    void OrderBook::clear(){
        buyOrders.clear();
        sellOrders.clear();
        orders.clear();
        curOrderId = 0;
        trade.clear();
    }

    OrderBook::~OrderBook(){
        clear();
    }

    std::vector<Order> OrderBook::getBuyOrders(){
        std::vector<Order>ans;
        for(auto i = buyOrders.rbegin(); i!=buyOrders.rend(); ++i){
            for(auto j:(*i).second){
                ans.push_back(j);
            }
        }

        return ans;
    }

    std::vector<Order> OrderBook::getSellOrders(){
        std::vector<Order>ans;
        for(auto i = sellOrders.begin(); i!=sellOrders.end(); i++){
            for(auto j:(*i).second){
                ans.push_back(j);
            }
        }
    
        return ans;
    }

    std::vector<Trade> OrderBook::getTrade(){
        return trade;
    }

    std::vector<Trade>OrderBook::getTrade(std::int64_t s){
        std::vector<Trade>ans;
        for(std::int64_t i = std::max(std::int64_t{0},std::int64_t(trade.size())-s);i<std::int64_t(trade.size());i++){
            ans.push_back(trade[i]);
        }
        return ans;
    }




}

