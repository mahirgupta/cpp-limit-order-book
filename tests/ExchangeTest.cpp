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



void addSymbol(){
    Exch::Exchange ex;
    CHECK(ex.addSymbol("AAPL"));
    CHECK(ex.hasSymbol("AAPL"));
}

void duplicateSymbolRejected(){
    Exch::Exchange ex;
    CHECK(ex.addSymbol("AAPL"));
    CHECK(!ex.addSymbol("AAPL"));
}

void symbolList(){
    Exch::Exchange ex;
    ex.addSymbol("MSFT");
    ex.addSymbol("AAPL");
    auto symbols = ex.getSymbol();
    CHECK(symbols.size()==2);
    CHECK(symbols[0]=="AAPL");
    CHECK(symbols[1]=="MSFT"); // make sure symbols are keys so should in sorted order
}

void rejectBuyFromUnknownSymbol(){
    Exch::Exchange ex;
    auto r = ex.buy("AAPL",100,10);
    CHECK(r.accepted==false);
    CHECK(r.orderId==0);
    CHECK(r.remainQuantity==0);
    CHECK(r.trades.empty());
    CHECK(ex.getBestBid("AAPL")==-1);
}

void rejectSellFromUnknownSymbol(){
    Exch::Exchange ex;
    auto r = ex.sell("AAPL",100,10);
    CHECK(r.accepted==false);
    CHECK(r.orderId==0);
    CHECK(r.remainQuantity==0);
    CHECK(r.trades.empty());
    CHECK(ex.getBestAsk("AAPL")==-1);
}

void rejectInvalidBuyPriceOrQuantity(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");

    CHECK(!ex.buy("AAPL",0,10).accepted);
    CHECK(!ex.buy("AAPL",-1,10).accepted);
    CHECK(!ex.buy("AAPL",100,0).accepted);
    CHECK(!ex.buy("AAPL",100,-1).accepted);
}

void rejectInvalidSellPriceOrQuantity(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");

    CHECK(!ex.sell("AAPL",0,10).accepted);
    CHECK(!ex.sell("AAPL",-1,10).accepted);
    CHECK(!ex.sell("AAPL",100,0).accepted);
    CHECK(!ex.sell("AAPL",100,-1).accepted);
}

void buyRestsInCorrectSymbol(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    auto r = ex.buy("AAPL",100,10);
    CHECK(r.accepted);
    CHECK(r.orderId==1);
    CHECK(r.remainQuantity==10);
    CHECK(r.symbol=="AAPL");
    CHECK(r.trades.empty());
    CHECK(ex.getBestBid("AAPL")==100);
    CHECK(ex.getBuyOrders("AAPL").size()==1);
}

void sellRestsInCorrectSymbol(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    auto r = ex.sell("AAPL",100,10);
    CHECK(r.accepted);
    CHECK(r.orderId==1);
    CHECK(r.remainQuantity==10);
    CHECK(r.symbol=="AAPL");
    CHECK(r.trades.empty());
    CHECK(ex.getBestAsk("AAPL")==100);
    CHECK(ex.getSellOrders("AAPL").size()==1);
}

void symbolsAreIsolated(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    ex.addSymbol("MSFT");
    
    auto r1 = ex.buy("AAPL",100,10);
    auto r2 = ex.sell("MSFT",90,5);

    CHECK(r1.accepted);
    CHECK(r2.accepted);

    CHECK(r1.trades.empty());
    CHECK(r2.trades.empty());

    CHECK(ex.getBestBid("AAPL")==100);
    CHECK(ex.getBestAsk("MSFT")==90);

    CHECK(ex.getTrades("AAPL").empty());
    CHECK(ex.getTrades("MSFT").empty());
}






