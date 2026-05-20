#include<orderBook/OrderBook.hpp>

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

void test_no_match() {
    Orderbook::OrderBook book;

    auto r1 = book.makeBuyOrder(100,10,1);
    auto r2 = book.makeSellOrder(105,5,2);

    CHECK(r1.trade.empty());
    CHECK(r2.trade.empty());
    CHECK(book.getBestBid()!=-1);
    CHECK(book.getBestAsk()!=-1);
    CHECK(book.getBestBid() == 100);
    CHECK(book.getBestAsk() == 105);
    CHECK(book.getSpread() == 5);
}


void test_invalid_buy_order(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(0,10,1);
    CHECK(r1.accepted==0 && r1.orderId==0 && r1.trade.empty() && r1.remainQuantity==0);
    auto r2 = book.makeBuyOrder(-1,10,2);
    CHECK(r2.accepted==0 && r2.orderId==0 && r2.trade.empty() && r2.remainQuantity==0);
    auto r3 = book.makeBuyOrder(100,0,3);
    CHECK(r3.accepted==0 && r3.orderId==0 && r3.trade.empty() && r3.remainQuantity==0);
    auto r4 = book.makeBuyOrder(100,-1,4);
    CHECK(r4.accepted==0 && r4.orderId==0 && r4.trade.empty() && r4.remainQuantity==0);
    CHECK(book.getBestBid()==-1);
    CHECK(book.getBuyOrders().empty());
    CHECK(book.getBestAsk()==-1);
}

void test_invalid_sell_order(){
    Orderbook::OrderBook book;
    auto r1 = book.makeSellOrder(0,10,1);
    CHECK(r1.accepted==0 && r1.orderId==0 && r1.trade.empty() && r1.remainQuantity==0);
    auto r2 = book.makeSellOrder(-1,10,2);
    CHECK(r2.accepted==0 && r2.orderId==0 && r2.trade.empty() && r2.remainQuantity==0);
    auto r3 = book.makeSellOrder(100,0,3);
    CHECK(r3.accepted==0 && r3.orderId==0 && r3.trade.empty() && r3.remainQuantity==0);
    auto r4 = book.makeSellOrder(100,-1,4);
    CHECK(r4.accepted==0 && r4.orderId==0 && r4.trade.empty() && r4.remainQuantity==0);
    CHECK(book.getBestAsk()==-1);
    CHECK(book.getSellOrders().empty());
    CHECK(book.getBestBid()==-1);
}

void test_accept_valid_buy(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(100,10,1);
    CHECK(r1.accepted==1 && r1.orderId!=0 && r1.trade.empty() && r1.remainQuantity==10);
    CHECK(book.getBestBid()==100);
    CHECK(book.getBestAsk()==-1);
    CHECK(!book.getBuyOrders().empty());
}

void test_accept_valid_sell(){
    Orderbook::OrderBook book;
    auto r1 = book.makeSellOrder(100,10,1);
    CHECK(r1.accepted==1 && r1.orderId!=0 && r1.trade.empty() && r1.remainQuantity==10);
    CHECK(book.getBestAsk()==100);
    CHECK(book.getBestBid()==-1);
    CHECK(!book.getSellOrders().empty());
}



void test_buy_crosses_sell(){
    Orderbook::OrderBook book;
    auto r1 = book.makeSellOrder(100,10,1);
    CHECK(r1.trade.empty());
    CHECK(book.getBestAsk()==100);
    auto r2 = book.makeBuyOrder(105,4,2);
    CHECK(r2.accepted==1);
    CHECK(!r2.trade.empty());
    CHECK(r2.trade.size()==1);
    CHECK(r2.trade[0].buyerOrderId == r2.orderId);
    CHECK(r2.trade[0].sellerOrderId == r1.orderId);
    CHECK(r2.remainQuantity==0);
    CHECK(r2.trade[0].price==100);
    CHECK(book.getBestBid()==-1);
    CHECK(book.getBestAsk()==100);
    CHECK(book.getSellOrders().size()==1);
    CHECK(book.getSellOrders()[0].quantity==6);
    CHECK(book.getBuyOrders().empty());
}


