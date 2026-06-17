#include<exchange/Exchange.hpp>

#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>

#define CHECK(condition)                                                   \
    do {                                                                   \
        if (!(condition)) {                                                 \
            std::cerr << "CHECK failed at " << __FILE__ << ":" << __LINE__ \
                      << " -> " << #condition << std::endl;                \
            std::exit(1);                                                   \
        }                                                                  \
    } while (false)

#define RUN_TEST(test_name)                    \
    do {                                       \
        std::cout << "Running " #test_name;    \
        test_name();                           \
        std::cout << " ... OK\n";              \
    } while (false)

class TestExchange : public Exch::Exchange{
public:
    Exch::ExchangeOrderResult buy(const Orderbook::Symbol& symbol, Orderbook::Price price, Orderbook::Quantity quantity){
        return Exch::Exchange::buy(symbol, price, quantity, fundedBuyer());
    }

    Exch::ExchangeOrderResult sell(const Orderbook::Symbol& symbol, Orderbook::Price price, Orderbook::Quantity quantity){
        return Exch::Exchange::sell(symbol, price, quantity, fundedSeller(symbol));
    }

    Exch::CancelResult cancelOrder(Orderbook::OrderId orderId){
        return Exch::Exchange::cancelOrder(orderId, orderOwner(orderId));
    }

private:
    Orderbook::UserId fundedBuyer(){
        const auto userId = addUser();
        depositCash(1'000'000'000, userId);
        return userId;
    }

    Orderbook::UserId fundedSeller(const Orderbook::Symbol& symbol){
        const auto userId = addUser();
        if(hasSymbol(symbol)){
            depositPosition(1'000'000'000, symbol, userId);
        }
        return userId;
    }

    Orderbook::UserId orderOwner(Orderbook::OrderId orderId) const{
        for(const auto& symbol:getSymbol()){
            for(const auto& order:getBuyOrders(symbol)){
                if(order.orderId==orderId) return order.UserId;
            }
            for(const auto& order:getSellOrders(symbol)){
                if(order.orderId==orderId) return order.UserId;
            }
        }
        return 0;
    }
};

Exch::account accountOf(const Exch::Exchange& ex, Orderbook::UserId userId){
    auto account = ex.getAccount(userId);
    CHECK(account);
    return account.value();
}

Orderbook::Quantity positionOf(const Exch::account& account, const Orderbook::Symbol& symbol){
    auto it = account.positions.find(symbol);
    if(it==account.positions.end()) return 0;
    return it->second;
}

Orderbook::Quantity reservedPositionOf(const Exch::account& account, const Orderbook::Symbol& symbol){
    auto it = account.reservedPositions.find(symbol);
    if(it==account.reservedPositions.end()) return 0;
    return it->second;
}



void addSymbol(){
    TestExchange ex;
    CHECK(ex.addSymbol("AAPL"));
    CHECK(ex.hasSymbol("AAPL"));
    CHECK(ex.checkInvariant());
}

void duplicateSymbolRejected(){
    TestExchange ex;
    CHECK(ex.addSymbol("AAPL"));
    CHECK(!ex.addSymbol("AAPL"));
    CHECK(ex.checkInvariant());
}

void symbolList(){
    TestExchange ex;
    ex.addSymbol("MSFT");
    ex.addSymbol("AAPL");
    auto symbols = ex.getSymbol();
    CHECK(symbols.size()==2);
    CHECK(symbols[0]=="AAPL");
    CHECK(symbols[1]=="MSFT"); // make sure symbols are keys so should in sorted order
    CHECK(ex.checkInvariant());
}

void rejectBuyFromUnknownSymbol(){
    TestExchange ex;
    auto r = ex.buy("AAPL",100,10);
    CHECK(r.accepted==false);
    CHECK(r.orderId==0);
    CHECK(r.remainQuantity==0);
    CHECK(r.trades.empty());
    CHECK(!ex.getBestBid("AAPL"));
    CHECK(ex.checkInvariant());
}

void rejectSellFromUnknownSymbol(){
    TestExchange ex;
    auto r = ex.sell("AAPL",100,10);
    CHECK(r.accepted==false);
    CHECK(r.orderId==0);
    CHECK(r.remainQuantity==0);
    CHECK(r.trades.empty());
    CHECK(!ex.getBestAsk("AAPL"));
    CHECK(ex.checkInvariant());
}

void rejectInvalidBuyPriceOrQuantity(){
    TestExchange ex;
    ex.addSymbol("AAPL");

    CHECK(!ex.buy("AAPL",0,10).accepted);
    CHECK(!ex.buy("AAPL",-1,10).accepted);
    CHECK(!ex.buy("AAPL",100,0).accepted);
    CHECK(!ex.buy("AAPL",100,-1).accepted);
    CHECK(ex.checkInvariant());
}

void rejectInvalidSellPriceOrQuantity(){
    TestExchange ex;
    ex.addSymbol("AAPL");

    CHECK(!ex.sell("AAPL",0,10).accepted);
    CHECK(!ex.sell("AAPL",-1,10).accepted);
    CHECK(!ex.sell("AAPL",100,0).accepted);
    CHECK(!ex.sell("AAPL",100,-1).accepted);
    CHECK(ex.checkInvariant());
}

void buyRestsInCorrectSymbol(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    auto r = ex.buy("AAPL",100,10);
    CHECK(r.accepted);
    CHECK(r.orderId==1);
    CHECK(r.remainQuantity==10);
    CHECK(r.symbol=="AAPL");
    CHECK(r.trades.empty());
    CHECK(ex.getBestBid("AAPL").value()==100);
    CHECK(ex.getBuyOrders("AAPL").size()==1);
    CHECK(ex.checkInvariant());
}

void sellRestsInCorrectSymbol(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    auto r = ex.sell("AAPL",100,10);
    CHECK(r.accepted);
    CHECK(r.orderId==1);
    CHECK(r.remainQuantity==10);
    CHECK(r.symbol=="AAPL");
    CHECK(r.trades.empty());
    CHECK(ex.getBestAsk("AAPL").value()==100);
    CHECK(ex.getSellOrders("AAPL").size()==1);
    CHECK(ex.checkInvariant());
}

void symbolsAreIsolated(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    ex.addSymbol("MSFT");

    auto r1 = ex.buy("AAPL",100,10);
    CHECK(ex.checkInvariant());
    auto r2 = ex.sell("MSFT",90,5);

    CHECK(r1.accepted);
    CHECK(r2.accepted);

    CHECK(r1.trades.empty());
    CHECK(r2.trades.empty());

    CHECK(ex.getBestBid("AAPL").value()==100);
    CHECK(ex.getBestAsk("MSFT").value()==90);

    CHECK(ex.getTrades("AAPL").empty());
    CHECK(ex.getTrades("MSFT").empty());
    CHECK(ex.checkInvariant());
}






void sameSymbolBuyCrossesSell(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    auto s = ex.sell("AAPL",100,5);
    CHECK(ex.checkInvariant());
    auto b = ex.buy("AAPL",105,2);
    CHECK(ex.checkInvariant());

    CHECK(b.accepted);
    CHECK(b.remainQuantity==0);
    CHECK(!b.trades.empty());
    CHECK(b.trades.size()==1);
    CHECK(b.trades[0].symbol=="AAPL");
    CHECK(b.trades[0].buyerOrderId == b.orderId);
    CHECK(b.trades[0].sellerOrderId == s.orderId);
    CHECK(b.trades[0].price == 100);
    CHECK(b.trades[0].quantity == 2);
    CHECK(ex.getSellOrders("AAPL")[0].quantity==3);
}

void sameSymbolSellCrossesBuy(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    auto b = ex.buy("AAPL",100,5);
    CHECK(ex.checkInvariant());
    auto s = ex.sell("AAPL",95,2);
    CHECK(ex.checkInvariant());

    CHECK(s.accepted);
    CHECK(s.remainQuantity==0);
    CHECK(!s.trades.empty());
    CHECK(s.trades.size()==1);
    CHECK(s.trades[0].symbol=="AAPL");
    CHECK(s.trades[0].buyerOrderId == b.orderId);
    CHECK(s.trades[0].sellerOrderId == s.orderId);
    CHECK(s.trades[0].price == 100);
    CHECK(s.trades[0].quantity == 2);
    CHECK(ex.getBuyOrders("AAPL")[0].quantity==3);
}

void tradePriceIsRestingPrice(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    ex.sell("AAPL",100,5);
    auto b = ex.buy("AAPL",110,5);
    CHECK(ex.checkInvariant());

    CHECK(b.trades.size()==1);
    CHECK(b.trades[0].price==100);

    ex.addSymbol("MSFT");
    ex.buy("MSFT",100,5);
    CHECK(ex.checkInvariant());
    auto s = ex.sell("MSFT",90,5);

    CHECK(s.trades.size()==1);
    CHECK(s.trades[0].price==100);

}

void globalOrderIdsAcrossSymbols(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    ex.addSymbol("MSFT");

    auto a = ex.buy("AAPL",100,10);
    CHECK(ex.checkInvariant());
    auto m = ex.sell("MSFT",200,5);
    CHECK(ex.checkInvariant());

    CHECK(a.orderId==1);
    CHECK(m.orderId==2);
}

void cancelActiveBuyByGlobalIds(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    auto b = ex.buy("AAPL",100,10);
    CHECK(ex.checkInvariant());
    CHECK(b.accepted);
    CHECK(!ex.getBuyOrders("AAPL").empty());
    CHECK(ex.getBestBid("AAPL").value()==100);
    auto c = ex.cancelOrder(b.orderId);
    CHECK(ex.checkInvariant());

    CHECK(c.cancelled);
    CHECK(c.symbol=="AAPL");
    CHECK(ex.getBuyOrders("AAPL").empty());
    CHECK(!ex.getBestBid("AAPL"));
}

void cancelActiveSellByGlobalIds(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    auto s = ex.sell("AAPL",100,10);
    CHECK(ex.checkInvariant());
    CHECK(s.accepted);
    CHECK(!ex.getSellOrders("AAPL").empty());
    CHECK(ex.getBestAsk("AAPL").value()==100);
    auto c = ex.cancelOrder(s.orderId);
    CHECK(ex.checkInvariant());

    CHECK(c.cancelled);
    CHECK(c.symbol=="AAPL");
    CHECK(ex.getSellOrders("AAPL").empty());
    CHECK(!ex.getBestAsk("AAPL"));
}

void cancelUnknownOrderIdFails(){
    TestExchange ex;
    auto c = ex.cancelOrder(999);
    CHECK(ex.checkInvariant());

    CHECK(!c.cancelled);

}

void cancelFullyFilledIncomingOrderFails(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    ex.sell("AAPL",100,5);
    auto b = ex.buy("AAPL",100,5);
    CHECK(ex.checkInvariant());
    CHECK(b.accepted);
    auto c = ex.cancelOrder(b.orderId);
    CHECK(ex.checkInvariant());
    CHECK(!c.cancelled);
}

void cancelFullyFilledRestingOrderFails(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    auto b = ex.buy("AAPL",100,5);
    auto s = ex.sell("AAPL",100,5);
    CHECK(ex.checkInvariant());
    CHECK(b.accepted);
    CHECK(s.accepted);

    auto c = ex.cancelOrder(b.orderId);
    CHECK(ex.checkInvariant());

    CHECK(!c.cancelled);
    CHECK(ex.getBuyOrders("AAPL").empty());
    CHECK(ex.getSellOrders("AAPL").empty());
}

void cancelPartialyFilledRestingBuy(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    auto b = ex.buy("AAPL",100,10);
    CHECK(ex.checkInvariant());
    CHECK(b.accepted);
    auto s = ex.sell("AAPL",100,4);
    CHECK(ex.checkInvariant());
    CHECK(s.accepted);
    CHECK(s.trades.size()==1);
    CHECK(ex.getBuyOrders("AAPL").size()==1);
    CHECK(ex.getBuyOrders("AAPL")[0].quantity==6);
    auto c = ex.cancelOrder(b.orderId);
    CHECK(ex.checkInvariant());
    CHECK(c.cancelled);
    CHECK(c.symbol=="AAPL");
    CHECK(ex.getBuyOrders("AAPL").empty());
}





void cancelPartialyFilledRestingSell(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    auto s = ex.sell("AAPL",100,10);
    CHECK(ex.checkInvariant());
    CHECK(s.accepted);
    auto b = ex.buy("AAPL",100,4);
    CHECK(ex.checkInvariant());
    CHECK(b.accepted);
    CHECK(b.trades.size()==1);
    CHECK(ex.getSellOrders("AAPL").size()==1);
    CHECK(ex.getSellOrders("AAPL")[0].quantity==6);
    auto c = ex.cancelOrder(s.orderId);
    CHECK(ex.checkInvariant());
    CHECK(c.cancelled);
    CHECK(c.symbol=="AAPL");
    CHECK(ex.getSellOrders("AAPL").empty());
}

void cancelOneSymbolDoesNotAffectOtherSymbol(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    ex.addSymbol("MSFT");

    auto a = ex.buy("AAPL",100,10);
    auto m = ex.buy("MSFT",200,5);
    CHECK(ex.checkInvariant());

    CHECK(a.accepted);
    CHECK(m.accepted);
    CHECK(ex.getBuyOrders("AAPL").size()==1);
    CHECK(ex.getBuyOrders("MSFT").size()==1);

    auto c = ex.cancelOrder(a.orderId);
    CHECK(ex.checkInvariant());
    CHECK(c.cancelled);
    CHECK(c.symbol=="AAPL");
    CHECK(ex.getBuyOrders("AAPL").empty());
    CHECK(ex.getBestBid("MSFT").value()==200);
    CHECK(ex.getBuyOrders("MSFT").size()==1);
}

void tradesAreSymbolSpecific(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    ex.addSymbol("MSFT");

    ex.sell("AAPL",100,5);
    ex.buy("AAPL",100,5);
    CHECK(ex.checkInvariant());

    ex.sell("MSFT",200,3);
    ex.buy("MSFT",200,3);
    CHECK(ex.checkInvariant());

    CHECK(ex.getTrades("AAPL").size()==1);
    CHECK(ex.getTrades("MSFT").size()==1);
    CHECK(ex.getTrades("AAPL")[0].symbol=="AAPL");
    CHECK(ex.getTrades("MSFT")[0].symbol=="MSFT");
}

void getTradesUnknownSymbolReturnsEmpty(){
    TestExchange ex;
    auto trade = ex.getTrades("UNKNOWN");
    CHECK(trade.empty());
    CHECK(ex.checkInvariant());
}

void bestBidAskSpreadPerSymbol(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    ex.addSymbol("MSFT");

    ex.buy("AAPL",100,10);
    ex.sell("AAPL",105,5);
    CHECK(ex.checkInvariant());

    ex.buy("MSFT",200,10);
    ex.sell("MSFT",220,5);
    CHECK(ex.checkInvariant());

    CHECK(ex.getBestBid("AAPL").value()==100);
    CHECK(ex.getBestAsk("AAPL").value()==105);
    CHECK(ex.getSpread("AAPL").value()==5);

    CHECK(ex.getBestBid("MSFT").value()==200);
    CHECK(ex.getBestAsk("MSFT").value()==220);
    CHECK(ex.getSpread("MSFT").value()==20);

}

void clearOneSymbol(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    ex.addSymbol("MSFT");
    auto a = ex.buy("AAPL",100,10);
    auto m = ex.buy("MSFT",200,5);
    CHECK(ex.checkInvariant());
    CHECK(a.accepted);
    CHECK(m.accepted);

    CHECK(!ex.getBuyOrders("AAPL").empty());
    CHECK(!ex.getBuyOrders("MSFT").empty());
    ex.clearSymbol("AAPL");
    CHECK(ex.checkInvariant());
    CHECK(ex.getBuyOrders("AAPL").empty());
    CHECK(!ex.getBuyOrders("MSFT").empty());

    auto c = ex.cancelOrder(a.orderId);
    CHECK(ex.checkInvariant());
    CHECK(!c.cancelled);

    auto d = ex.cancelOrder(m.orderId);
    CHECK(ex.checkInvariant());
    CHECK(d.cancelled);

}

void clearUnknownSymbolDoesNothing(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    ex.buy("AAPL",100,10);
    CHECK(ex.checkInvariant());
    CHECK(ex.getBuyOrders("AAPL").size()==1);
    CHECK(ex.getBuyOrders("AAPL")[0].price==100);
    CHECK(ex.getBuyOrders("AAPL")[0].quantity==10);

    ex.clearSymbol("UNKNOWN");
    CHECK(ex.checkInvariant());
    CHECK(ex.getBuyOrders("AAPL").size()==1);
    CHECK(ex.getBuyOrders("AAPL")[0].price==100);
    CHECK(ex.getBuyOrders("AAPL")[0].quantity==10);
}

void clearAll(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    ex.addSymbol("MSFT");

    auto a = ex.buy("AAPL",100,10);
    auto m = ex.buy("MSFT",200,5);
    CHECK(ex.checkInvariant());

    CHECK(ex.getBuyOrders("AAPL").size()==1);
    CHECK(ex.getBuyOrders("MSFT").size()==1);
    ex.clearAll();
    CHECK(ex.checkInvariant());

    CHECK(ex.getBuyOrders("AAPL").empty());
    CHECK(ex.getBuyOrders("MSFT").empty());

    auto c1 = ex.cancelOrder(a.orderId);
    auto c2 = ex.cancelOrder(m.orderId);
    CHECK(ex.checkInvariant());

    CHECK(!c1.cancelled);
    CHECK(!c2.cancelled);

    auto n = ex.buy("AAPL", 101, 1);
    CHECK(ex.checkInvariant());
    CHECK(n.orderId == 1);
}

void addSymbolAfterClearAll(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    CHECK(ex.checkInvariant());
    ex.clearAll();
    CHECK(ex.checkInvariant());
    CHECK(ex.hasSymbol("AAPL"));

}

void multiLevelSweepOnOneSymbol(){
    TestExchange ex;
    ex.addSymbol("AAPL");

    ex.sell("AAPL",100,5);
    ex.sell("AAPL",101,5);
    ex.sell("AAPL",102,5);
    CHECK(ex.checkInvariant());

    auto b = ex.buy("AAPL",102,12);
    CHECK(ex.checkInvariant());

    CHECK(b.trades.size()==3);
    CHECK(b.trades[0].quantity==5);
    CHECK(b.trades[0].price==100);
    CHECK(b.trades[1].quantity==5);
    CHECK(b.trades[1].price==101);
    CHECK(b.trades[2].quantity==2);
    CHECK(b.trades[2].price==102);

    CHECK(ex.getSellOrders("AAPL").size()==1);
    CHECK(ex.getSellOrders("AAPL")[0].price==102);
    CHECK(ex.getSellOrders("AAPL")[0].quantity==3);
}





void FIFOSamePriceThroughExchange(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    auto s1 = ex.sell("AAPL", 100, 5);
    auto s2 = ex.sell("AAPL", 100, 7);
    auto b = ex.buy("AAPL", 100, 6);
    CHECK(ex.checkInvariant());

    CHECK(b.trades.size()==2);
    CHECK(b.trades[0].sellerOrderId == s1.orderId);
    CHECK(b.trades[0].quantity==5);
    CHECK(b.trades[1].sellerOrderId == s2.orderId);
    CHECK(b.trades[1].quantity==1);
    CHECK(ex.getSellOrders("AAPL").size()==1);
    CHECK(ex.getSellOrders("AAPL")[0].quantity==6);
}

void pricePriorityThroughExchange(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    auto b1 = ex.buy("AAPL", 99, 5);
    auto b2 = ex.buy("AAPL", 101, 5);
    CHECK(ex.checkInvariant());
    auto s = ex.sell("AAPL", 99, 5);
    CHECK(ex.checkInvariant());

    CHECK(s.trades.size()==1);
    CHECK(s.trades[0].buyerOrderId==b2.orderId);
    CHECK(s.trades[0].price==101);

}

void orderIdsDoesNotAdvanceOnRejectedUnknownSymbol(){
    TestExchange ex;
    auto bad = ex.buy("UNKNOWN",100,10);
    CHECK(ex.checkInvariant());
    CHECK(!bad.accepted);
    ex.addSymbol("AAPL");
    auto good = ex.buy("AAPL",100,10);
    CHECK(ex.checkInvariant());
    CHECK(good.accepted);
    CHECK(good.orderId==1);
}

void orderIdDoesNotAdvanceOnInvalidPriceQuantity(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    ex.buy("AAPL",0,10);
    ex.buy("AAPL",100,0);
    CHECK(ex.checkInvariant());
    auto good = ex.buy("AAPL",100,1);
    CHECK(ex.checkInvariant());
    CHECK(good.accepted);
    CHECK(good.orderId==1);
}

void checkVisibleExchangeInvariants(Exch::Exchange& ex, const std::vector<Orderbook::Symbol>& symbols){
    for(auto &sym:symbols){
        auto buyOrders = ex.getBuyOrders(sym);
        auto sellOrders = ex.getSellOrders(sym);

        bool firstBuy = true;
        Orderbook::Price lastBuyPrice = 0;
        for(auto i:buyOrders){
            CHECK(i.quantity>0);
            CHECK(i.price>0);
            CHECK(i.type==Orderbook::Type::buy);
            if(firstBuy){
                CHECK(i.price==ex.getBestBid(sym).value());
                firstBuy = false;
            }
            else{
                CHECK(i.price<=lastBuyPrice);
            }
            lastBuyPrice = i.price;
        }

        bool firstSell = true;
        Orderbook::Price lastSellPrice = 0;
        for(auto i:sellOrders){
            CHECK(i.quantity>0);
            CHECK(i.price>0);
            CHECK(i.type==Orderbook::Type::sell);
            if(firstSell){
                CHECK(i.price==ex.getBestAsk(sym).value());
                firstSell = false;
            }
            else{
                CHECK(i.price>=lastSellPrice);
            }
            lastSellPrice = i.price;
        }

        if(buyOrders.empty()) CHECK(!ex.getBestBid(sym));
        if(sellOrders.empty()) CHECK(!ex.getBestAsk(sym));

        if(!buyOrders.empty() && !sellOrders.empty()){
            CHECK(ex.getBestBid(sym).value()<ex.getBestAsk(sym).value());
            CHECK(ex.getSpread(sym).value()==ex.getBestAsk(sym).value()-ex.getBestBid(sym).value());
        }
        else{
            CHECK(!ex.getSpread(sym));
        }

        auto trades = ex.getTrades(sym);
        for(auto i:trades){
            CHECK(i.symbol==sym);
            CHECK(i.buyerOrderId>0);
            CHECK(i.sellerOrderId>0);
            CHECK(i.price>0);
            CHECK(i.quantity>0);
        }
    }
    CHECK(ex.checkInvariant());
}

void randomizedExchangeOperationsPreserveInvariants(){
    TestExchange ex;
    std::vector<Orderbook::Symbol> symbols = {"AAPL","MSFT","TSLA"};
    std::vector<Orderbook::OrderId> candidateOrderIds;
    std::mt19937 rng(42);

    for(auto &i:symbols){
        CHECK(ex.addSymbol(i));
    }

    for(int step = 0; step<5000; ++step){
        const auto symbolIndex = static_cast<std::size_t>(rng()%symbols.size());
        const auto sym = symbols[symbolIndex];
        const auto op = static_cast<int>(rng()%4);
        const auto price = static_cast<Orderbook::Price>(90 + (rng()%30));
        const auto quantity = static_cast<Orderbook::Quantity>(1 + (rng()%10));

        if(op==0){
            auto result = ex.buy(sym,price,quantity);
            CHECK(ex.checkInvariant());
            CHECK(result.accepted);
            if(result.remainQuantity>0){
                candidateOrderIds.push_back(result.orderId);
            }
        }
        else if(op==1){
            auto result = ex.sell(sym,price,quantity);
            CHECK(ex.checkInvariant());
            CHECK(result.accepted);
            if(result.remainQuantity>0){
                candidateOrderIds.push_back(result.orderId);
            }
        }
        else if(op==2 && !candidateOrderIds.empty()){
            const auto index = static_cast<std::size_t>(rng()%candidateOrderIds.size());
            ex.cancelOrder(candidateOrderIds[index]);
            CHECK(ex.checkInvariant());
        }
        else{
            ex.clearSymbol(sym);
            CHECK(ex.checkInvariant());
        }

        checkVisibleExchangeInvariants(ex,symbols);
    }
}



void clearSymbolTradeHistoryTest(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    ex.sell("AAPL", 100, 5);
    ex.buy("AAPL", 100, 5);
    CHECK(ex.getTrades("AAPL").size() == 1);
    ex.clearSymbol("AAPL");
    CHECK(ex.getTrades("AAPL").empty());
}


void clearAllDuplicateSymbolSemeticsTest(){
    TestExchange ex;
    ex.addSymbol("AAPL");
    ex.clearAll();
    CHECK(ex.hasSymbol("AAPL"));
    CHECK(!ex.addSymbol("AAPL"));
}

void buyOrderReservesCash(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    const auto user = ex.addUser();
    CHECK(ex.depositCash(1000, user));

    auto result = ex.buy("AAPL", 100, 5, user);
    CHECK(result.accepted);
    CHECK(result.remainQuantity==5);
    CHECK(ex.getBuyOrders("AAPL").size()==1);

    auto account = accountOf(ex, user);
    CHECK(account.cash==500);
    CHECK(account.reservedCash==500);
    CHECK(account.orders.size()==1);
    CHECK(ex.checkInvariant());
}

void sellOrderReservesPosition(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    const auto user = ex.addUser();
    CHECK(ex.depositPosition(10, "AAPL", user)==10);

    auto result = ex.sell("AAPL", 100, 6, user);
    CHECK(result.accepted);
    CHECK(result.remainQuantity==6);
    CHECK(ex.getSellOrders("AAPL").size()==1);

    auto account = accountOf(ex, user);
    CHECK(positionOf(account, "AAPL")==4);
    CHECK(reservedPositionOf(account, "AAPL")==6);
    CHECK(account.orders.size()==1);
    CHECK(ex.checkInvariant());
}

void buyRejectsWhenAvailableCashIsInsufficient(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    const auto user = ex.addUser();
    CHECK(ex.depositCash(100, user));

    auto result = ex.buy("AAPL", 100, 2, user);
    CHECK(!result.accepted);
    CHECK(result.message=="Insufficient Cash");
    CHECK(ex.getBuyOrders("AAPL").empty());

    auto account = accountOf(ex, user);
    CHECK(account.cash==100);
    CHECK(account.reservedCash==0);
    CHECK(account.orders.empty());
    CHECK(ex.checkInvariant());
}

void sellRejectsWhenAvailablePositionIsInsufficient(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    const auto user = ex.addUser();
    CHECK(ex.depositPosition(1, "AAPL", user)==1);

    auto result = ex.sell("AAPL", 100, 2, user);
    CHECK(!result.accepted);
    CHECK(result.message=="Insufficient Quantity");
    CHECK(ex.getSellOrders("AAPL").empty());

    auto account = accountOf(ex, user);
    CHECK(positionOf(account, "AAPL")==1);
    CHECK(reservedPositionOf(account, "AAPL")==0);
    CHECK(account.orders.empty());
    CHECK(ex.checkInvariant());
}

void fullFillSettlesBuyerAndSellerAccounts(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    const auto buyer = ex.addUser();
    const auto seller = ex.addUser();
    CHECK(ex.depositCash(1000, buyer));
    CHECK(ex.depositPosition(5, "AAPL", seller)==5);

    auto sell = ex.sell("AAPL", 100, 5, seller);
    auto buy = ex.buy("AAPL", 100, 5, buyer);
    CHECK(sell.accepted);
    CHECK(buy.accepted);
    CHECK(buy.trades.size()==1);
    CHECK(ex.getBuyOrders("AAPL").empty());
    CHECK(ex.getSellOrders("AAPL").empty());

    auto buyerAccount = accountOf(ex, buyer);
    CHECK(buyerAccount.cash==500);
    CHECK(buyerAccount.reservedCash==0);
    CHECK(positionOf(buyerAccount, "AAPL")==5);

    auto sellerAccount = accountOf(ex, seller);
    CHECK(sellerAccount.cash==500);
    CHECK(positionOf(sellerAccount, "AAPL")==0);
    CHECK(reservedPositionOf(sellerAccount, "AAPL")==0);
    CHECK(ex.checkInvariant());
}

void partialFillLeavesRemainingBuyReserved(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    const auto buyer = ex.addUser();
    const auto seller = ex.addUser();
    CHECK(ex.depositCash(1000, buyer));
    CHECK(ex.depositPosition(3, "AAPL", seller)==3);

    auto buy = ex.buy("AAPL", 100, 5, buyer);
    auto sell = ex.sell("AAPL", 100, 3, seller);
    CHECK(buy.accepted);
    CHECK(sell.accepted);
    CHECK(sell.trades.size()==1);
    CHECK(ex.getBuyOrders("AAPL").size()==1);
    CHECK(ex.getBuyOrders("AAPL")[0].quantity==2);

    auto buyerAccount = accountOf(ex, buyer);
    CHECK(buyerAccount.cash==500);
    CHECK(buyerAccount.reservedCash==200);
    CHECK(positionOf(buyerAccount, "AAPL")==3);

    auto sellerAccount = accountOf(ex, seller);
    CHECK(sellerAccount.cash==300);
    CHECK(positionOf(sellerAccount, "AAPL")==0);
    CHECK(reservedPositionOf(sellerAccount, "AAPL")==0);
    CHECK(ex.checkInvariant());
}

void partialFillLeavesRemainingSellReserved(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    const auto buyer = ex.addUser();
    const auto seller = ex.addUser();
    CHECK(ex.depositCash(1000, buyer));
    CHECK(ex.depositPosition(5, "AAPL", seller)==5);

    auto sell = ex.sell("AAPL", 100, 5, seller);
    auto buy = ex.buy("AAPL", 100, 3, buyer);
    CHECK(sell.accepted);
    CHECK(buy.accepted);
    CHECK(buy.trades.size()==1);
    CHECK(ex.getSellOrders("AAPL").size()==1);
    CHECK(ex.getSellOrders("AAPL")[0].quantity==2);

    auto buyerAccount = accountOf(ex, buyer);
    CHECK(buyerAccount.cash==700);
    CHECK(buyerAccount.reservedCash==0);
    CHECK(positionOf(buyerAccount, "AAPL")==3);

    auto sellerAccount = accountOf(ex, seller);
    CHECK(sellerAccount.cash==300);
    CHECK(positionOf(sellerAccount, "AAPL")==0);
    CHECK(reservedPositionOf(sellerAccount, "AAPL")==2);
    CHECK(ex.checkInvariant());
}

void priceImprovementRefundsBuyerCash(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    const auto buyer = ex.addUser();
    const auto seller = ex.addUser();
    CHECK(ex.depositCash(1000, buyer));
    CHECK(ex.depositPosition(5, "AAPL", seller)==5);

    auto sell = ex.sell("AAPL", 90, 5, seller);
    auto buy = ex.buy("AAPL", 100, 5, buyer);
    CHECK(sell.accepted);
    CHECK(buy.accepted);
    CHECK(buy.trades.size()==1);
    CHECK(buy.trades[0].price==90);

    auto buyerAccount = accountOf(ex, buyer);
    CHECK(buyerAccount.cash==550);
    CHECK(buyerAccount.reservedCash==0);
    CHECK(positionOf(buyerAccount, "AAPL")==5);

    auto sellerAccount = accountOf(ex, seller);
    CHECK(sellerAccount.cash==450);
    CHECK(positionOf(sellerAccount, "AAPL")==0);
    CHECK(reservedPositionOf(sellerAccount, "AAPL")==0);
    CHECK(ex.checkInvariant());
}

void withdrawUsesAvailableCashOnly(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    const auto user = ex.addUser();
    CHECK(ex.depositCash(1000, user));
    CHECK(ex.buy("AAPL", 100, 5, user).accepted);

    CHECK(!ex.withdrawCash(600, user));
    CHECK(ex.withdrawCash(500, user));

    auto account = accountOf(ex, user);
    CHECK(account.cash==0);
    CHECK(account.reservedCash==500);
    CHECK(ex.checkInvariant());
}

void cancelBuyReleasesReservedCash(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    const auto user = ex.addUser();
    CHECK(ex.depositCash(1000, user));
    auto buy = ex.buy("AAPL", 100, 5, user);
    CHECK(buy.accepted);

    auto cancel = ex.cancelOrder(buy.orderId, user);
    CHECK(cancel.cancelled);

    auto account = accountOf(ex, user);
    CHECK(account.cash==1000);
    CHECK(account.reservedCash==0);
    CHECK(account.orders.empty());
    CHECK(ex.getBuyOrders("AAPL").empty());
    CHECK(ex.checkInvariant());
}

void cancelSellReleasesReservedPosition(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    const auto user = ex.addUser();
    CHECK(ex.depositPosition(5, "AAPL", user)==5);
    auto sell = ex.sell("AAPL", 100, 5, user);
    CHECK(sell.accepted);

    auto cancel = ex.cancelOrder(sell.orderId, user);
    CHECK(cancel.cancelled);

    auto account = accountOf(ex, user);
    CHECK(positionOf(account, "AAPL")==5);
    CHECK(reservedPositionOf(account, "AAPL")==0);
    CHECK(account.orders.empty());
    CHECK(ex.getSellOrders("AAPL").empty());
    CHECK(ex.checkInvariant());
}

void clearSymbolReleasesOnlyThatSymbolReservations(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    ex.addSymbol("MSFT");
    const auto user = ex.addUser();
    CHECK(ex.depositCash(1000, user));
    CHECK(ex.depositPosition(10, "MSFT", user)==10);
    CHECK(ex.buy("AAPL", 100, 5, user).accepted);
    CHECK(ex.sell("MSFT", 200, 4, user).accepted);

    ex.clearSymbol("AAPL");

    auto account = accountOf(ex, user);
    CHECK(account.cash==1000);
    CHECK(account.reservedCash==0);
    CHECK(positionOf(account, "MSFT")==6);
    CHECK(reservedPositionOf(account, "MSFT")==4);
    CHECK(ex.getBuyOrders("AAPL").empty());
    CHECK(ex.getSellOrders("MSFT").size()==1);
    CHECK(ex.checkInvariant());
}

void clearAllReleasesAllReservations(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    ex.addSymbol("MSFT");
    const auto user = ex.addUser();
    CHECK(ex.depositCash(1000, user));
    CHECK(ex.depositPosition(10, "MSFT", user)==10);
    CHECK(ex.buy("AAPL", 100, 5, user).accepted);
    CHECK(ex.sell("MSFT", 200, 4, user).accepted);

    ex.clearAll();

    auto account = accountOf(ex, user);
    CHECK(account.cash==1000);
    CHECK(account.reservedCash==0);
    CHECK(positionOf(account, "MSFT")==10);
    CHECK(reservedPositionOf(account, "MSFT")==0);
    CHECK(ex.getBuyOrders("AAPL").empty());
    CHECK(ex.getSellOrders("MSFT").empty());
    CHECK(ex.checkInvariant());
}

void selfTradeRegistersAndSettlesBothSides(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    const auto user = ex.addUser();
    CHECK(ex.depositCash(1000, user));
    CHECK(ex.depositPosition(5, "AAPL", user)==5);

    auto buy = ex.buy("AAPL", 100, 5, user);
    auto sell = ex.sell("AAPL", 90, 3, user);
    CHECK(buy.accepted);
    CHECK(sell.accepted);
    CHECK(sell.trades.size()==1);
    CHECK(ex.getTrades("AAPL").size()==1);
    CHECK(sell.trades[0].buyerUserId==user);
    CHECK(sell.trades[0].sellerUserId==user);

    auto account = accountOf(ex, user);
    CHECK(account.cash==800);
    CHECK(account.reservedCash==200);
    CHECK(positionOf(account, "AAPL")==5);
    CHECK(reservedPositionOf(account, "AAPL")==0);
    CHECK(account.trade.size()==2);
    CHECK(ex.getBuyOrders("AAPL").size()==1);
    CHECK(ex.getBuyOrders("AAPL")[0].quantity==2);
    CHECK(ex.checkInvariant());
}

void deleteUserRejectsNonEmptyAccounts(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    const auto cashUser = ex.addUser();
    CHECK(ex.depositCash(100, cashUser));
    CHECK(!ex.deleteUser(cashUser));
    CHECK(ex.hasUser(cashUser));

    const auto positionUser = ex.addUser();
    CHECK(ex.depositPosition(1, "AAPL", positionUser)==1);
    CHECK(!ex.deleteUser(positionUser));
    CHECK(ex.hasUser(positionUser));
    CHECK(ex.checkInvariant());
}

void deleteUserAllowsEmptyAccount(){
    Exch::Exchange ex;
    const auto user = ex.addUser();
    CHECK(ex.deleteUser(user));
    CHECK(!ex.hasUser(user));
    CHECK(ex.checkInvariant());
}




int main() {

    RUN_TEST(addSymbol);
    RUN_TEST(duplicateSymbolRejected);
    RUN_TEST(symbolList);
    RUN_TEST(rejectBuyFromUnknownSymbol);
    RUN_TEST(rejectSellFromUnknownSymbol);
    RUN_TEST(rejectInvalidBuyPriceOrQuantity);
    RUN_TEST(rejectInvalidSellPriceOrQuantity);
    RUN_TEST(buyRestsInCorrectSymbol);
    RUN_TEST(sellRestsInCorrectSymbol);
    RUN_TEST(symbolsAreIsolated);

    RUN_TEST(sameSymbolBuyCrossesSell);
    RUN_TEST(sameSymbolSellCrossesBuy);
    RUN_TEST(tradePriceIsRestingPrice);
    RUN_TEST(globalOrderIdsAcrossSymbols);
    RUN_TEST(cancelActiveBuyByGlobalIds);
    RUN_TEST(cancelActiveSellByGlobalIds);
    RUN_TEST(cancelUnknownOrderIdFails);
    RUN_TEST(cancelFullyFilledIncomingOrderFails);
    RUN_TEST(cancelFullyFilledRestingOrderFails);
    RUN_TEST(cancelPartialyFilledRestingBuy);


    RUN_TEST(cancelPartialyFilledRestingSell);
    RUN_TEST(cancelOneSymbolDoesNotAffectOtherSymbol);
    RUN_TEST(tradesAreSymbolSpecific);
    RUN_TEST(getTradesUnknownSymbolReturnsEmpty);
    RUN_TEST(bestBidAskSpreadPerSymbol);
    RUN_TEST(clearOneSymbol);
    RUN_TEST(clearUnknownSymbolDoesNothing);
    RUN_TEST(clearAll);
    RUN_TEST(addSymbolAfterClearAll);
    RUN_TEST(multiLevelSweepOnOneSymbol);


    RUN_TEST(FIFOSamePriceThroughExchange);
    RUN_TEST(pricePriorityThroughExchange);
    RUN_TEST(orderIdsDoesNotAdvanceOnRejectedUnknownSymbol);
    RUN_TEST(orderIdDoesNotAdvanceOnInvalidPriceQuantity);
    RUN_TEST(randomizedExchangeOperationsPreserveInvariants);

    RUN_TEST(clearSymbolTradeHistoryTest);
    RUN_TEST(clearAllDuplicateSymbolSemeticsTest);
    RUN_TEST(buyOrderReservesCash);
    RUN_TEST(sellOrderReservesPosition);
    RUN_TEST(buyRejectsWhenAvailableCashIsInsufficient);
    RUN_TEST(sellRejectsWhenAvailablePositionIsInsufficient);
    RUN_TEST(fullFillSettlesBuyerAndSellerAccounts);
    RUN_TEST(partialFillLeavesRemainingBuyReserved);
    RUN_TEST(partialFillLeavesRemainingSellReserved);
    RUN_TEST(priceImprovementRefundsBuyerCash);
    RUN_TEST(withdrawUsesAvailableCashOnly);
    RUN_TEST(cancelBuyReleasesReservedCash);
    RUN_TEST(cancelSellReleasesReservedPosition);
    RUN_TEST(clearSymbolReleasesOnlyThatSymbolReservations);
    RUN_TEST(clearAllReleasesAllReservations);
    RUN_TEST(selfTradeRegistersAndSettlesBothSides);
    RUN_TEST(deleteUserRejectsNonEmptyAccounts);
    RUN_TEST(deleteUserAllowsEmptyAccount);



    std::cout << "\nAll tests passed.\n";
    return 0;
}
