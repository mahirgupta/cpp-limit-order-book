#include<iostream>
#include<Cli/Cli.hpp>
#include<cstdint>
#include<limits>
#include<string>
#include<sstream>
#include<vector>



void Cli::printBuyOrderList(Exch::Exchange &ex, const Orderbook::Symbol& sym){
    if(!ex.getBestBid(sym)){
        std::cout<<"No pending Bid ...\n";
        return ;
    }
    std::cout<<sym<<" Bid::\n";
    std::cout<<"OrderId\tPrice\tQuantity\tUserId\n";
    for(auto &i:ex.getBuyOrders(sym)){
        std::cout<<i.orderId<<"\t"<<i.price<<"\t"<<i.quantity<<"\t"<<i.UserId<<"\n";
    }
    std::cout<<"\n";
    return ;

}

void Cli::printSellOrderList(Exch::Exchange& ex, const Orderbook::Symbol& sym){
    if(!ex.getBestAsk(sym)){
        std::cout<<"No pending Ask...\n";
        return ;
    }
    std::cout<<sym<<" Ask::\n";
    std::cout<<"OrderId\tPrice\tQuantity\tUserId\n";
    for(auto &i:ex.getSellOrders(sym)){
        std::cout<<i.orderId<<"\t"<<i.price<<"\t"<<i.quantity<<"\t"<<i.UserId<<"\n";
    }
    std::cout<<"\n";
    return ;

}

void Cli::printTrade(const std::vector<Exch::ExchangeTrade>& trade){
    if(trade.empty()){
        std::cout<<"No Trade till now\n";
        return ;
    }
    std::cout<<"buyer\tseller\tPrice\tQuantity\tSymbol\tBuyerUserId\tSellerUserId\n";
    for(auto &i:trade){
        std::cout<<i.buyerOrderId<<"\t"<<i.sellerOrderId<<"\t"<<i.price<<"\t"<<i.quantity<<"\t"<<i.symbol<<"\t"<<i.buyerUserId<<"\t"<<i.sellerUserId<<"\n";
    }
    std::cout<<"\n";
    return;
}

void Cli::printSymbol(const std::vector<std::string>& s){
    if(s.size()==0){
        std::cout<<"No symbols added"<<std::endl;
        return ;
    }

    std::cout<<"Symbols::"<<std::endl;
    for(auto &i:s){
        std::cout<<i<<std::endl;
    }
    std::cout<<std::endl;
    return ;
}

void Cli::printUsers(const std::vector<Orderbook::UserId> &v){
    if(v.size()==0){
        std::cout<<"No Users added"<<std::endl;
        return ;
    }
    std::cout<<"Users\t"<<v.size()<<std::endl;
    for(auto &i:v) std::cout<<i<<std::endl;
    std::cout<<std::endl;
    return ;
}

void Cli::printAccount(const Exch::account& account){
    std::cout<<"UserId\tCash\tReservedCash\n";
    std::cout<<account.userid<<"\t"<<account.cash<<"\t"<<account.reservedCash<<"\n";
    std::cout<<"\n";

    bool hasPosition = false;
    std::cout<<"Symbol\tPosition\tReservedPosition\n";
    for(const auto &i:account.positions){
        hasPosition = true;
        auto reservedIt = account.reservedPositions.find(i.first);
        const auto reserved = reservedIt == account.reservedPositions.end() ? 0 : reservedIt->second;
        std::cout<<i.first<<"\t"<<i.second<<"\t"<<reserved<<"\n";
    }
    for(const auto &i:account.reservedPositions){
        if(account.positions.find(i.first)!=account.positions.end()) continue;
        hasPosition = true;
        std::cout<<i.first<<"\t"<<0<<"\t"<<i.second<<"\n";
    }
    if(!hasPosition){
        std::cout<<"No positions\n";
    }
    std::cout<<"\n";

    std::vector<Exch::ExchangeTrade> trades(account.trade.begin(), account.trade.end());
    Cli::printTrade(trades);
    return ;
}