void test_sell_crosses_buy(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(100,10,1);
    CHECK(r1.trade.empty());
    CHECK(book.getBestBid()==100);
    auto r2 = book.makeSellOrder(95,4,2);
    CHECK(r2.accepted==1);
    CHECK(!r2.trade.empty());
    CHECK(r2.trade.size()==1);
    CHECK(r2.trade[0].sellerOrderId == r2.orderId);
    CHECK(r2.trade[0].buyerOrderId == r1.orderId);
    CHECK(r2.remainQuantity==0);
    CHECK(r2.trade[0].price==100);
    CHECK(book.getBestAsk()==-1);
    CHECK(book.getBestBid()==100);
    CHECK(book.getBuyOrders().size()==1);
    CHECK(book.getBuyOrders()[0].quantity==6);
    CHECK(book.getSellOrders().empty());
}


void test_full_fill(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(100,10,1);
    CHECK(r1.trade.empty());
    CHECK(book.getBestBid()==100);
    auto r2 = book.makeSellOrder(100,10,2);
    CHECK(r2.accepted==1);
    CHECK(!r2.trade.empty());
    CHECK(r2.trade.size()==1);
    CHECK(r2.trade[0].sellerOrderId == r2.orderId);
    CHECK(r2.trade[0].buyerOrderId == r1.orderId);
    CHECK(r2.remainQuantity==0);
    CHECK(r2.trade[0].price==100);
    CHECK(book.getBestAsk()==-1);
    CHECK(book.getBestBid()==-1);
    CHECK(book.getBuyOrders().empty());
    CHECK(book.getSellOrders().empty());
}

void test_partial_fill_resting(){
    Orderbook::OrderBook book;
    auto r1 = book.makeSellOrder(100,10,1);
    CHECK(r1.trade.empty());
    CHECK(book.getBestAsk()==100);
    auto r2 = book.makeBuyOrder(100,4,2);
    CHECK(r2.accepted==1);
    CHECK(!r2.trade.empty());
    CHECK(r2.trade.size()==1);
    CHECK(r2.trade[0].buyerOrderId == r2.orderId);
    CHECK(r2.trade[0].sellerOrderId == r1.orderId);
    CHECK(r2.remainQuantity==0);
    CHECK(r2.trade[0].price==100);
    CHECK(book.getBestBid()==-1);
    CHECK(book.getBestAsk()==100);
    CHECK(book.getSellOrders().size()==1);
    CHECK(book.getSellOrders()[0].quantity==6);
    CHECK(book.getBuyOrders().empty());
}

void test_partial_fill_incoming(){
    Orderbook::OrderBook book;
    auto r1 = book.makeSellOrder(100,4,1);
    CHECK(r1.trade.empty());
    CHECK(book.getBestAsk()==100);
    auto r2 = book.makeBuyOrder(100,10,2);
    CHECK(r2.accepted==1);
    CHECK(!r2.trade.empty());
    CHECK(r2.trade.size()==1);
    CHECK(r2.trade[0].buyerOrderId == r2.orderId);
    CHECK(r2.trade[0].sellerOrderId == r1.orderId);
    CHECK(r2.remainQuantity==6);
    CHECK(r2.trade[0].price==100);
    CHECK(book.getBestBid()==100);
    CHECK(book.getBestAsk()==-1);
    CHECK(book.getBuyOrders().size()==1);
    CHECK(book.getBuyOrders()[0].quantity==6);
    CHECK(book.getSellOrders().empty());
}

void test_FIFO_same_price_buy(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(100,5,1);
    auto r2 = book.makeBuyOrder(100,7,2);
    CHECK(r1.trade.empty());
    CHECK(r2.trade.empty());
    CHECK(book.getBestBid()==100);
    auto r3 = book.makeSellOrder(100,6,3);
    CHECK(!r3.trade.empty());
    CHECK(r3.trade.size()==2);
    CHECK(r3.trade[0].buyerOrderId == r1.orderId);
    CHECK(r3.trade[1].buyerOrderId == r2.orderId);
    CHECK(r3.trade[0].sellerOrderId==r3.orderId);
    CHECK(r3.trade[1].sellerOrderId==r3.orderId);
    CHECK(r3.remainQuantity==0);
    CHECK(r3.trade[0].price==100);
    CHECK(r3.trade[1].price==100);
    CHECK(book.getBestBid()==100);
    CHECK(book.getBestAsk()==-1);
    CHECK(book.getBuyOrders().size()==1);
    CHECK(book.getBuyOrders()[0].quantity==6);
    CHECK(book.getSellOrders().empty());

}


