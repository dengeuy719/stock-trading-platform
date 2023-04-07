
#include<pqxx/pqxx>
#include<vector>
#include<string>
#include<iostream>
#include<pthread.h>

#include "database.h"
#include "server.h"
#include "xml_funcs.h"

using namespace std;
using namespace pqxx;

int main(){
    connection* C;
    
    try{
        C = new connection("dbname=STK_SYS user=postgres password=passw0rd hostaddr=127.0.0.1");
        if (C->is_open()) {
            cout << "Opened database successfully: " << C->dbname() << endl;
        } else {
            cout << "Can't open database" << endl;
            return 1;
        }
    }catch (const std::exception &e){
        cerr << e.what() << std::endl;
        return 1;
    }

    // setup isolation level : read commited - repeatable read - serializable
    work w(*C);
    stringstream ss;
    ss << "set transaction isolation level SERIALIZABLE";
    w.exec(ss.str());
    w.commit();

    // drop tables if exist
    vector<string> drop_querys;
    drop_querys.emplace_back("DROP TABLE IF EXISTS SYMBOL CASCADE;");
    drop_querys.emplace_back("DROP TABLE IF EXISTS POSITION CASCADE;");
    drop_querys.emplace_back("DROP TABLE IF EXISTS ACCOUNT CASCADE;");
    drop_querys.emplace_back("DROP TABLE IF EXISTS TRANSACTIONS CASCADE;");
    drop_querys.emplace_back("DROP TABLE IF EXISTS ORDERS CASCADE;");
    drop_querys.emplace_back("DROP TABLE IF EXISTS CANCEL CASCADE;");
    for(string& it:drop_querys){
        work w(*C);
        w.exec(it);
        w.commit();
    }

    // create tables
    vector<string> create_querys;
    create_querys.emplace_back("CREATE TABLE SYMBOL (SYM_NAME VARCHAR(256) PRIMARY KEY);");
    create_querys.emplace_back("CREATE TABLE ACCOUNT (ACT_ID BIGINT PRIMARY KEY, ACT_BALANCE FLOAT);");

    stringstream position;
    position <<  "CREATE TABLE POSITION ( SYM_NAME VARCHAR(256), ACT_ID BIGINT, POS_AMOUNT FLOAT, "
            << "CONSTRAINT POSITION_PK PRIMARY KEY (SYM_NAME, ACT_ID), "
            << "FOREIGN KEY (SYM_NAME) REFERENCES SYMBOL(SYM_NAME) ON DELETE SET NULL ON UPDATE CASCADE,"
            << "FOREIGN KEY (ACT_ID) REFERENCES ACCOUNT(ACT_ID) ON DELETE SET NULL ON UPDATE CASCADE);";
    create_querys.emplace_back();
    create_querys.back() = move(position.str());

    stringstream transactions;
    transactions <<  "CREATE TABLE TRANSACTIONS ( TRAN_ID BIGSERIAL PRIMARY KEY, TRAN_STATUS VARCHAR(20), "
            << "SYM_NAME VARCHAR(256), ACT_ID BIGINT, TRAN_AMOUNT FLOAT, TRAN_LIMIT_PRICE FLOAT, TRAN_TIME BIGINT, "
            << "FOREIGN KEY (SYM_NAME) REFERENCES SYMBOL(SYM_NAME) ON DELETE SET NULL ON UPDATE CASCADE,"
            << "FOREIGN KEY (ACT_ID) REFERENCES ACCOUNT(ACT_ID) ON DELETE SET NULL ON UPDATE CASCADE);";
    create_querys.emplace_back();
    create_querys.back() = move(transactions.str());

    stringstream orders;
    orders <<  "CREATE TABLE ORDERS ( ODR_ID BIGSERIAL PRIMARY KEY, "
            << "TRAN_ID BIGINT, ODR_AMOUNT FLOAT, ODR_PRICE FLOAT, ODR_TIME BIGINT, "
            << "FOREIGN KEY (TRAN_ID) REFERENCES TRANSACTIONS(TRAN_ID) ON DELETE SET NULL ON UPDATE CASCADE);";
    create_querys.emplace_back();
    create_querys.back() = move(orders.str());

    stringstream cancel;
    cancel << "CREATE TABLE CANCEL ( CAN_ID BIGSERIAL PRIMARY KEY, TRAN_ID BIGINT, CAN_TIME BIGINT, "
    << "FOREIGN KEY (TRAN_ID) REFERENCES TRANSACTIONS(TRAN_ID) ON DELETE SET NULL ON UPDATE CASCADE);";
    create_querys.emplace_back();
    create_querys.back() = move(cancel.str());

    for(string& it:create_querys){
        work w(*C);
        w.exec(it);
        w.commit();
    }
    
    //Close database connection
    try{
        C->disconnect();
        if(C->is_open()){
            throw std::exception();
        }
        else{
            cout << "Closed database successfully: " << C->dbname() << endl;
        }
    }catch (const std::exception &e){
        cerr << e.what() << std::endl;
        return 1;
    }
    delete C;

    // start server
    Server stoke_server;
    stoke_server.run();

    //cout << "server started" << endl;
    // listen to user's connection and allocate threads
    pthread_mutex_t mutex;
    while(true){
        pthread_mutex_lock(&mutex);
        pair<int,char*> client = stoke_server.accept_connections();
        int client_fd = client.first;
        string ip(client.second);
        //cout << "Connected with the client from "<< client_fd << " " << ip << endl;
        pthread_t thread;
        pthread_create(&thread,NULL,handler,&client);
        pthread_mutex_unlock(&mutex);
    }
    //handler(nullptr);

}