void Cli::printBest(const Exch::Exchange& ex, const Orderbook::Symbol& sym){
    std::cout<<"Best Bid\tBest Ask\tSpread\n";
    if(!ex.getBestBid(sym))std::cout<<"none\t";
    else std::cout<<ex.getBestBid(sym).value()<<"\t";
    if(!ex.getBestAsk(sym))std::cout<<"none\t";
    else std::cout<<ex.getBestAsk(sym).value()<<"\t";
    if(!ex.getSpread(sym))std::cout<<"none\n";
    else std::cout<<ex.getSpread(sym).value()<<"\n";
    std::cout<<"\n";
    return;
}

int64_t stti(const std::string &s){
    int64_t ans = 0;

    for(auto i:s){
        if((i-'0') <0 || (i-'0') >9) return -1;
        if(ans>(std::numeric_limits<std::int64_t>::max() - (i-'0'))/10) return -1;
        ans= ans*10+(i-'0');
    }
    return int64_t(ans);
}

void printOrderResult(const Exch::ExchangeOrderResult &ok, const std::string& OrderType){
    if(ok.accepted){
        std::cout<<"Successfully Placed "<<OrderType<<" at "<<ok.symbol<<" with OrderId: "<<ok.orderId<<std::endl;
        if(!ok.trades.empty()){
            std::cout<<"Executed Trade"<<std::endl;
            Cli::printTrade(ok.trades);

            if(ok.remainQuantity>0){
                std::cout<<"Partial order remains with orderId: "<<ok.orderId<<" Quantity: "<<ok.remainQuantity<<std::endl;
            }
            else std::cout<<"Order Complete Fully"<<std::endl;
        }
    }
    else{
        std::cout<<"Error..! Order is not placed "<<ok.message<<std::endl;
    }
}