void test_FIFO_same_price_sell(){
    Orderbook::OrderBook book;
    auto r1 = book.makeSellOrder(100,5,1);
    auto r2 = book.makeSellOrder(100,7,2);
    CHECK(r1.trade.empty());
    CHECK(r2.trade.empty());
    CHECK(book.getBestAsk()==100);
    auto r3 = book.makeBuyOrder(100,6,3);
    CHECK(!r3.trade.empty());
    CHECK(r3.trade.size()==2);
    CHECK(r3.trade[0].sellerOrderId == r1.orderId);
    CHECK(r3.trade[1].sellerOrderId == r2.orderId);
    CHECK(r3.trade[0].buyerOrderId==r3.orderId);
    CHECK(r3.trade[1].buyerOrderId==r3.orderId);
    CHECK(r3.remainQuantity==0);
    CHECK(r3.trade[0].price==100);
    CHECK(r3.trade[1].price==100);
    CHECK(book.getBestAsk()==100);
    CHECK(book.getBestBid()==-1);
    CHECK(book.getSellOrders().size()==1);
    CHECK(book.getSellOrders()[0].quantity==6);
    CHECK(book.getBuyOrders().empty());

}


void test_best_price_priority_before_FIFO(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(99,10,1);
    auto r2 = book.makeBuyOrder(101,10,2);
    CHECK(r1.trade.empty());
    CHECK(r2.trade.empty());
    CHECK(book.getBestBid()==101);
    auto r3 = book.makeSellOrder(99,5,3);
    CHECK(!r3.trade.empty());
    CHECK(r3.trade.size()==1);
    CHECK(r3.trade[0].buyerOrderId == r2.orderId);
    CHECK(r3.trade[0].sellerOrderId==r3.orderId);
    CHECK(r3.remainQuantity==0);
    CHECK(r3.trade[0].price==101);
    CHECK(book.getBestBid()==101);
    CHECK(book.getBestAsk()==-1);
    CHECK(book.getBuyOrders().size()==2);
    CHECK(book.getBuyOrders()[0].quantity==5);
    CHECK(book.getBuyOrders()[0].price==101);
    CHECK(book.getBuyOrders()[1].quantity==10);
    CHECK(book.getBuyOrders()[1].price==99);
    CHECK(book.getSellOrders().empty());
}


void test_FIFO_within_same_price(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(100,5,1);
    auto r2 = book.makeBuyOrder(101,5,2);
    auto r3 = book.makeBuyOrder(100,5,3);
    CHECK(r1.trade.empty());
    CHECK(r2.trade.empty());
    CHECK(r3.trade.empty());
    CHECK(book.getBestBid()==101);
    auto r4 = book.makeSellOrder(100,6,4);
    CHECK(!r4.trade.empty());
    CHECK(r4.trade.size()==2);
    CHECK(r4.trade[0].buyerOrderId == r2.orderId);
    CHECK(r4.trade[0].sellerOrderId==r4.orderId);
    CHECK(r4.trade[1].buyerOrderId == r1.orderId);
    CHECK(r4.trade[1].sellerOrderId==r4.orderId);
    CHECK(r4.remainQuantity==0);
    CHECK(r4.trade[0].price==101);
    CHECK(r4.trade[1].price==100);
    CHECK(book.getBestBid()==100);
    CHECK(book.getBestAsk()==-1);
    CHECK(book.getBuyOrders().size()==2);
    CHECK(book.getBuyOrders()[0].quantity==4);
    CHECK(book.getBuyOrders()[0].price==100);
    CHECK(book.getBuyOrders()[1].quantity==5);
    CHECK(book.getBuyOrders()[1].price==100);
    CHECK(book.getSellOrders().empty());
}