void sameSymbolBuyCrossesSell(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    auto s = ex.sell("AAPL",100,5);
    auto b = ex.buy("AAPL",105,2);

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
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    auto b = ex.buy("AAPL",100,5);
    auto s = ex.sell("AAPL",95,2);

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
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    ex.sell("AAPL",100,5);
    auto b = ex.buy("AAPL",110,5);

    CHECK(b.trades.size()==1);
    CHECK(b.trades[0].price==100);

    ex.addSymbol("MSFT");
    ex.buy("MSFT",100,5);
    auto s = ex.sell("MSFT",90,5);

    CHECK(s.trades.size()==1);
    CHECK(s.trades[0].price==100);

}

void globalOrderIdsAcrossSymbols(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    ex.addSymbol("MSFT");

    auto a = ex.buy("AAPL",100,10);
    auto m = ex.sell("MSFT",200,5);

    CHECK(a.orderId==1);
    CHECK(m.orderId==2);
}

void cancelActiveBuyByGlobalIds(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    auto b = ex.buy("AAPL",100,10);
    CHECK(b.accepted);
    CHECK(!ex.getBuyOrders("AAPL").empty());
    CHECK(ex.getBestBid("AAPL")==100);
    auto c = ex.cancelOrder(b.orderId);

    CHECK(c.cancelled);
    CHECK(c.symbol=="AAPL");
    CHECK(ex.getBuyOrders("AAPL").empty());
    CHECK(ex.getBestBid("AAPL")==-1);
}

void cancelActiveSellByGlobalIds(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    auto s = ex.sell("AAPL",100,10);
    CHECK(s.accepted);
    CHECK(!ex.getSellOrders("AAPL").empty());
    CHECK(ex.getBestAsk("AAPL")==100);
    auto c = ex.cancelOrder(s.orderId);

    CHECK(c.cancelled);
    CHECK(c.symbol=="AAPL");
    CHECK(ex.getSellOrders("AAPL").empty());
    CHECK(ex.getBestAsk("AAPL")==-1);
}

void cancelUnknownOrderIdFails(){
    Exch::Exchange ex;
    auto c = ex.cancelOrder(999);

    CHECK(!c.cancelled);

}

void cancelFullyFilledIncomingOrderFails(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    ex.sell("AAPL",100,5);
    auto b = ex.buy("AAPL",100,5);
    CHECK(b.accepted);
    auto c = ex.cancelOrder(b.orderId);

    CHECK(!c.cancelled);
}

void cancelFullyFilledRestingOrderFails(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    auto b = ex.buy("AAPL",100,5);
    auto s = ex.sell("AAPL",100,5);
    CHECK(b.accepted);
    CHECK(s.accepted);

    auto c = ex.cancelOrder(b.orderId);

    CHECK(!c.cancelled);
    CHECK(ex.getBuyOrders("AAPL").empty());
    CHECK(ex.getSellOrders("AAPL").empty());
}

void cancelPartialyFilledRestingBuy(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    auto b = ex.buy("AAPL",100,10);
    CHECK(b.accepted);
    auto s = ex.sell("AAPL",100,4);
    CHECK(s.accepted);
    CHECK(s.trades.size()==1);
    CHECK(ex.getBuyOrders("AAPL").size()==1);
    CHECK(ex.getBuyOrders("AAPL")[0].quantity==6);
    auto c = ex.cancelOrder(b.orderId);
    CHECK(c.cancelled);
    CHECK(c.symbol=="AAPL");
    CHECK(ex.getBuyOrders("AAPL").empty());
}





void cancelPartialyFilledRestingSell(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    auto s = ex.sell("AAPL",100,10);
    CHECK(s.accepted);
    auto b = ex.buy("AAPL",100,4);
    CHECK(b.accepted);
    CHECK(b.trades.size()==1);
    CHECK(ex.getSellOrders("AAPL").size()==1);
    CHECK(ex.getSellOrders("AAPL")[0].quantity==6);
    auto c = ex.cancelOrder(s.orderId);
    CHECK(c.cancelled);
    CHECK(c.symbol=="AAPL");
    CHECK(ex.getSellOrders("AAPL").empty());
}

