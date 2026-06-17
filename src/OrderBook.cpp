#include<orderBook/OrderBook.hpp>
#include<algorithm>
#include<iterator>
#include<set>

namespace Orderbook{

    OrderBook::OrderBook(){
        OrderBook::buyOrders.clear();
        OrderBook::sellOrders.clear();
        // OrderBook::curOrderId = 0;
        OrderBook::orders.clear();
        trade.clear();
    }

    // OrderId OrderBook::getCurOrderId() const{
    //     return OrderBook::curOrderId;
    // }

    std::optional<Price> OrderBook::getBestBid() const{
        if(OrderBook::buyOrders.empty()) return std::nullopt;
        return (--buyOrders.end())->first;
    }

    std::optional<Price> OrderBook::getBestAsk() const{
        if(OrderBook::sellOrders.empty()) return std::nullopt;
        return (sellOrders.begin())->first;
    }

    std::optional<Price> OrderBook::getSpread() const{
        auto bid = OrderBook::getBestBid();
        auto ask = OrderBook::getBestAsk();

        if(bid && ask ){
            return *ask-*bid;
        }

        return std::nullopt;
    }

    Orderbook::OrderResult OrderBook::makeBuyOrder(Price price, Quantity quantity, OrderId curOrderId, Orderbook::UserId userid){
        if(OrderBook::orders.find(curOrderId)!=OrderBook::orders.end()) return{0,1,0,{},0, userid};
        if(price<=0 || quantity<=0) return {0,0,0,{},0, userid};
        Order b;
        b.orderId = curOrderId;
        b.price = price;
        b.quantity = quantity;
        b.type = Type::buy;
        b.UserId = userid;

        OrderBook::buyOrders[price].push_back(b);
        OrderBook::orders[curOrderId] = {Type::buy, price, std::prev(OrderBook::buyOrders[price].end())};
        Orderbook::OrderResult ans;
        ans.accepted = 1;
        ans.alreadyPresent = false;
        ans.orderId = curOrderId;
        ans.trade = match(Type::buy);
        ans.remainQuantity = quantity;
        for(auto &i:ans.trade){
            ans.remainQuantity-=i.quantity;
        }
        ans.UserId = userid;
        return ans;

    }

    Orderbook::OrderResult OrderBook::makeSellOrder(Price price, Quantity quantity, OrderId curOrderId, Orderbook::UserId userid){
        if(OrderBook::orders.find(curOrderId)!=OrderBook::orders.end()) return {0,1,0,{},0, userid};
        if(price<=0 || quantity<=0) return {0,0,0,{},0, userid};
        Order b;
        b.orderId = curOrderId;
        b.price = price;
        b.quantity = quantity;
        b.type = Type::sell;
        b.UserId = userid;

        OrderBook::sellOrders[price].push_back(b);
        OrderBook::orders[curOrderId] = {Type::sell, price, std::prev(OrderBook::sellOrders[price].end())};
        Orderbook::OrderResult ans;
        ans.accepted = 1;
        ans.alreadyPresent = false;
        ans.orderId = curOrderId;
        ans.trade = match(Type::sell);
        ans.remainQuantity = quantity;
        for(auto &i:ans.trade){
            ans.remainQuantity-=i.quantity;
        }
        ans.UserId = userid;
        return ans;

    }

    Orderbook::CancelOrderResult OrderBook::cancelOrder(OrderId id){
        auto orderIt = OrderBook::orders.find(id);
        if(orderIt==OrderBook::orders.end()){
            return {0,0,0,0, Type::buy};
        }
        auto location = orderIt->second;

        Orderbook::CancelOrderResult ans;
        ans.cancelled=1;
        ans.orderPrice = location.iterator->price;
        ans.orderQuantity = location.iterator->quantity;
        ans.type = location.iterator->type;
        ans.userid = location.iterator->UserId;

        if(location.side==Type::buy){
            auto levelIt = OrderBook::buyOrders.find(location.price);
            if(levelIt==OrderBook::buyOrders.end()) return {0,0,0,0, Type::buy};
            levelIt->second.erase(location.iterator);
            if(levelIt->second.empty()) OrderBook::buyOrders.erase(levelIt);
        }
        else{
            auto levelIt = OrderBook::sellOrders.find(location.price);
            if(levelIt==OrderBook::sellOrders.end()) return {0,0,0,0, Type::buy};
            levelIt->second.erase(location.iterator);
            if(levelIt->second.empty()) OrderBook::sellOrders.erase(levelIt);
        }
        OrderBook::orders.erase(orderIt);
        return ans;
    }

