#include "database.h"
using namespace std;
using namespace pqxx;

bool createAccount(connection* C, const char* char_id, const char* char_balance){
    double balance = atof(char_balance);
    long id = atol(char_id);
    work W(*C);
    stringstream ss;
    // if already exist, return error
    bool flag = queryAccount(W, atol(char_id));
    if(flag){
        return false;
    }
    ss << "INSERT INTO ACCOUNT (ACT_ID, ACT_BALANCE) " << "VALUES (" << W.quote(id) << ", " << W.quote(balance)<< ");";
    try{
        W.exec(ss.str());
        W.commit();
    }
    catch(const exception& e){
        cerr << e.what() << endl;
        W.abort();
        return false;
    }
    return true;
}

bool querySymbol(work& W, string& sym_name){
    stringstream ss;
    ss << "SELECT * FROM SYMBOL WHERE SYM_NAME=" << W.quote(sym_name) << ";";
    
    result res(W.exec(ss.str()));
    return res.size()!=0;
    
}

bool queryAccount(work& W, long id){
    stringstream ss;
    ss << "SELECT * FROM ACCOUNT WHERE ACT_ID=" << W.quote(id) << ";";
    result res(W.exec(ss.str()));
    return res.size()!=0;
}
bool createSymbol(connection* C, const char* char_sym, const char* char_id, const char* char_num){
    int num = atoi(char_num);
    long id = atol(char_id);
    string sym(char_sym);
    work W(*C);
    if(sym.size()==0||!queryAccount(W,id)){
        // empty sym name or null account
        return false;
    }
    // create a new one
    
    if(!querySymbol(W,sym)){
        stringstream ss;
        ss << "INSERT INTO SYMBOL (SYM_NAME) " << "VALUES (" << W.quote(sym) << ");";
        W.exec(ss.str());
    }
    try{
        W.commit();
    }
    catch(const exception& e){
        cerr << e.what() << endl;
        return false;
    }
    return true;
}

void createPosition(connection* C, const char* char_sym, const char* char_id, const char* char_num){
    work W(*C);
    stringstream ss;
    string sym(char_sym);
    long id = atol(char_id);
    double num = atof(char_num);

    ss << "INSERT INTO POSITION (SYM_NAME, ACT_ID, POS_AMOUNT) " << "VALUES ("
    << W.quote(sym)<< ", " << W.quote(id) << ", " << W.quote(num) << ") "
    <<" ON CONFLICT ON CONSTRAINT POSITION_PK "
    << "DO UPDATE SET POS_AMOUNT = POSITION.POS_AMOUNT + " << W.quote(num) << ";";
    try{
        W.exec(ss.str());
        W.commit();
    }
    catch(const exception& e){
        cerr << e.what() << endl;
        W.abort();
    }
}

TiXmlElement* handleOrder(connection *C, const char* char_id, string& sym, double amount, double limit){
    cout << "inside handleOrder:" <<endl;
    work W(*C);
    TiXmlElement* res;
    // buy
    if(amount>0){
        cout << "inside handleOrder:amount>0" <<endl;
        double balance = queryBalance(W,char_id);
        // validate balance
        if(balance<amount*limit){
            res = new TiXmlElement("error");
            res->LinkEndChild(new TiXmlText("buy: Insufficient balance"));
            return res;
        }else{
            double newBalance = -amount*limit;
            res = new TiXmlElement("opened");
            updateAccountBalance(W,char_id,newBalance);
        }
    }
    // sell
    else{
        cout << "inside handleOrder:amount<=0" <<endl;
        double shares = queryShares(W,char_id,sym);
        cout << "finish queryShares" <<endl;
        if(shares<-amount){
            cout << "inside handleOrder:amount<=0:shares<amount" <<endl;
            res = new TiXmlElement("error");
            res->LinkEndChild(new TiXmlText("sell: Insufficient shares"));
            return res;
        }else{
            cout << "inside handleOrder:amount<=0:shares>=amount" <<endl;
            double newShares = amount;
            res = new TiXmlElement("opened");
            updatePositionShares(W, char_id, sym, newShares);
        }
    }
    // create transaction
    cout << "create transaction" << endl;
    long tran_id = createTran(W,char_id,sym,amount,limit);
    cout << "create transaction: finish" << endl;
    try{
        W.commit();
    }
    catch(const exception &e){
        W.abort();
        cerr<<e.what()<<endl;
        return nullptr;
    }
    // order matching
    if(amount>0){
        // buy
        matchBuyOrder(C,tran_id);
    }
    else{
        // sell
        matchSellOrder(C,tran_id);
    }
    return res;
}