void cancelOneSymbolDoesNotAffectOtherSymbol(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    ex.addSymbol("MSFT");

    auto a = ex.buy("AAPL",100,10);
    auto m = ex.buy("MSFT",200,5);

    CHECK(a.accepted);
    CHECK(m.accepted);
    CHECK(ex.getBuyOrders("AAPL").size()==1);
    CHECK(ex.getBuyOrders("MSFT").size()==1);

    auto c = ex.cancelOrder(a.orderId);
    CHECK(c.cancelled);
    CHECK(c.symbol=="AAPL");
    CHECK(ex.getBuyOrders("AAPL").empty());
    CHECK(ex.getBestBid("MSFT")==200);
    CHECK(ex.getBuyOrders("MSFT").size()==1);
}

void tradesAreSymbolSpecific(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    ex.addSymbol("MSFT");

    ex.sell("AAPL",100,5);
    ex.buy("AAPL",100,5);

    ex.sell("MSFT",200,3);
    ex.buy("MSFT",200,3);

    CHECK(ex.getTrades("AAPL").size()==1);
    CHECK(ex.getTrades("MSFT").size()==1);
    CHECK(ex.getTrades("AAPL")[0].symbol=="AAPL");
    CHECK(ex.getTrades("MSFT")[0].symbol=="MSFT");
}

void getTradesUnknownSymbolReturnsEmpty(){
    Exch::Exchange ex;
    auto trade = ex.getTrades("UNKNOWN");
    CHECK(trade.empty());
}

void bestBidAskSpreadPerSymbol(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    ex.addSymbol("MSFT");

    ex.buy("AAPL",100,10);
    ex.sell("AAPL",105,5);

    ex.buy("MSFT",200,10);
    ex.sell("MSFT",220,5);

    CHECK(ex.getBestBid("AAPL")==100);
    CHECK(ex.getBestAsk("AAPL")==105);
    CHECK(ex.getSpread("AAPL")==5);
    
    CHECK(ex.getBestBid("MSFT")==200);
    CHECK(ex.getBestAsk("MSFT")==220);
    CHECK(ex.getSpread("MSFT")==20);

}

void clearOneSymbol(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    ex.addSymbol("MSFT");
    auto a = ex.buy("AAPL",100,10);
    auto m = ex.buy("MSFT",200,5);
    CHECK(a.accepted);
    CHECK(m.accepted);

    CHECK(!ex.getBuyOrders("AAPL").empty());
    CHECK(!ex.getBuyOrders("MSFT").empty());
    ex.clearSymbol("AAPL");
    CHECK(ex.getBuyOrders("AAPL").empty());
    CHECK(!ex.getBuyOrders("MSFT").empty());

    auto c = ex.cancelOrder(a.orderId);
    CHECK(!c.cancelled);

    auto d = ex.cancelOrder(m.orderId);
    CHECK(d.cancelled);

}

void clearUnknownSymbolDoesNothing(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    ex.buy("AAPL",100,10);
    CHECK(ex.getBuyOrders("AAPL").size()==1);
    CHECK(ex.getBuyOrders("AAPL")[0].price==100);
    CHECK(ex.getBuyOrders("AAPL")[0].quantity==10);
    
    ex.clearSymbol("UNKNOWN");
    CHECK(ex.getBuyOrders("AAPL").size()==1);
    CHECK(ex.getBuyOrders("AAPL")[0].price==100);
    CHECK(ex.getBuyOrders("AAPL")[0].quantity==10);
}

void clearAll(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    ex.addSymbol("MSFT");

    auto a = ex.buy("AAPL",100,10);
    auto m = ex.buy("MSFT",200,5);
    
    CHECK(ex.getBuyOrders("AAPL").size()==1);
    CHECK(ex.getBuyOrders("MSFT").size()==1);
    ex.clearAll();

    CHECK(ex.getBuyOrders("AAPL").empty());
    CHECK(ex.getBuyOrders("MSFT").empty());

    auto c1 = ex.cancelOrder(a.orderId);
    auto c2 = ex.cancelOrder(m.orderId);

    CHECK(!c1.cancelled);
    CHECK(!c2.cancelled);

    auto n = ex.buy("AAPL", 101, 1);
    CHECK(n.orderId == 1);
}