    std::vector<Orderbook::Trade> OrderBook::match(Type type){

        std::vector<Orderbook::Trade>ans;

        while((!OrderBook::buyOrders.empty()) && (!OrderBook::sellOrders.empty()) &&  (OrderBook::getBestBid().value() >= OrderBook::getBestAsk().value())){

            auto buySidePrice = OrderBook::getBestBid();
            auto sellSidePrice = OrderBook::getBestAsk();

            auto buyLevelIt = std::prev(buyOrders.end());
            auto sellLevelIt = sellOrders.begin();

            auto &buyOrder = buyLevelIt->second.front();
            auto &sellOrder = sellLevelIt->second.front();

            auto buyOrderid = buyOrder.orderId;
            auto sellOrderid = sellOrder.orderId;

            auto buyUserid = buyOrder.UserId;
            auto sellUserid = sellOrder.UserId;

            auto price = (type==Type::buy)?sellSidePrice : buySidePrice;
            Orderbook::Quantity quantity = std::min(buyOrder.quantity, sellOrder.quantity);

            ans.push_back(OrderBook::makeTrade(buyOrderid,sellOrderid, quantity, *price, buyUserid, sellUserid, buySidePrice.value(), sellSidePrice.value()));

            buyOrder.quantity-=quantity;
            sellOrder.quantity-=quantity;

            if(buyOrder.quantity==0){
                buyLevelIt->second.erase(OrderBook::orders[buyOrderid].iterator);
                OrderBook::orders.erase(buyOrderid);
                if(buyLevelIt->second.empty()) OrderBook::buyOrders.erase(buyLevelIt);
            }

            if(sellOrder.quantity==0){
                sellLevelIt->second.erase(OrderBook::orders[sellOrderid].iterator);
                OrderBook::orders.erase(sellOrderid);
                if(sellLevelIt->second.empty()) OrderBook::sellOrders.erase(sellLevelIt);
            }
        }

        return ans;

    }

    Orderbook::Trade OrderBook::makeTrade(OrderId buyId, OrderId sellId, Quantity quantity, Price price, UserId buyUserId, UserId sellUserId, Price buyerPrice, Price sellerPrice){
        Trade t;
        t.buyerOrderId = buyId;
        t.sellerOrderId = sellId;
        t.price= price;
        t.quantity = quantity;
        t.buyerUserId = buyUserId;
        t.sellerUserId = sellUserId;
        t.buyerPrice = buyerPrice;
        t.sellerPrice = sellerPrice;
        OrderBook::trade.push_back(t);
        return t;
    }

    void OrderBook::clear(){
        buyOrders.clear();
        sellOrders.clear();
        orders.clear();
        // curOrderId = 0;
        trade.clear();
    }

    OrderBook::~OrderBook(){
        clear();
    }

    std::vector<Order> OrderBook::getBuyOrders() const{
        std::vector<Order>ans;
        for(auto i = buyOrders.rbegin(); i!=buyOrders.rend(); ++i){
            for(auto j:(*i).second){
                ans.push_back(j);
            }
        }

        return ans;
    }

    std::vector<Order> OrderBook::getSellOrders() const{
        std::vector<Order>ans;
        for(auto i = sellOrders.begin(); i!=sellOrders.end(); i++){
            for(auto j:(*i).second){
                ans.push_back(j);
            }
        }

        return ans;
    }

    std::vector<Trade> OrderBook::getTrade() const{
        return trade;
    }

    std::vector<Trade>OrderBook::getTrade(std::int64_t s) const{
        std::vector<Trade>ans;
        if(s <= 0) return ans;
        auto start = std::max(std::int64_t{0},std::int64_t(trade.size())-s);
        for(auto i = static_cast<std::size_t>(start); i<trade.size(); i++){
            ans.push_back(trade[i]);
        }
        return ans;
    }

    bool OrderBook::checkInvariants() const{
        std::set<OrderId> activeOrderIds;

        auto checkSide = [&](const auto& sideOrders, Type side) {
            for(const auto& level : sideOrders){
                if(level.second.empty()) return false;

                for(auto it = level.second.begin(); it != level.second.end(); ++it){
                    const auto& order = *it;
                    if(order.quantity <= 0) return false;
                    if(order.price != level.first) return false;
                    if(order.type != side) return false;
                    if(!activeOrderIds.insert(order.orderId).second) return false;

                    auto locationIt = orders.find(order.orderId);
                    if(locationIt == orders.end()) return false;
                    const auto& location = locationIt->second;
                    if(location.side != side) return false;
                    if(location.price != level.first) return false;
                    if(location.iterator != it) return false;
                }
            }
            return true;
        };

        if(!checkSide(buyOrders, Type::buy)) return false;
        if(!checkSide(sellOrders, Type::sell)) return false;
        if(activeOrderIds.size() != orders.size()) return false;

        if(!buyOrders.empty() && !sellOrders.empty()){
            if(std::prev(buyOrders.end())->first >= sellOrders.begin()->first) return false;
        }

        for(const auto& currentTrade : trade){
            if(currentTrade.price <= 0) return false;
            if(currentTrade.quantity <= 0) return false;
        }

        return true;
    }


    bool OrderBook::checkOrder(Orderbook::OrderId id)const{
        return (OrderBook::orders.find(id)!=OrderBook::orders.end());
    }

    Orderbook::UserId OrderBook::getUserIdFromOrder(Orderbook::OrderId id)const{
        auto orderIt = OrderBook::orders.find(id);
        if(orderIt==OrderBook::orders.end()){
            return 0;
        }
        return orderIt->second.iterator->UserId;

    }

}
