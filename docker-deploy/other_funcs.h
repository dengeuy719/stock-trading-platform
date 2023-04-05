#include<ctime>
#include<sstream>
#include<vector>
#include<tinyxml.h>
#include<pqxx/pqxx> 
#include<iostream>

using namespace std;
using namespace pqxx;

long generateTime();
void matchBuyOrder(connection* C, long tran_id);
void matchSellOrder(connection* C, long tran_id);