void test_incoming_buy_sweeps_multiple_ask(){
    Orderbook::OrderBook book;
    auto r1 = book.makeSellOrder(100,5,1);
    auto r2 = book.makeSellOrder(101,5,2);
    auto r3 = book.makeSellOrder(102,5,3);
    CHECK(r1.trade.empty());
    CHECK(r2.trade.empty());
    CHECK(r3.trade.empty());
    CHECK(book.getBestAsk()==100);
    auto r4 = book.makeBuyOrder(102,12,4);
    CHECK(!r4.trade.empty());
    CHECK(r4.trade.size()==3);
    CHECK(r4.remainQuantity==0);
    CHECK(r4.trade[0].buyerOrderId == r4.orderId);
    CHECK(r4.trade[1].buyerOrderId == r4.orderId);
    CHECK(r4.trade[2].buyerOrderId == r4.orderId);

    CHECK(r4.trade[0].sellerOrderId == r1.orderId);
    CHECK(r4.trade[1].sellerOrderId == r2.orderId);
    CHECK(r4.trade[2].sellerOrderId == r3.orderId);

    CHECK(r4.trade[0].quantity==5);
    CHECK(r4.trade[1].quantity==5);
    CHECK(r4.trade[2].quantity==2);

    CHECK(r4.trade[0].price==100);
    CHECK(r4.trade[1].price==101);
    CHECK(r4.trade[2].price==102);

    CHECK(book.getBestAsk()==102);
    CHECK(book.getBestBid()==-1);

    CHECK(book.getBuyOrders().empty());
    CHECK(!book.getSellOrders().empty());
    CHECK(book.getSellOrders().size()==1);

    CHECK(book.getSellOrders()[0].orderId==r3.orderId);
    CHECK(book.getSellOrders()[0].price==102);
    CHECK(book.getSellOrders()[0].quantity==3);

}

void test_incoming_sell_sweeps_multiple_bid(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(102,5,1);
    auto r2 = book.makeBuyOrder(101,5,2);
    auto r3 = book.makeBuyOrder(100,5,3);
    CHECK(r1.trade.empty());
    CHECK(r2.trade.empty());
    CHECK(r3.trade.empty());
    CHECK(book.getBestBid()==102);
    auto r4 = book.makeSellOrder(100,12,4);
    CHECK(!r4.trade.empty());
    CHECK(r4.trade.size()==3);
    CHECK(r4.remainQuantity==0);
    CHECK(r4.trade[0].sellerOrderId == r4.orderId);
    CHECK(r4.trade[1].sellerOrderId == r4.orderId);
    CHECK(r4.trade[2].sellerOrderId == r4.orderId);

    CHECK(r4.trade[0].buyerOrderId == r1.orderId);
    CHECK(r4.trade[1].buyerOrderId == r2.orderId);
    CHECK(r4.trade[2].buyerOrderId == r3.orderId);

    CHECK(r4.trade[0].quantity==5);
    CHECK(r4.trade[1].quantity==5);
    CHECK(r4.trade[2].quantity==2);

    CHECK(r4.trade[0].price==102);
    CHECK(r4.trade[1].price==101);
    CHECK(r4.trade[2].price==100);

    CHECK(book.getBestBid()==100);
    CHECK(book.getBestAsk()==-1);

    CHECK(book.getSellOrders().empty());
    CHECK(!book.getBuyOrders().empty());
    CHECK(book.getBuyOrders().size()==1);

    CHECK(book.getBuyOrders()[0].orderId==r3.orderId);
    CHECK(book.getBuyOrders()[0].price==100);
    CHECK(book.getBuyOrders()[0].quantity==3);

}

void test_incoming_order_stops_when_no_longer_crossing(){
    Orderbook::OrderBook book;
    auto r1 = book.makeSellOrder(100,5,1);
    auto r2 = book.makeSellOrder(105,5,2);
    CHECK(r1.trade.empty());
    CHECK(r2.trade.empty());
    CHECK(book.getBestAsk()==100);
    auto r3 = book.makeBuyOrder(102,10,3);
    CHECK(!r3.trade.empty());
    CHECK(r3.trade.size()==1);
    CHECK(r3.remainQuantity==5);
    CHECK(r3.trade[0].buyerOrderId == r3.orderId);

    CHECK(r3.trade[0].sellerOrderId == r1.orderId);

    CHECK(r3.trade[0].quantity==5);

    CHECK(r3.trade[0].price==100);

    CHECK(book.getBestAsk()==105);
    CHECK(book.getBestBid()==102);

    CHECK(!book.getBuyOrders().empty());
    CHECK(!book.getSellOrders().empty());
    CHECK(book.getSellOrders().size()==1);
    CHECK(book.getBuyOrders().size()==1);

    CHECK(book.getBuyOrders()[0].quantity==5);
    CHECK(book.getSellOrders()[0].quantity==5);

    CHECK(book.getBuyOrders()[0].orderId==r3.orderId);
    CHECK(book.getSellOrders()[0].orderId==r2.orderId);

    CHECK(book.getBuyOrders()[0].price==102);
    CHECK(book.getSellOrders()[0].price==105);

}