void addSymbolAfterClearAll(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    ex.clearAll();
    CHECK(ex.hasSymbol("AAPL"));
    
}

void multiLevelSweepOnOneSymbol(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");

    ex.sell("AAPL",100,5);
    ex.sell("AAPL",101,5);
    ex.sell("AAPL",102,5);

    auto b = ex.buy("AAPL",102,12);

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
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    auto s1 = ex.sell("AAPL", 100, 5);
    auto s2 = ex.sell("AAPL", 100, 7);
    auto b = ex.buy("AAPL", 100, 6);

    CHECK(b.trades.size()==2);
    CHECK(b.trades[0].sellerOrderId == s1.orderId);
    CHECK(b.trades[0].quantity==5);
    CHECK(b.trades[1].sellerOrderId == s2.orderId);
    CHECK(b.trades[1].quantity==1);
    CHECK(ex.getSellOrders("AAPL").size()==1);
    CHECK(ex.getSellOrders("AAPL")[0].quantity==6);
}

void pricePriorityThroughExchange(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    auto b1 = ex.buy("AAPL", 99, 5);
    auto b2 = ex.buy("AAPL", 101, 5);
    auto s = ex.sell("AAPL", 99, 5);

    CHECK(s.trades.size()==1);
    CHECK(s.trades[0].buyerOrderId==b2.orderId);
    CHECK(s.trades[0].price==101);

}

void orderIdsDoesNotAdvanceOnRejectedUnknownSymbol(){
    Exch::Exchange ex;
    auto bad = ex.buy("UNKNOWN",100,10);
    CHECK(!bad.accepted);
    ex.addSymbol("AAPL");
    auto good = ex.buy("AAPL",100,10);
    CHECK(good.accepted);
    CHECK(good.orderId==1);
}

void orderIdDoesNotAdvanceOnInvalidPriceQuantity(){
    Exch::Exchange ex;
    ex.addSymbol("AAPL");
    ex.buy("AAPL",0,10);
    ex.buy("AAPL",100,0);
    auto good = ex.buy("AAPL",100,1);
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
                CHECK(i.price==ex.getBestBid(sym));
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
                CHECK(i.price==ex.getBestAsk(sym));
                firstSell = false;
            }
            else{
                CHECK(i.price>=lastSellPrice);
            }
            lastSellPrice = i.price;
        }

        if(buyOrders.empty()) CHECK(ex.getBestBid(sym)==-1);
        if(sellOrders.empty()) CHECK(ex.getBestAsk(sym)==-1);

        if(!buyOrders.empty() && !sellOrders.empty()){
            CHECK(ex.getBestBid(sym)<ex.getBestAsk(sym));
            CHECK(ex.getSpread(sym)==ex.getBestAsk(sym)-ex.getBestBid(sym));
        }
        else{
            CHECK(ex.getSpread(sym)==-1);
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
}

void randomizedExchangeOperationsPreserveInvariants(){
    Exch::Exchange ex;
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
            CHECK(result.accepted);
            if(result.remainQuantity>0){
                candidateOrderIds.push_back(result.orderId);
            }
        }
        else if(op==1){
            auto result = ex.sell(sym,price,quantity);
            CHECK(result.accepted);
            if(result.remainQuantity>0){
                candidateOrderIds.push_back(result.orderId);
            }
        }
        else if(op==2 && !candidateOrderIds.empty()){
            const auto index = static_cast<std::size_t>(rng()%candidateOrderIds.size());
            ex.cancelOrder(candidateOrderIds[index]);
        }
        else{
            ex.clearSymbol(sym);
        }

        checkVisibleExchangeInvariants(ex,symbols);
    }
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



    std::cout << "\nAll tests passed.\n";
    return 0;
}
