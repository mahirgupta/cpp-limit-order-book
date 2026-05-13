#include "OrderBook.hpp"
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

    std::int64_t OrderBook::makeBuyOrder(Price price, Quantity quantity){
        Order b;
        b.orderId = ++curOrderId;
        b.price = price;
        b.quantity = quantity;
        b.type = Type::buy;

        OrderBook::buyOrders[price].push_back(b);
        OrderBook::orders[curOrderId] = std::prev(OrderBook::sellOrders[price].end());
        return OrderBook::match(Type::buy);
        
    }
    
    std::int64_t OrderBook::makeSellOrder(Price price, Quantity quantity){
        Order b;
        b.orderId = ++curOrderId;
        b.price = price;
        b.quantity = quantity;
        b.type = Type::sell;

        OrderBook::sellOrders[price].push_back(b);
        OrderBook::orders[curOrderId] = std::prev(OrderBook::sellOrders[price].end());

        return OrderBook::match(Type::sell);

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

    std::int64_t OrderBook::match(Type type){
        Price npr = -1;

        std::int64_t count=0;

        while(OrderBook::getSpread()!=npr && (OrderBook::getBestBid() >= OrderBook::getBestAsk())){
            Order buyside = OrderBook::buyOrders[getBestBid()].front();
            Order sellside = OrderBook::sellOrders[getBestAsk()].front();
            Price price = (type==Type::buy)?sellside.price : buyside.price;
            Quantity quantity = std::min(buyside.quantity, sellside.quantity);
            OrderBook::makeTrade(buyside.orderId, sellside.orderId, quantity,price);
            count++;

            OrderBook::buyOrders[getBestBid()].front().quantity-=quantity;
            OrderBook::sellOrders[getBestAsk()].front().quantity-=quantity;
            
            if(OrderBook::buyOrders[getBestBid()].front().quantity==0){
                OrderBook::buyOrders[OrderBook::getBestBid()].erase(OrderBook::orders[buyside.orderId]);
                OrderBook::orders.erase(buyside.orderId);
                if(OrderBook::buyOrders[OrderBook::getBestBid()].empty()) OrderBook::buyOrders.erase(getBestBid());
        
            }
            if(OrderBook::sellOrders[getBestAsk()].front().quantity==0){
                OrderBook::sellOrders[OrderBook::getBestAsk()].erase(OrderBook::orders[sellside.orderId]);
                OrderBook::orders.erase(sellside.orderId);
                if(OrderBook::sellOrders[OrderBook::getBestAsk()].empty()) OrderBook::sellOrders.erase(getBestAsk());
        
            }
        }

        return count;

    }

    void OrderBook::makeTrade(OrderId buyId, OrderId sellId, Quantity quantity, Price price){
        Trade t;
        t.buyerOrderId = buyId;
        t.sellerOrderId = sellId;
        t.price= price;
        t.quantity = quantity;
        OrderBook::trade.push_back(t);
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
        for(auto i = buyOrders.rbegin(); i!=buyOrders.rend(); i--){
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