void test_cancel_active_buy_order(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(100,10,1);
    CHECK(r1.trade.empty());
    CHECK(book.getBestBid()==100);
    auto r2 = book.cancelOrder(r1.orderId);
    CHECK(r2==1);
    CHECK(book.getBestBid()==-1);
    CHECK(book.getBuyOrders().empty());
}

void test_cancel_active_sell_order(){
    Orderbook::OrderBook book;
    auto r1 = book.makeSellOrder(100,10,1);
    CHECK(r1.trade.empty());
    CHECK(book.getBestAsk()==100);
    auto r2 = book.cancelOrder(r1.orderId);
    CHECK(r2==1);
    CHECK(book.getBestAsk()==-1);
    CHECK(book.getSellOrders().empty());
}


void test_cancel_missing_order(){
    Orderbook::OrderBook book;
    auto r1 = book.cancelOrder(999);
    CHECK(r1==0);
}


void test_cancel_partially_filled_resting_order(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(100,10,1);
    auto r2 = book.makeSellOrder(100,4,2);
    CHECK(book.getBestAsk()==-1);
    CHECK(book.getBestBid()==100);
    CHECK(book.getBuyOrders()[0].quantity==6);
    CHECK(!r2.trade.empty());

    auto r3 = book.cancelOrder(r1.orderId);
    CHECK(r3==1);

    CHECK(book.getBestBid()==-1);
    CHECK(book.getBuyOrders().empty());


}


void test_cancel_fully_filled_order(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(100,10,1);
    auto r2 = book.makeSellOrder(100,10,2);
    CHECK(!r2.trade.empty());
    CHECK(book.getBestAsk()==-1);
    CHECK(book.getBestBid()==-1);
    auto r3 = book.cancelOrder(r1.orderId);
    CHECK(r3==0);
    auto r4 = book.cancelOrder(r2.orderId);
    CHECK(r4==0);
}


void test_cancel_one_order_at_price_level_with_multiple_orders(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(100,5,1);
    auto r2 = book.makeBuyOrder(100,7,2);
    CHECK(book.getBestBid()==100);
    CHECK(book.getBuyOrders()[0].quantity==5);
    CHECK(book.getBuyOrders()[1].quantity==7);

    auto r3 = book.cancelOrder(r1.orderId);

    CHECK(r3==1);

    CHECK(book.getBestBid()==100);
    CHECK(book.getBuyOrders().size()==1);

    CHECK(book.getBuyOrders()[0].orderId == r2.orderId);
    CHECK(book.getBuyOrders()[0].price == 100);
    CHECK(book.getBuyOrders()[0].quantity == r2.remainQuantity);



}



void test_best_bid_updates_correctly(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(99,2,1);
    auto r2 = book.makeBuyOrder(101,2,2);
    auto r3 = book.makeBuyOrder(100,2,3);
    CHECK(book.getBestBid()==101);
    auto r4 = book.cancelOrder(r2.orderId);
    CHECK(r4==1);
    CHECK(book.getBestBid()==100);
}

void test_best_ask_updates_correctly(){
    Orderbook::OrderBook book;
    auto r1 = book.makeSellOrder(105,2,1);
    auto r2 = book.makeSellOrder(103,2,2);
    auto r3 = book.makeSellOrder(104,2,3);
    CHECK(book.getBestAsk()==103);
    auto r4 = book.cancelOrder(r2.orderId);
    CHECK(r4==1);
    CHECK(book.getBestAsk()==104);
}

void test_spread_calculation(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(100,1,1);
    auto r2 = book.makeSellOrder(105,2,2);
    CHECK(book.getSpread()==5);

    book.clear();
    CHECK(book.getSpread()==-1);
    auto r3 = book.makeBuyOrder(100,1,3);
    CHECK(book.getSpread()==-1);
    
    book.clear();

    auto r4 = book.makeSellOrder(99,1,4);
    CHECK(book.getSpread()==-1);

}

