#include<iostream>
#include<Cli/Cli.hpp>
#include<string>
#include<sstream>
#include<vector>



void Cli::printBuyOrderList(Orderbook::OrderBook *bk){
    if(bk->getBestBid()==-1){
        std::cout<<"No pending Bid ...\n";
        return ;
    }
    std::cout<<"Bid::\n";
    std::cout<<"OrderId\tPrice\tQuantity\n";
    for(auto &i:bk->getBuyOrders()){
        std::cout<<i.orderId<<"\t"<<i.price<<"\t"<<i.quantity<<"\n";
    }
    std::cout<<"\n";
    return ;

}

void Cli::printSellOrderList(Orderbook::OrderBook *bk){
    if(bk->getBestAsk()==-1){
        std::cout<<"No pending Ask...\n";
        return ;
    }
    std::cout<<"Ask::\n";
    std::cout<<"OrderId\tPrice\tQuantity\n";
    for(auto &i:bk->getSellOrders()){
        std::cout<<i.orderId<<"\t"<<i.price<<"\t"<<i.quantity<<"\n";
    }
    std::cout<<"\n";
    return ;

}

void Cli::printTrade(std::vector<Orderbook::Trade>trade){
    if(trade.empty()){
        std::cout<<"No Trade till now\n";
        return ;
    }
    std::cout<<"buyer\tseller\tPrice\tQuantity\n";
    for(auto &i:trade){
        std::cout<<i.buyerOrderId<<"\t"<<i.sellerOrderId<<"\t"<<i.price<<"\t"<<i.quantity<<"\n";
    }
    std::cout<<"\n";
    return;
}

void Cli::printBest(Orderbook::OrderBook *bk){
    std::cout<<"Best Bid\tBest Ask\tSpread\n";
    if(bk->getBestBid()==-1)std::cout<<"none\t";
    else std::cout<<bk->getBestBid()<<"\t";
    if(bk->getBestAsk()==-1)std::cout<<"none\t";
    else std::cout<<bk->getBestAsk()<<"\t";
    if(bk->getSpread()==-1)std::cout<<"none\n";
    else std::cout<<bk->getSpread()<<"\n";
    std::cout<<"\n";
    return;
}

int64_t stti(std::string &s){
    int64_t ans = 0;

    for(auto i:s){
        if((i-'0') <0 || (i-'0') >9) return -1;
        if(ans>(std::numeric_limits<std::int64_t>::max() - (i-'0'))/10) return -1;
        ans= ans*10+(i-'0');
    }
    return int64_t(ans);
}

void printOrderResult(Orderbook::OrderResult &ok, std::string OrderType){
    if(ok.accepted){
        std::cout<<"Successfully Placed "<<OrderType<<" with OrderId: "<<ok.orderId<<std::endl;
        if(!ok.trade.empty()){
            std::cout<<"Executed Trade"<<std::endl;
            Cli::printTrade(ok.trade);

            if(ok.remainQuantity>0){
                std::cout<<"Partial Order reamins with orderId: "<<ok.orderId<<" Quantity: "<<ok.remainQuantity<<std::endl;
            }
            else std::cout<<"Order Complete Fully"<<std::endl;
        }
    }
    else{
        std::cout<<"Error..! Order is not placed"<<std::endl;
    }
}


void Cli::run(Orderbook::OrderBook *bk){
    printBuyOrderList(bk);
    printSellOrderList(bk);
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
            std::cout<<"buy <askPrice> <quantity>"<<std::endl;
            std::cout<<"sell <askPrice> <quantity>"<<std::endl;
            std::cout<<"cancel <orderid>"<<std::endl;
            std::cout<<"book"<<std::endl;
            std::cout<<"trades"<<std::endl;
            std::cout<<"best"<<std::endl;
            std::cout<<"clear"<<std::endl;
            std::cout<<"exit/quit"<<std::endl;
            std::cout<<std::endl;
        }

        else if(words[0]=="buy" && words.size()==3){

              try {
                    std::int64_t myInt = stti(words[1]);
                    std::int64_t qn = stti(words[2]);
                    if(myInt<=0) {
                        std::cout<<"Error..! Ask price is not positive number"<<std::endl;
                    }
                    else if(qn<=0){
                        std::cout<<"Error..! Quantity is not positive Integer"<<std::endl;

                    }

                    else{
                        Orderbook::OrderResult ok = bk->makeBuyOrder(myInt,qn);
                        printOrderResult(ok,"BuyOrder");
                        printBuyOrderList(bk);
                        printSellOrderList(bk);
                    }    
                    
                } catch (const std::exception& e) {
                    std::cout << "Error..! Please enter positive number in AskPrice and positive Integer in Quantity" << e.what();
                }
            }
            
        else if(words[0]=="sell" && words.size()==3){
                
                try {
                    std::int64_t myInt = stti(words[1]);
                    std::int64_t qn = stti(words[2]);
                    if(myInt<=0) {
                        std::cout<<"Error..! Ask price is not positive number"<<std::endl;
                    }
                    else if(qn<=0){
                        std::cout<<"Error..! Quantity is not positive Integer"<<std::endl;
                        
                    }
                    
                    else{
                        auto ok = bk->makeSellOrder(myInt,qn);
                        printOrderResult(ok,"sellOrder");
                        printBuyOrderList(bk);
                        printSellOrderList(bk);
                    }    
                    
                } catch (const std::exception& e) {
                    std::cout << "Error..! Please enter positive number in AskPrice and positive Integer in Quantity" << e.what();
                }
        }

        else if(words[0]=="cancel" && words.size()==2){
            try{
                int myInt = stti(words[1]);
                if(myInt<=0) {
                    std::cout<<"Error..! Order id must be a positive integer"<<std::endl;
                }
                else{
                    bool ok = bk->cancelOrder(myInt);
                if(ok>0){
                    std::cout<<"Successfully canceld the order id: "<<myInt<<std::endl;
                    printBuyOrderList(bk);
                    printSellOrderList(bk);
                }
                else std::cout<<"Error..! Order id: "<<myInt<<" is not editable"<<std::endl;
            }
            }catch(const std::exception& e){
                std::cout<<"Error..! while canceling the order"<<std::endl;
            }
        }

        else if (words[0]=="book" && words.size()==1){
            printBuyOrderList(bk);
            printSellOrderList(bk);
        }

        else if(words[0]=="trades" && words.size()==1){
            printTrade(bk->getTrade());
        }

        else if(words[0]=="best" && words.size()==1){
            printBest(bk);
        }

        else if(words[0]=="clear" && words.size()==1){
            bk->clear();
            std::cout<<"Success Book got cleared"<<std::endl;
        }

        else if((words[0]=="quit" || words[0]=="exit") && words.size()==1 ){
            exit = 1;
        }

        else{
            std::cout<<"Error..! Invalid command"<<std::endl;
        }
    }
}




