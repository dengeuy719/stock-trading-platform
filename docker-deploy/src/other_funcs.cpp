#include"other_funcs.h"
#include"database.h"

long generateTime(){
    time_t cur = time(nullptr);
    std::stringstream ss;
    ss << cur;
    long res;
    ss >> res;
    return res;
}

void matchBuyOrder(connection* C,long tran_id){
    work W(*C);
    stringstream query;
    query << "SELECT SYM_NAME, ACT_ID, TRAN_LIMIT_PRICE, TRAN_AMOUNT, TIME FROM TRANSACTIONS "
    << "WHERE TRAN_ID = " << W.quote(tran_id) << ";";
    result res(W.exec(query.str()));
    string sym = res.begin()[0].as<string>();
    string act_id = res.begin()[1].as<string>();
    double limit_price = res.begin()[2].as<double>();
    double tran_amount = res.begin()[3].as<double>();
    long tran_time = res.begin()[4].as<long>();
    while(tran_amount>0){ 
        stringstream matching;
        matching << "SELECT TRAN_ID, ACT_ID, TRAN_LIMIT_PRICE, TRAN_AMOUNT, TIME FROM TRANSACTIONS "
        << "WHERE SYM_NAME = " << W.quote(sym) << " AND TRAN_STATUS = " << W.quote("OPEN") 
        << " AND TRAN_AMOUNT < 0 AND TRAN_LIMIT_PRICE <= " << W.quote(limit_price)
        << "ORDER BY TRAN_LIMIT_PRICE ASC, TIME ASC;";
        result matching_res(W.exec(matching.str()));
        for(auto it = matching_res.begin(); it != matching_res.end()&&tran_amount>0; ++it){
            long seller_tran_id = it[0].as<long>();
            string seller_act_id = it[1].as<string>();
            double seller_price = it[2].as<double>();
            double seller_amount = it[3].as<double>();
            long seller_time = it[4].as<long>();
            long cur_time = generateTime();
            if((-seller_amount)<tran_amount){
                // cut buy order
                tran_amount+=seller_amount;
                double seller_income = seller_price*seller_amount;
                double buyer_refund = (limit_price-seller_price)*seller_amount;
                // refund
                updateAccountBalance(W,seller_act_id.c_str(),seller_income);
                updateAccountBalance(W,act_id.c_str(),buyer_refund);
                updatePositionShares(W,act_id.c_str(),sym,-seller_amount);
                updateTransaction(W,tran_id,tran_amount);
                createOrder(W, tran_id, seller_amount, seller_price, cur_time);
            }
            else{
                // cut sell order
                double seller_income = seller_price*tran_amount;
                double buyer_refund = (limit_price-seller_price)*tran_amount;
                // refund
                updateAccountBalance(W,seller_act_id.c_str(),seller_income);
                updateAccountBalance(W,act_id.c_str(),buyer_refund);
                updatePositionShares(W,act_id.c_str(),sym,tran_amount);
                updateTransaction(W,tran_id,tran_amount);
                createOrder(W, tran_id, tran_amount, seller_price, cur_time);
                tran_amount = 0;
            }
        }
    }
    try{
        W.commit();
    }
    catch(const exception& e){
        cerr << e.what() << endl;
        W.abort();
        return matchBuyOrder(C, tran_id);
    }

}

void matchSellOrder(connection* C, long tran_id){
    work W(*C);
    stringstream query;
    query << "SELECT SYM_NAME, ACT_ID, TRAN_LIMIT_PRICE, TRAN_AMOUNT, TIME FROM TRANSACTIONS "
    << "WHERE TRAN_ID = " << W.quote(tran_id) << ";";
    result res(W.exec(query.str()));
    string sym = res.begin()[0].as<string>();
    string act_id = res.begin()[1].as<string>();
    double limit_price = res.begin()[2].as<double>();
    double tran_amount = res.begin()[3].as<double>();
    long tran_time = res.begin()[4].as<long>();
    while(tran_amount>0){ 
        stringstream matching;
        matching << "SELECT TRAN_ID, ACT_ID, TRAN_LIMIT_PRICE, TRAN_AMOUNT, TIME FROM TRANSACTIONS "
        << "WHERE SYM_NAME = " << W.quote(sym) << " AND TRAN_STATUS = " << W.quote("OPEN") 
        << " AND TRAN_AMOUNT > 0 AND TRAN_LIMIT_PRICE >= " << W.quote(limit_price)
        << "ORDER BY TRAN_LIMIT_PRICE DESC, TIME ASC;";
        result matching_res(W.exec(matching.str()));
        for(auto it = matching_res.begin(); it != matching_res.end()&&tran_amount>0; ++it){
            long buyer_tran_id = it[0].as<long>();
            string buyer_act_id = it[1].as<string>();
            double buyer_price = it[2].as<double>();
            double buyer_amount = it[3].as<double>();
            long buyer_time = it[4].as<long>();
            long cur_time = generateTime();
            if((buyer_amount)<(-tran_amount)){
                // cut buy order
                tran_amount+=buyer_amount;
                double seller_income = buyer_price*buyer_amount;
                double buyer_refund = 0;
                // refund : no need to refund buyer
                updateAccountBalance(W,act_id.c_str(),seller_income);
                //updateAccountBalance(W,act_id.c_str(),buyer_refund);
                updatePositionShares(W,buyer_act_id.c_str(),sym,buyer_amount);
                updateTransaction(W,tran_id,tran_amount);
                createOrder(W, tran_id, buyer_amount, buyer_price, cur_time);
            }
            else{
                // cut sell order
                double seller_income = buyer_price*tran_amount;
                double buyer_refund = 0;
                // refund : no need to refund buyer
                updateAccountBalance(W,act_id.c_str(),seller_income);
                //updateAccountBalance(W,act_id.c_str(),buyer_refund);
                updatePositionShares(W,buyer_act_id.c_str(),sym,tran_amount);
                updateTransaction(W,tran_id,tran_amount);
                createOrder(W, tran_id, tran_amount, buyer_price, cur_time);
                tran_amount = 0;
            }
        }
    }
    try{
        W.commit();
    }
    catch(const exception& e){
        cerr << e.what() << endl;
        W.abort();
        return matchSellOrder(C, tran_id);
    }
}