void test_clear_book(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(100,1,1);
    auto r2 = book.makeSellOrder(105,2,2);
    auto r3 = book.makeBuyOrder(101,3,3);
    auto r4  = book.makeSellOrder(100,1,4);

    CHECK(book.getBestAsk()==105);
    CHECK(book.getBestBid()==101);
    book.clear();

    CHECK(book.getBestAsk()==-1);
    CHECK(book.getBestBid()==-1);
    CHECK(book.getSpread()==-1);
    CHECK(book.getBuyOrders().empty());
    CHECK(book.getSellOrders().empty());
    CHECK(book.getTrade().empty());
    // CHECK(book.getCurOrderId()==0);
}

// void test_order_id_increments(){
//      Orderbook::OrderBook book;
//     auto r1 = book.makeBuyOrder(100,1,1);
//     auto r2 = book.makeSellOrder(105,2,2);
//     auto r3 = book.makeBuyOrder(101,3,3);
//     auto r4  = book.makeSellOrder(100,1,4);

//     // CHECK(book.getCurOrderId()==4);

//     book.clear();
//     auto r5 = book.makeBuyOrder(100,1,5);
//     // CHECK(book.getCurOrderId()==1);
// }


void test_accepted_result_for_resting_order(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(100,10,1);
    CHECK(r1.accepted==1 && r1.orderId==1 && r1.trade.empty() && r1.remainQuantity==10);
}

void test_rejected_result_shape(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(100,0,2);
    CHECK(r1.accepted==0 && r1.orderId==0 && r1.trade.empty() && r1.remainQuantity==0);
}

void test_no_empty_price_levels_after_full_fill(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(100,10,1);
    auto r2 = book.makeSellOrder(100,10,2);
    CHECK(book.getBestAsk()==-1);
    CHECK(book.getBestBid()==-1);
    CHECK(book.getBuyOrders().empty());
    CHECK(book.getSellOrders().empty());
}

void test_no_zero_quantity_orders_visible(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(100,5,1);
    auto r2 = book.makeBuyOrder(101,5,2);
    auto r3 = book.makeSellOrder(99,5,3);
    CHECK(book.getSellOrders().empty());
    auto b = book.getBuyOrders();
    for(auto i:b){
        CHECK(i.quantity>0);
    }
    auto s = book.getSellOrders();
    for(auto i:s){
        CHECK(i.quantity>0);
    }
}


void test_trade_quantities_always_positive(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(100,5,1);
    auto s5 = book.makeSellOrder(103,3,2);
    auto r2 = book.makeBuyOrder(101,6,3);
    auto r3 = book.makeBuyOrder(100,5,4);
    auto s4 = book.makeSellOrder(99,5,5);
    auto s3 = book.makeSellOrder(110,2,6);
    auto r4 = book.makeBuyOrder(99,5,7);
    auto r5 = book.makeBuyOrder(101,8,8);
    auto s1 = book.makeSellOrder(100,3,9);
    auto r6 = book.makeBuyOrder(105,1,10);

    auto s2 = book.makeSellOrder(105,1,11);
    auto s6 = book.makeSellOrder(102,1,12);


    auto t = book.getTrade();

    for(auto &i:t){
        CHECK(i.quantity>0);
    }


}

void test_trade_price_always_positive(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(100,5,1);
    auto s5 = book.makeSellOrder(103,3,2);
    auto r2 = book.makeBuyOrder(101,6,3);
    auto r3 = book.makeBuyOrder(100,5,4);
    auto s4 = book.makeSellOrder(99,5,5);
    auto s3 = book.makeSellOrder(110,2,6);
    auto r4 = book.makeBuyOrder(99,5,7);
    auto r5 = book.makeBuyOrder(101,8,8);
    auto s1 = book.makeSellOrder(100,3,9);
    auto r6 = book.makeBuyOrder(105,1,10);

    auto s2 = book.makeSellOrder(105,1,11);
    auto s6 = book.makeSellOrder(102,1,12);


    auto t = book.getTrade();

    for(auto &i:t){
        CHECK(i.price>0);
    }


}