double queryBalance(work& W, const char* char_id){
    stringstream ss;
    ss << "SELECT ACT_BALANCE FROM ACCOUNT WHERE" << " ACT_ID = " << W.quote(char_id) << ";";
    result res(W.exec(ss.str()));
    return res.begin()[0].as<double>();
}

void updateAccountBalance(work& W, const char* char_id, double balance){
    stringstream ss;
    ss << "UPDATE ACCOUNT SET ACT_BALANCE = ACT_BALANCE + " << W.quote(balance) << "WHERE ACT_ID = " << W.quote(char_id) << ";";
    W.exec(ss.str());
}

double queryShares(work& W, const char* char_id, string &sym){
    cout << sym << endl;
    cout << W.quote(char_id) << endl;
    stringstream ss;
    ss << "SELECT POS_AMOUNT FROM POSITION WHERE"
    << " ACT_ID = " << W.quote(char_id)
    << " AND SYM_NAME = " << W.quote(sym) << ";";
    result res(W.exec(ss.str()));
    cout << "queryShares:finish" << endl;
    cout << res.size() << endl;
    if(res.size() == 0) {
        return 0;
    }
    return res.begin()[0].as<double>();
}

void updatePositionShares(work& W, const char* char_id, string& sym, double shares){
    cout << shares <<endl;
    stringstream ss;
    ss << "INSERT INTO POSITION (SYM_NAME, ACT_ID, POS_AMOUNT) " << "VALUES ("
    << W.quote(sym)<< ", " << W.quote(char_id) << ", " << W.quote(shares) << ")"
    <<" ON CONFLICT ON CONSTRAINT POSITION_PK "
    << "DO UPDATE SET POS_AMOUNT = POSITION.POS_AMOUNT + " << W.quote(shares) << ";";
    W.exec(ss.str());   
}

long createTran(work& W, const char* char_id, string& sym, double amount, double limit){
    cout << "inside createTran" << endl;
    stringstream ss;
    // generate time
    long time = generateTime();
    ss << "INSERT INTO TRANSACTIONS "
    << "(TRAN_STATUS, SYM_NAME, ACT_ID, TRAN_AMOUNT, TRAN_LIMIT_PRICE, TRAN_TIME)"
    << "VALUES ( " << W.quote("OPEN") << ", " << W.quote(sym) << ", " << W.quote(char_id) << ", " << W.quote(amount) << ", "
    << W.quote(limit) << ", " << W.quote(time) << ") " << "RETURNING TRAN_ID;";
    result res(W.exec(ss.str()));
    long tran_id = res.begin()[0].as<long>();
    cout << "inside createTran: finish" << endl;
    return tran_id;
}

void updateTransaction(work& W, long tran_id, double amount){
    stringstream ss;
    ss << "UPDATE TRANSACTIONS SET TRAN_AMOUNT = " << W.quote(amount) << "WHERE TRAN_ID = " << W.quote(tran_id) << ";";
    W.exec(ss.str());
}

void createOrder(work& W, long tran_id, double order_amount, double order_price, long order_time){
    stringstream ss;
    ss << "INSERT INTO ORDERS (TRAN_ID, ODR_AMOUNT, ODR_PRICE, ODR_TIME) VALUES ( "
    << W.quote(tran_id) << ", " << W.quote(order_amount) << ", " << W.quote(order_price) 
    << ", " << W.quote(order_time) << ");";
    W.exec(ss.str());
}

