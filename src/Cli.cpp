#include<iostream>
#include<Cli/Cli.hpp>
#include<cstdint>
#include<limits>
#include<string>
#include<sstream>
#include<vector>



void Cli::printBuyOrderList(Exch::Exchange &ex, const Orderbook::Symbol& sym){
    if(ex.getBestBid(sym)==-1){
        std::cout<<"No pending Bid ...\n";
        return ;
    }
    std::cout<<sym<<" Bid::\n";
    std::cout<<"OrderId\tPrice\tQuantity\n";
    for(auto &i:ex.getBuyOrders(sym)){
        std::cout<<i.orderId<<"\t"<<i.price<<"\t"<<i.quantity<<"\n";
    }
    std::cout<<"\n";
    return ;

}

void Cli::printSellOrderList(Exch::Exchange& ex, const Orderbook::Symbol& sym){
    if(ex.getBestAsk(sym)==-1){
        std::cout<<"No pending Ask...\n";
        return ;
    }
    std::cout<<sym<<" Ask::\n";
    std::cout<<"OrderId\tPrice\tQuantity\n";
    for(auto &i:ex.getSellOrders(sym)){
        std::cout<<i.orderId<<"\t"<<i.price<<"\t"<<i.quantity<<"\n";
    }
    std::cout<<"\n";
    return ;

}

void Cli::printTrade(const std::vector<Exch::ExchangeTrade>& trade){
    if(trade.empty()){
        std::cout<<"No Trade till now\n";
        return ;
    }
    std::cout<<"buyer\tseller\tPrice\tQuantity\tSymbol\n";
    for(auto &i:trade){
        std::cout<<i.buyerOrderId<<"\t"<<i.sellerOrderId<<"\t"<<i.price<<"\t"<<i.quantity<<"\t"<<i.symbol<<"\n";
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

void Cli::printBest(const Exch::Exchange& ex, const Orderbook::Symbol& sym){
    std::cout<<"Best Bid\tBest Ask\tSpread\n";
    if(ex.getBestBid(sym)==-1)std::cout<<"none\t";
    else std::cout<<ex.getBestBid(sym)<<"\t";
    if(ex.getBestAsk(sym)==-1)std::cout<<"none\t";
    else std::cout<<ex.getBestAsk(sym)<<"\t";
    if(ex.getSpread(sym)==-1)std::cout<<"none\n";
    else std::cout<<ex.getSpread(sym)<<"\n";
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
        std::cout<<"Error..! Order is not placed"<<std::endl;
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
            std::cout<<"buy <symbol> <price> <quantity>"<<std::endl;
            std::cout<<"sell <symbol> <price> <quantity>"<<std::endl;
            std::cout<<"cancel <orderid>"<<std::endl;
            std::cout<<"book <symbol>"<<std::endl;
            std::cout<<"trades <symbol>"<<std::endl;
            std::cout<<"best <symbol>"<<std::endl;
            std::cout<<"clear <symbol>"<<std::endl;
            std::cout<<"clear_all"<<std::endl;
            std::cout<<"exit/quit"<<std::endl;
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



        else if(words[0]=="buy" && words.size()==4){

              try {
                    const Orderbook::Symbol sym = words[1];
                    Orderbook::Price myInt = stti(words[2]);
                    Orderbook::Quantity qn = stti(words[3]);
                    if(myInt<=0) {
                        std::cout<<"Error..! price is not positive number"<<std::endl;
                    }
                    else if(qn<=0){
                        std::cout<<"Error..! Quantity is not positive Integer"<<std::endl;
                    }

                    else{
                        Exch::ExchangeOrderResult ok = ex.buy(sym,myInt, qn);

                        printOrderResult(ok,"BuyOrder");
                        printBuyOrderList(ex,sym);
                        printSellOrderList(ex,sym);
                    }    
                    
                } catch (const std::exception& e) {
                    std::cout << "Error..! Please enter positive number in AskPrice and positive Integer in Quantity" << e.what();
                }
            }
            
        else if(words[0]=="sell" && words.size()==4){
                
                try {
                    const Orderbook::Symbol sym = words[1];
                    std::int64_t myInt = stti(words[2]);
                    std::int64_t qn = stti(words[3]);
                    if(myInt<=0) {
                        std::cout<<"Error..! Price is not positive number"<<std::endl;
                    }
                    else if(qn<=0){
                        std::cout<<"Error..! Quantity is not positive Integer"<<std::endl;
                        
                    }
                    
                    else{
                        Exch::ExchangeOrderResult ok = ex.sell(sym,myInt,qn);
                        printOrderResult(ok,"sellOrder");
                        printBuyOrderList(ex,sym);
                        printSellOrderList(ex,sym);
                    }    
                    
                } catch (const std::exception& e) {
                    std::cout << "Error..! Please enter positive number in AskPrice and positive Integer in Quantity" << e.what();
                }
        }

        else if(words[0]=="cancel" && words.size()==2){
            try{
                std::int64_t myInt = stti(words[1]);
                if(myInt<=0) {
                    std::cout<<"Error..! Order id must be a positive integer"<<std::endl;
                }
                else{
                    bool ok = ex.cancelOrder(myInt);
                    if(ok>0){
                        const Orderbook::Symbol sym = ex.getSymbolfromid(myInt);
                        std::cout<<"Successfully canceld the order id: "<<myInt<<" at "<<sym<<std::endl;
                        printBuyOrderList(ex,sym);
                        printSellOrderList(ex,sym);
                }
                else std::cout<<"Error..! Order id: "<<myInt<<" is not editable"<<std::endl;
            }
            }catch(const std::exception& e){
                std::cout<<"Error..! while canceling the order"<<std::endl;
            }
        }

        else if (words[0]=="book" && words.size()==2){
            const Orderbook::Symbol sym = words[1];
            printBuyOrderList(ex,sym);
            printSellOrderList(ex,sym);
        }
        
        else if(words[0]=="trades" && words.size()==2){
            const Orderbook::Symbol sym = words[1];
            printTrade(ex.getTrades(sym));
        }
        
        else if(words[0]=="best" && words.size()==2){
            const Orderbook::Symbol sym = words[1];
            printBest(ex,sym);
        }
        
        else if(words[0]=="clear" && words.size()==2){
            const Orderbook::Symbol sym = words[1];
            ex.clearSymbol(sym);
            std::cout<<"Success "<<sym<<" Book got cleared"<<std::endl;
        }

        else if(words[0]=="clear_all" && words.size()==1){
            ex.clearAll();
            std::cout<<"Success all books got cleared"<<std::endl;
        }

        else if((words[0]=="quit" || words[0]=="exit") && words.size()==1 ){
            exit = 1;
        }

        else{
            std::cout<<"Error..! Invalid command"<<std::endl;
        }
    }
}