void test_trade_book_always_sorted(){
    Orderbook::OrderBook book;
    auto r1 = book.makeBuyOrder(100,5,1);
    auto s5 = book.makeSellOrder(103,3,2);
    auto r2 = book.makeBuyOrder(101,6,3);
    auto r3 = book.makeBuyOrder(100,5,4);
    auto s4 = book.makeSellOrder(99,5,5);
    auto s3 = book.makeSellOrder(110,2,6);
    auto r4 = book.makeBuyOrder(99,5,7);
    auto r5 = book.makeBuyOrder(101,8,8);
    auto s1 = book.makeSellOrder(100,3,9);
    auto r6 = book.makeBuyOrder(105,1,10);

    auto s2 = book.makeSellOrder(105,1,11);
    auto s6 = book.makeSellOrder(102,1,12);



    auto b = book.getBuyOrders();
    auto s = book.getSellOrders();
    Orderbook::Price last = 1000;
    for(auto i:b){
        CHECK(i.price<=last && i.quantity>0);
        last = i.price;
    }

    last = 0;
    for(auto i:s){
        CHECK(i.price>=last && i.quantity>0);
        last = i.price;
    }


}

void test_randomized_operations_preserve_invariants(){
    Orderbook::OrderBook book;
    std::vector<Orderbook::OrderId> candidateOrderIds;
    std::mt19937 rng(42);
    Orderbook::OrderId curOrderId = 0;
    for(int step = 0; step < 5000; ++step){
        const auto op = static_cast<int>(rng() % 3);
        const auto price = static_cast<Orderbook::Price>(90 + (rng() % 25));
        const auto quantity = static_cast<Orderbook::Quantity>(1 + (rng() % 10));

        if(op == 0){
            auto result = book.makeBuyOrder(price, quantity,++curOrderId);
            if(result.accepted && result.remainQuantity > 0){
                candidateOrderIds.push_back(result.orderId);
            }
        }
        else if(op == 1){
            auto result = book.makeSellOrder(price, quantity,++curOrderId);
            if(result.accepted && result.remainQuantity > 0){
                candidateOrderIds.push_back(result.orderId);
            }
        }
        else if(!candidateOrderIds.empty()){
            const auto index = static_cast<std::size_t>(rng() % candidateOrderIds.size());
            book.cancelOrder(candidateOrderIds[index]);
        }

        CHECK(book.checkInvariants());
    }
}





int main() {
    RUN_TEST(test_invalid_buy_order);
    RUN_TEST(test_invalid_sell_order);
    RUN_TEST(test_accept_valid_buy);
    RUN_TEST(test_accept_valid_sell);
    RUN_TEST(test_no_match);
    RUN_TEST(test_buy_crosses_sell);
    RUN_TEST(test_sell_crosses_buy);
    RUN_TEST(test_full_fill);
    RUN_TEST(test_partial_fill_resting);
    RUN_TEST(test_partial_fill_incoming);
    RUN_TEST(test_FIFO_same_price_buy);
    RUN_TEST(test_FIFO_same_price_sell);
    RUN_TEST(test_best_price_priority_before_FIFO);
    RUN_TEST(test_FIFO_within_same_price);
    RUN_TEST(test_incoming_buy_sweeps_multiple_ask);
    RUN_TEST(test_incoming_sell_sweeps_multiple_bid);
    RUN_TEST(test_incoming_order_stops_when_no_longer_crossing);
    RUN_TEST(test_cancel_active_buy_order);
    RUN_TEST(test_cancel_active_sell_order);
    RUN_TEST(test_cancel_missing_order);
    RUN_TEST(test_cancel_partially_filled_resting_order);
    RUN_TEST(test_cancel_fully_filled_order);
    RUN_TEST(test_cancel_one_order_at_price_level_with_multiple_orders);
    RUN_TEST(test_best_bid_updates_correctly);
    RUN_TEST(test_best_ask_updates_correctly);
    RUN_TEST(test_spread_calculation);
    RUN_TEST(test_clear_book);
    // RUN_TEST(test_order_id_increments);
    RUN_TEST(test_accepted_result_for_resting_order);
    RUN_TEST(test_rejected_result_shape);
    RUN_TEST(test_no_empty_price_levels_after_full_fill);
    RUN_TEST(test_no_zero_quantity_orders_visible);
    RUN_TEST(test_trade_quantities_always_positive);
    RUN_TEST(test_trade_price_always_positive);
    RUN_TEST(test_trade_book_always_sorted);
    RUN_TEST(test_randomized_operations_preserve_invariants);

    std::cout << "\nAll tests passed.\n";
    return 0;
}