void Cli::run(Exch::Exchange &ex){
    std::string inp;
    bool exit = 0;

    while(!exit){
        std::cout<<"Enter any command for help type \"help\"\n";
        if(!std::getline(std::cin, inp)){
            exit=1;
            break;
        };
        std::stringstream ss(inp);
        std::string word;
        std::vector<std::string>words;
        while(ss>>word){
            words.push_back(word);
        }
        if(words.size()==0) continue;
        if(words[0] == "help" && words.size()==1){
            std::cout<<"Available commands: help, buy, sell"<<std::endl;
            std::cout<<"add_symbol <symbol>"<<std::endl;
            std::cout<<"symbols"<<std::endl;
            std::cout<<"buy <UserID> <symbol> <price> <quantity>"<<std::endl;
            std::cout<<"sell <UserId> <symbol> <price> <quantity>"<<std::endl;
            std::cout<<"cancel <UserId> <orderid>"<<std::endl;
            std::cout<<"book <symbol>"<<std::endl;
            std::cout<<"trades <symbol>"<<std::endl;
            std::cout<<"best <symbol>"<<std::endl;
            std::cout<<"clear <symbol>"<<std::endl;
            std::cout<<"clear_all"<<std::endl;
            std::cout<<"exit/quit"<<std::endl;
            std::cout<<"add_user"<<std::endl;
            std::cout<<"users"<<std::endl;
            std::cout<<"account <UserId>"<<std::endl;
            std::cout<<"deposit_cash <UserId> <amount>"<<std::endl;
            std::cout<<"withdraw_cash <UserId> <amount>"<<std::endl;
            std::cout<<"deposit_position <UserId> <symbol> <quantity>"<<std::endl;
            std::cout<<std::endl;
        }

        else if(words[0]=="add_symbol" && words.size()==2){
            const Orderbook::Symbol sym = words[1];
            if(ex.addSymbol(sym)){
                std::cout<<"Successfully added the symbol : "<<sym<<std::endl;
            }
            else{
                std::cout<<"Error..! symbol : "<<sym<<" not edded"<<std::endl;
            }
        }

        else if(words[0]=="symbols" && words.size()==1){
            const auto s = ex.getSymbol();
            Cli::printSymbol(s);
        }



        else if(words[0]=="buy" && words.size()==5){

              try {
                    const Orderbook::UserId userId = stti(words[1]);
                    const Orderbook::Symbol sym = words[2];
                    Orderbook::Price myInt = stti(words[3]);
                    Orderbook::Quantity qn = stti(words[4]);
                    if(myInt<=0) {
                        std::cout<<"Error..! price is not positive number"<<std::endl;
                    }
                    else if(qn<=0){
                        std::cout<<"Error..! Quantity is not positive Integer"<<std::endl;
                    }
                    else if(!ex.hasSymbol(sym)){
                        std::cout<<"Error..! Unknown Symbol "<<sym<<std::endl;
                    }
                    else if(!ex.hasUser(userId)){
                        std::cout<<"Error..! Unknown User "<<userId<<std::endl;
                    }

                    else{
                        Exch::ExchangeOrderResult ok = ex.buy(sym,myInt, qn,userId);

                        printOrderResult(ok,"BuyOrder");
                        printBuyOrderList(ex,sym);
                        printSellOrderList(ex,sym);
                    }

                } catch (const std::exception& e) {
                    std::cout << "Error..! Please enter positive number in AskPrice and positive Integer in Quantity" << e.what();
                }
            }

        else if(words[0]=="sell" && words.size()==5){

                try {
                    const Orderbook::UserId userId = stti(words[1]);
                    const Orderbook::Symbol sym = words[2];
                    std::int64_t myInt = stti(words[3]);
                    std::int64_t qn = stti(words[4]);
                    if(myInt<=0) {
                        std::cout<<"Error..! Price is not positive number"<<std::endl;
                    }
                    else if(qn<=0){
                        std::cout<<"Error..! Quantity is not positive Integer"<<std::endl;

                    }

                    else if(!ex.hasSymbol(sym)){
                        std::cout<<"Error..! Unknown Symbol "<<sym<<std::endl;
                    }

                    else if(!ex.hasUser(userId)){
                        std::cout<<"Error..! Unknown User "<<userId<<std::endl;
                    }

                    else{
                        Exch::ExchangeOrderResult ok = ex.sell(sym,myInt,qn,userId);
                        printOrderResult(ok,"sellOrder");
                        printBuyOrderList(ex,sym);
                        printSellOrderList(ex,sym);
                    }

                } catch (const std::exception& e) {
                    std::cout << "Error..! Please enter positive number in AskPrice and positive Integer in Quantity" << e.what();
                }
        }

        else if(words[0]=="cancel" && words.size()==3){
            try{
                const Orderbook::UserId userId = stti(words[1]);
                std::int64_t myInt = stti(words[2]);
                if(myInt<=0) {
                    std::cout<<"Error..! Order id must be a positive integer"<<std::endl;
                }
                else if(!ex.hasUser(userId)){
                    std::cout<<"Error..! Unknown User "<<userId<<std::endl;
                }
                else{
                    auto ok = ex.cancelOrder(myInt, userId);
                    if(ok.cancelled){
                        const Orderbook::Symbol sym = ok.symbol;
                        std::cout<<"Successfully canceld the order id: "<<myInt<<" at "<<sym<<std::endl;
                        printBuyOrderList(ex,sym);
                        printSellOrderList(ex,sym);
                }
                else std::cout<<"Error..! Order id: "<<myInt<<" is not editable "<<ok.message<<std::endl;
            }
            }catch(const std::exception& e){
                std::cout<<"Error..! while canceling the order"<<std::endl;
            }
        }

        else if (words[0]=="book" && words.size()==2){
            const Orderbook::Symbol sym = words[1];
            if(!ex.hasSymbol(sym)){
                std::cout<<"Error..! Unknown Symbol "<<sym<<std::endl;
            }
            else{
                printBuyOrderList(ex,sym);
                printSellOrderList(ex,sym);
            }
        }

        else if(words[0]=="trades" && words.size()==2){
            const Orderbook::Symbol sym = words[1];
            if(!ex.hasSymbol(sym)){
                std::cout<<"Error..! Unknown Symbol "<<sym<<std::endl;
            }
            else printTrade(ex.getTrades(sym));
        }

        else if(words[0]=="best" && words.size()==2){
            const Orderbook::Symbol sym = words[1];
            if(!ex.hasSymbol(sym)){
                std::cout<<"Error..! Unknown Symbol "<<sym<<std::endl;
            }
            else printBest(ex,sym);
        }

        else if(words[0]=="clear" && words.size()==2){
            const Orderbook::Symbol sym = words[1];
            if(!ex.hasSymbol(sym)){
                std::cout<<"Error..! Unknown Symbol "<<sym<<std::endl;
            }
            else{
                ex.clearSymbol(sym);
                std::cout<<"Success "<<sym<<" Book got cleared"<<std::endl;
            }
        }

        else if(words[0]=="clear_all" && words.size()==1){
            ex.clearAll();
            std::cout<<"Success all books got cleared"<<std::endl;
        }

        else if((words[0]=="quit" || words[0]=="exit") && words.size()==1 ){
            exit = 1;
        }

        else if(words[0]=="add_user" && words.size()==1){
            try{
                const Orderbook::UserId userid = ex.addUser();
                std::cout<<"Succesfully added new User with User ID "<<userid<<std::endl;
            }catch (const std::exception& e) {
                    std::cout << "Error..! Can't add User Right now Please try again later" << e.what();
                }

        }

        else if(words[0]=="users" && words.size()==1){
            auto ans = ex.getUsers();
            Cli::printUsers(ans);
        }

        else if(words[0]=="account" && words.size()==2){
            try{
                const Orderbook::UserId userId = stti(words[1]);
                if(!ex.hasUser(userId)){
                    std::cout<<"Error..! Unknown User "<<userId<<std::endl;
                }
                else{
                    auto account = ex.getAccount(userId);
                    if(account) Cli::printAccount(account.value());
                }
            }catch(const std::exception &e){
                std::cout<<"Error..! Please enter valid userId in valid format "<<e.what();
            }
        }

        else if(words[0]=="deposit_cash" && words.size()==3){
            try{
                const Orderbook::UserId userId = stti(words[1]);
                const Orderbook::Cash amount = stti(words[2]);
                if(!ex.hasUser(userId)){
                    std::cout<<"Error..! Unknown User "<<userId<<std::endl;
                }
                else if(amount<=0){
                    std::cout<<"Error..! amount should be positive interger number"<<std::endl;
                }
                else{
                    bool ok = ex.depositCash(amount,userId);
                    if(ok) std::cout<<"Successfully deposit the amount "<<std::endl;
                    else std::cout<<"Error..! Cash deposit failed"<<std::endl;
                }

            }catch(const std::exception &e){
                std::cout<<"Error..! Please enter valid userId and amount in valid format "<<e.what();
            }
        }

        else if(words[0]=="withdraw_cash" && words.size()==3){
            try{
                const Orderbook::UserId userId = stti(words[1]);
                const Orderbook::Cash amount = stti(words[2]);
                if(!ex.hasUser(userId)){
                    std::cout<<"Error..! Unknown User "<<userId<<std::endl;
                }
                else if(amount<=0){
                    std::cout<<"Error..! amount should be positive interger number"<<std::endl;
                }
                else{
                    bool ok = ex.withdrawCash(amount,userId);
                    if(ok) std::cout<<"Successfully withdraw the amount "<<std::endl;
                    else std::cout<<"Error..! Insufficient Cash"<<std::endl;
                }

            }catch(const std::exception &e){
                std::cout<<"Error..! Please enter valid userId and amount in valid format "<<e.what();
            }
        }

        else if(words[0]=="deposit_position" && words.size()==4){
            try{
                const Orderbook::UserId userId = stti(words[1]);
                const Orderbook::Symbol sym = words[2];
                const Orderbook::Quantity quantity = stti(words[3]);
                if(!ex.hasUser(userId)){
                    std::cout<<"Error..! Unknown User "<<userId<<std::endl;
                }
                else if(!ex.hasSymbol(sym)){
                    std::cout<<"Error..! Unknown Symbol "<<sym<<std::endl;
                }
                else if(quantity<=0){
                    std::cout<<"Error..! Quantity is not positive Integer"<<std::endl;
                }
                else{
                    const auto total = ex.depositPosition(quantity,sym,userId);
                    if(total>0) std::cout<<"Successfully deposit the position "<<std::endl;
                    else std::cout<<"Error..! Position deposit failed"<<std::endl;
                }

            }catch(const std::exception &e){
                std::cout<<"Error..! Please enter valid userId and quantity in valid format "<<e.what();
            }
        }




        else{
            std::cout<<"Error..! Invalid command"<<std::endl;
        }
    }
}
