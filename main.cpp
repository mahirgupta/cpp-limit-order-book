//
//  main.cpp
//  OrderBook
//
//  Created by mahirgup on 07/05/26.
//
#include<algorithm>
#include<iostream>
#include<cstdlib>
#include<set>
#include<map>
#include<vector>
#include<sstream>
#include<climits>
using namespace std;



struct order {
    int askPrice;
    int Quantity;
    int orderId;
    bool type; // 1 for sell 0 for buy

    struct CompareBuy {
        bool operator()(const order *a, const order *b) const {
            if (a->askPrice == b->askPrice)
                return a->orderId < b->orderId;
            return a->askPrice > b->askPrice;
        }
    };

    struct CompareSell {
        bool operator()(const order *a, const order *b) const {
            if (a->askPrice == b->askPrice)
                return a->orderId < b->orderId;
            return a->askPrice < b->askPrice;
        }
    };
};

struct trade{
    int buyerOrderId;
    int sellerOrderId;
    int price;
    int quantity;
};



class OrderBook{
private:
    map<int,order*>Orders;
    set<order*,order::CompareBuy>buyOrderList;
    set<order*,order::CompareSell>sellOrderList;
    vector<trade>tradelist;
    int curOrderId = 0;
    
public:
    OrderBook(){};
    ~OrderBook(){
        clear();
    };
    
    int printcurOrderId(){
        return curOrderId;    
    }

    int makeBuyOrder(int ap, int qn){
        order *b = new order();
        b->askPrice = ap;
        b->Quantity = qn;
        b->orderId = ++curOrderId;
        b->type = 0;
        
        Orders[curOrderId] = b;
        buyOrderList.insert(b);
        return match(0);

        
    }

    int makeSellOrder(int ap, int qn){
        order *b = new order();
        b->askPrice = ap;
        b->Quantity = qn;
        b->orderId = ++curOrderId;
        b->type = 1;
        
        Orders[curOrderId] = b;
        sellOrderList.insert(b);
        return match(1);
    }
  
    void printBuyOrderList(){

        if(buyOrderList.size()==0){
            cout<<"No pending Buy Ask..."<<endl;
            return ;
        }
        cout<<"OrderId\tBuyAskPrice\tQuantity"<<endl;
        for(auto i:buyOrderList){
            cout<<i->orderId<<"\t"<<i->askPrice<<"\t"<<i->Quantity<<"\n";
        }
        cout<<endl;
        return ;
    }

    void printSellOrderlist(){

        if(sellOrderList.size()==0){
            cout<<"No pending Sell Ask..."<<endl;
            return ;
        }
        cout<<"OrderId\tSellAskPrice\tQuantity"<<endl;
        for(auto i:sellOrderList){
            cout<<i->orderId<<"\t"<<i->askPrice<<"\t"<<i->Quantity<<"\n";
        }
        cout<<endl;
        return ;
    }

    void printTrade(){
        if(tradelist.size()==0){
            cout<<"No Trade till now"<<endl;
            return ;
        }
        cout<<"buyer\tseller\tPrice\tQuantity"<<endl;
        for(int i = 0;i<int(tradelist.size());i++){
            cout<<tradelist[i].buyerOrderId<<"\t"<<tradelist[i].sellerOrderId<<"\t"<<tradelist[i].price<<"\t"<<tradelist[i].quantity<<endl;
        }
        cout<<endl;
        return;
    }
    
    void printTrade(int s){
        if(tradelist.size()==0){
            cout<<"No Trade till now"<<endl;
            return ;
        }
        cout<<"buyer\tseller\tPrice\tQuantity"<<endl;
        int sz = int(tradelist.size());
        for(int i = max(0,sz-s);i<sz;i++){
            cout<<tradelist[i].buyerOrderId<<"\t"<<tradelist[i].sellerOrderId<<"\t"<<tradelist[i].price<<"\t"<<tradelist[i].quantity<<endl;
        }
        cout<<endl;
        return;
        
    }

    bool cancelOrder(int id){
        if(Orders.find(id)==Orders.end()){
            return 0;
        }
        bool type = Orders[id]->type;
        if(type==0){
            buyOrderList.erase(Orders[id]);
        }
        else{
            sellOrderList.erase(Orders[id]);
        }
        delete Orders[id];
        Orders.erase(id);
        return 1;
    }   

    void printbest(){
        cout<<"Best bid: ";
        if(buyOrderList.size()) cout<<((*buyOrderList.begin())->askPrice)<<endl;
        else cout<<"none"<<endl;
        cout<<"Best ask: ";
        if(sellOrderList.size()) cout<<((*sellOrderList.begin())->askPrice)<<endl;
        else cout<<"none"<<endl;

        cout<<"Spread: ";
        if(buyOrderList.size() && sellOrderList.size()) cout<<((*sellOrderList.begin())->askPrice  -  (*buyOrderList.begin())->askPrice)<<endl;
        else cout<<"none"<<endl;
        cout<<endl;

    }

    int match(bool side){ // side =0 for buy order side =1 for sell order
        if(!buyOrderList.size() || !sellOrderList.size() ) return 0;
        if((*buyOrderList.begin())->askPrice< (*sellOrderList.begin())->askPrice)return 0;
        int cnt = 0;
       while(buyOrderList.size() && sellOrderList.size() && ((*buyOrderList.begin())->askPrice>= (*sellOrderList.begin())->askPrice) ){

            int buyid = (*buyOrderList.begin())->orderId;
            int sellid = (*sellOrderList.begin())->orderId;
            int price = !side?(*sellOrderList.begin())->askPrice:(*buyOrderList.begin())->askPrice;
            int qn = min((*buyOrderList.begin())->Quantity, (*sellOrderList.begin())->Quantity);
            makeTrade(buyid,sellid, qn,price);
            cnt++;


            (*buyOrderList.begin())->Quantity -= qn;
            (*sellOrderList.begin())->Quantity -= qn;

            if((*buyOrderList.begin())->Quantity==0){
                buyOrderList.erase(buyOrderList.begin());
                delete Orders[buyid];
                Orders.erase(buyid);

            } 
            if((*sellOrderList.begin())->Quantity==0) {
                sellOrderList.erase(sellOrderList.begin());
                delete Orders[sellid];
                Orders.erase(sellid);
            }
       }

        return cnt;
    }