TiXmlElement* queryTran(connection* C, const char* char_tran_id){

    work W(*C);
    TiXmlElement* res = new TiXmlElement("status");
    stringstream ss1;
    ss1 << "SELECT TRAN_STATUS, TRAN_AMOUNT FROM TRANSACTIONS WHERE TRAN_ID = " << W.quote(char_tran_id) << ";";
    result res1(W.exec(ss1.str()));
    if(res1.size()==0){
        return nullptr;
    }
    
    if(res1.begin()[0].as<string>()==string("OPEN")){
        // status open
        TiXmlElement* subres = new TiXmlElement("open");
        subres->SetAttribute("shares=",res1.begin()[1].as<double>());
        res->LinkEndChild (subres);
    }
    else{
        // status canceled
        cout << "queryTran : else"<<endl;
        TiXmlElement* subres = new TiXmlElement("canceled");
        subres->SetAttribute("shares=",res1.begin()[1].as<double>());
        cout << "queryTran : else: subres"<<endl;
        // query cancel time
        stringstream ss;
        ss << "SELECT CAN_TIME FROM CANCEL WHERE TRAN_ID = " << W.quote(char_tran_id) << ";";
        result r(W.exec(ss.str()));
        cout << "queryTran : else: result"<<endl;
        long cancelTime = r.begin()[0].as<long>();
        cout << "queryTran : else: cancelTime"<<endl;
        subres->SetAttribute("time=",cancelTime);
        res->LinkEndChild (subres);
    }
    TiXmlElement* subres = new TiXmlElement("executed");
    stringstream ss2;
    ss2 << "SELECT ODR_AMOUNT, ODR_PRICE, ODR_TIME FROM ORDERS WHERE TRAN_ID = " << W.quote(char_tran_id) << ";";
    result res2(W.exec(ss2.str()));
    for(result::const_iterator it=res2.begin();it!=res2.end();++it){
        TiXmlElement* subres = new TiXmlElement("executed");
        subres->SetAttribute("shares=",it[0].as<double>());
        subres->SetAttribute("price=",it[1].as<double>());
        subres->SetAttribute("time=",it[2].as<long>());
        res->LinkEndChild (subres);
    }
    W.commit();
    return res;
}

TiXmlElement* cancelTran(connection* C, const char* char_tran_id){
    work W(*C);
    TiXmlElement* res = new TiXmlElement("canceled");  
    long curTime = generateTime(); 

    // query open transaction
    stringstream ss1;
    ss1 << "SELECT TRAN_STATUS, TRAN_AMOUNT, TRAN_LIMIT_PRICE, ACT_ID, SYM_NAME FROM TRANSACTIONS WHERE TRAN_ID=" << W.quote(char_tran_id) << ";";
    result res1(W.exec(ss1.str()));
    if(res1.size()==0||res1.begin()[0].as<string>()==string("CANCELED")){
        return nullptr;
    }
    // query executed orders
    stringstream ss2;
    ss2 << "SELECT ODR_AMOUNT, ODR_PRICE, ODR_TIME FROM ORDERS WHERE TRAN_ID = " << W.quote(char_tran_id) << ";";
    result res2(W.exec(ss2.str()));
    // update tran_status
    stringstream ss3;
    ss3 << "UPDATE TRANSACTIONS SET TRAN_STATUS=" << W.quote("CANCELED") << " WHERE TRAN_ID= " << W.quote(char_tran_id) << ";";
    W.exec(ss3.str());
    // update cancel time
    stringstream ss4;
    ss4 << "INSERT INTO CANCEL (TRAN_ID, CAN_TIME) VALUES(" << W.quote(char_tran_id) << ", " << W.quote(curTime)<< ");";
    W.exec(ss4.str());
    // <canceled shares=xxx time=xxx/>
    TiXmlElement* substr_canceled = new TiXmlElement("canceled");
    substr_canceled->SetAttribute("shares=",res1.begin()[1].as<double>());
    substr_canceled->SetAttribute("time=",curTime);  
    res->LinkEndChild (substr_canceled);
    // <executed shares=xxx price=xxx time=xxx/>
    for(result::const_iterator it=res2.begin();it!=res2.end();++it){
        TiXmlElement* substr_executed = new TiXmlElement("executed");
        substr_executed->SetAttribute("shares=",it[0].as<double>());
        substr_executed->SetAttribute("price=",it[1].as<double>());
        substr_executed->SetAttribute("time=",it[2].as<long>());
        res->LinkEndChild (substr_executed);
    }
    
    // if buy order, refund balance
    if(res1.begin()[1].as<double>()>0){
        string temp = res1.begin()[3].as<string>();
        const char* char_id = temp.c_str();
        // new balance = original balance + tran_amount * tran_limit_price
        double newBalance = res1.begin()[1].as<double>() * res1.begin()[2].as<double>();
        updateAccountBalance(W,char_id,newBalance);

    }
    // if sell order, refund position
    if(res1.begin()[1].as<double>()<0){
        string temp = res1.begin()[3].as<string>();
        const char* char_id = temp.c_str();
        string sym = res1.begin()[4].as<string>();
        updatePositionShares(W,char_id,sym,-res1.begin()[1].as<double>());
    }
    try{
        W.commit();
    }
    catch(const exception& e){
        cerr << e.what() << endl;
        W.abort();
        return cancelTran(C, char_tran_id);
    }
    return res;
}

