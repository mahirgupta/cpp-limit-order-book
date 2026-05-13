#include<orderBook/OrderBook.hpp>

#include <cstdlib>
#include <iostream>
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

    auto r1 = book.makeBuyOrder(100,10);
    auto r2 = book.makeSellOrder(105,5);

    CHECK(book.getTrade(r1).empty());
    CHECK(book.getTrade(r2).empty());
    CHECK(book.getBestBid()!=-1);
    CHECK(book.getBestAsk()!=-1);
    CHECK(book.getBestBid() == 100);
    CHECK(book.getBestAsk() == 105);
    CHECK(book.getSpread() == 5);

    // CHECK(book.checkInvariants());
}

int main() {
    RUN_TEST(test_no_match);

    std::cout << "\nAll tests passed.\n";
    return 0;
}