    void makeTrade(int buyId, int sellId, int qn, int pr){
        trade t;
        t.buyerOrderId = buyId;
        t.sellerOrderId = sellId;
        t.price = pr;
        t.quantity = qn;
        tradelist.push_back(t);
    }

    void clear(){
        
        buyOrderList.clear();
        sellOrderList.clear();
        tradelist.clear();
        for(auto i:Orders) delete(i.second);
        Orders.clear();
        curOrderId = 0;
    }

};


int stti(string &s){
    long long  ans = 0;

    for(auto i:s){
        if((i-'0') <0 || (i-'0') >9) return -1;
        ans= ans*10+(i-'0');
        if(ans>INT_MAX) return -1;
    }
    if(ans>INT_MAX) ans = -1;
    return int(ans);
}


void CLI(OrderBook &bk){
    bk.printBuyOrderList();
    bk.printSellOrderlist();
    string inp;
    bool exit=0;;
    while(!exit){
        cout<<"Enter any command for help type \"help\""<<endl;
        if(!std::getline(std::cin, inp)){
            exit=1;
            break;
        };
        stringstream ss(inp);
        string word;
        vector<string>words;
        while(ss>>word){
            words.push_back(word);
        }
        if(words.size()==0) continue;
        if(words[0] == "help" && words.size()==1){
            cout<<"Available commands: help, buy, sell"<<endl;
            cout<<"buy <askPrice> <quantity>"<<endl;
            cout<<"sell <askPrice> <quantity>"<<endl;
            cout<<"cancel <orderid>"<<endl;
            cout<<"book"<<endl;
            cout<<"trades"<<endl;
            cout<<"best"<<endl;
            cout<<"clear"<<endl;
            cout<<"exit/quit"<<endl;
            cout<<endl;
        }

        else if(words[0]=="buy" && words.size()==3){

              try {
                    int myInt = stti(words[1]);
                    int qn = stti(words[2]);
                    if(myInt<=0) {
                        cout<<"Error..! Ask price is not positive number"<<endl;
                    }
                    else if(qn<=0){
                        cout<<"Error..! Quantity is not positive Integer"<<endl;

                    }

                    else{
                        int ok = bk.makeBuyOrder(myInt,qn);
                        cout<<"Success"<<endl;
                        if(ok>0)bk.printTrade(ok);
                        bk.printBuyOrderList();
                        bk.printSellOrderlist();
                    }    
                    
                } catch (const std::exception& e) {
                    cout << "Error..! Please enter positive number in AskPrice and positive Integer in Quantity" << e.what();
                }
            }
            
        else if(words[0]=="sell" && words.size()==3){
                
                try {
                    int myInt = stti(words[1]);
                    int qn = stti(words[2]);
                    if(myInt<=0) {
                        cout<<"Error..! Ask price is not positive number"<<endl;
                    }
                    else if(qn<=0){
                        cout<<"Error..! Quantity is not positive Integer"<<endl;
                        
                    }
                    
                    else{
                        int ok = bk.makeSellOrder(myInt,qn);
                        cout<<"Success"<<endl;
                        if(ok>0)bk.printTrade(ok);
                        bk.printBuyOrderList();
                        bk.printSellOrderlist();
                    }    
                    
                } catch (const std::exception& e) {
                    cout << "Error..! Please enter positive number in AskPrice and positive Integer in Quantity" << e.what();
                }
        }

        else if(words[0]=="cancel" && words.size()==2){
            try{
                int myInt = stti(words[1]);
                if(myInt<=0) {
                    cout<<"Error..! Order id must be a positive integer"<<endl;
                }
                else{
                    bool ok = bk.cancelOrder(myInt);
                if(ok>0){
                    cout<<"Successfully canceld the order id: "<<myInt<<endl;
                    bk.printBuyOrderList();
                    bk.printSellOrderlist();
                }
                else cout<<"Error..! Order id: "<<myInt<<" is not editable"<<endl;
            }
            }catch(const std::exception& e){
                cout<<"Error..! while canceling the order"<<endl;
            }
        }

        else if (words[0]=="book" && words.size()==1){
            bk.printBuyOrderList();
            bk.printSellOrderlist();
        }

        else if(words[0]=="trades" && words.size()==1){
            bk.printTrade();
        }

        else if(words[0]=="best" && words.size()==1){
            bk.printbest();
        }

        else if(words[0]=="clear" && words.size()==1){
            bk.clear();
            cout<<"Success Book got cleared"<<endl;
        }

        else if((words[0]=="quit" || words[0]=="exit") && words.size()==1 ){
            exit = 1;
        }

        else{
            cout<<"Error..! Invalid command"<<endl;
        }
    }


}




int main(int argc, const char * argv[]) {
    OrderBook bk;
    // bk.makeBuyOrder(100,2);
    // bk.makeBuyOrder(150,4);
    // bk.makeBuyOrder(120,3);
    // bk.makeBuyOrder(150,5);
    // bk.makeBuyOrder(110,1);
    // bk.makeSellOrder(151,2);
    // bk.makeSellOrder(155,2);

    CLI(bk);
    
    return EXIT_SUCCESS;
}
