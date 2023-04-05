#include<vector>
#include<tinyxml.h>
#include<pqxx/pqxx> 
#include<iostream>
#include"other_funcs.h"
using namespace std;
using namespace pqxx;
bool createAccount(connection* C, const char* char_id, const char* char_balance);
bool querySymbol(connection* C, string& sym_name);
bool queryAccount(connection* C, long account_id);
bool createSymbol(connection* C, const char* char_sym, const char* char_id, const char* char_num);
void createPosition(connection* C, const char* char_sym, const char* char_id, const char* char_num);

TiXmlElement* handleOrder(connection *C, const char* char_id, string& sym, long amount, double limit);
double queryBalance(work& W, const char* char_id);
void updateAccountBalance(work& W, const char* char_id, double balance);
double queryShares(work& W, const char* char_id, string& sym);
void updatePositionShares(work& W, const char* char_id, string& sym, double shares);
long createTran(work& W, const char* char_id, string& sym, double amount, double limit);
void updateTransaction(work& W, long tran_id, double amount);
void createOrder(work& W, long tran_id, double order_amount, double order_price, long order_time);

TiXmlElement* queryTran(connection* C, const char* char_tran_id);
TiXmlElement* cancelTran(connection* C, const char* char_tran_id);

