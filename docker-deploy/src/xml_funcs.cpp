#include "xml_funcs.h"

void* handler(void* p){
    int client_fd = ((std::pair<int,char*>*)p)->first;
    std::string ip(((std::pair<int,char*>*)p)->second);
    connection *C;

    try{
        C = new connection("dbname=STK_SYS user=postgres password=passw0rd hostaddr=127.0.0.1");
        if (C->is_open()) {
            std::cout << "Opened database successfully: " << C->dbname() << std::endl;
        } else {
            std::cout << "Can't open database" << std::endl;
            return nullptr;
        }
    }catch (const std::exception &e){
        std::cerr << e.what() << std::endl;
        return nullptr;
    }

    // recv client's xml request
    TiXmlDocument* doc = recv_xml(client_fd);
    if(doc==nullptr){
        return nullptr;
    }
    // get root
    TiXmlElement* root = doc->RootElement();
    const char* req_root = root->Value();
    

    // build a xml response
    TiXmlDocument response;
    TiXmlElement* res_root = new TiXmlElement("results");
    response.LinkEndChild(res_root);

    // xml header
    if(req_root=="create"){
        // todo:handle creation
        for(TiXmlElement* child = root->FirstChildElement(); child; child = child->NextSiblingElement()){
            if(child->Value()=="account"){
                const char* id = child->Attribute("id");
                const char* balance = child->Attribute("balance");
                bool flag = createAccount(C,id,balance);
                // if success
                if(flag){
                    // <created id='account_id'/>
                    TiXmlElement* success = new TiXmlElement("created");
                    success->SetAttribute("id", id);
                    res_root->LinkEndChild(success);
                }
                // if fail
                else{
                    // <error id='account_id'>MSG</error>
                    TiXmlElement* error = new TiXmlElement("error");
                    error->SetAttribute("id", id);
                    error->LinkEndChild(new TiXmlText("Error create this account"));
                    res_root->LinkEndChild(error);
                }
            }
            else if(child->Value()=="symbol"){
                // can have 1 or more subchild
                const char* sym = child->Attribute("sym");
                for(TiXmlElement* subchild = child->FirstChildElement(); subchild; subchild = subchild->NextSiblingElement()){
                    const char* id = subchild->Attribute("id");
                    const char* num = subchild->Value();
                    bool flag = createSymbol(C,sym,id,num);
                    // if success
                    if(flag){
                        // <created sym='sym' id='account_id'/>
                        TiXmlElement* success = new TiXmlElement("created");
                        success->SetAttribute("sym",sym);
                        success->SetAttribute("id", id);
                        res_root->LinkEndChild(success);
                    }
                    // if fail
                    else{
                        TiXmlElement* error = new TiXmlElement("error");
                        error->SetAttribute("sym",sym);
                        error->SetAttribute("id", id);
                        error->LinkEndChild(new TiXmlText("Error create this symbol"));
                        res_root->LinkEndChild(error);   
                    }
                    createPosition(C,sym,id,num);
                }
            }
            else{
                //error child name
                TiXmlElement* error = new TiXmlElement("error");
                error->LinkEndChild(new TiXmlText("Unable handle this request"));
                res_root->LinkEndChild(error);
            }
        }

    }
    else if(req_root=="transactions"){
        // todo:handle transactions
        const char* char_id = root->Attribute("id");
        int numofchild = -1;
        std::string id(char_id);
        // invalid id or null id field
        if(id.size()==0||!queryAccount(C,std::atol(char_id))){
            numofchild = 0;
            for(TiXmlElement* child = root->FirstChildElement(); child; child = child->NextSiblingElement()){
                TiXmlElement* error = new TiXmlElement("error");
                if(id.size()==0){
                    error->LinkEndChild(new TiXmlText("Empty id attribute"));
                }
                else{
                    error->LinkEndChild(new TiXmlText("Invalid id"));
                }
                res_root->LinkEndChild(error);
                numofchild++;
            }  
        }
        else{
            numofchild = 0;
            for(TiXmlElement* child = root->FirstChildElement(); child; child = child->NextSiblingElement()){
                if(child->Value()=="order"){
                    // todo: place order
                    const char* char_sym = child->Attribute("sym");
                    const char* char_amount = child->Attribute("amount");
                    const char* char_limit = child->Attribute("limit");
                    std::string sym(char_sym);
                    if(!querySymbol(C,sym)){
                        TiXmlElement* error;
                        error->LinkEndChild(new TiXmlText("Invalid symbol name"));
                        res_root->LinkEndChild(error);
                    }
                    else{
                        TiXmlElement* res = handleOrder(C,char_id,sym,atol(char_amount),atof(char_limit));
                        res->SetAttribute("sym",char_sym);
                        res->SetAttribute("amount",char_amount);
                        res->SetAttribute("limit",char_limit);
                        res_root->LinkEndChild(res);
                    }
                }
                else if(child->Value()=="query"){
                    // todo: place query
                    const char* char_id = child->Attribute("id");
                    TiXmlElement* res = queryTran(C,char_id);
                    if(res==nullptr){
                        res = new TiXmlElement("error");
                        res->LinkEndChild (new TiXmlText("Invalid transaction id"));
                        res_root->LinkEndChild (res);
                    }
                    res->SetAttribute("id",char_id);
                    res_root->LinkEndChild (res);
                }
                else{
                    // todo: place cancel
                    const char* char_id = child->Attribute("id");
                    TiXmlElement* res = cancelTran(C,char_id);
                    if(res==nullptr){
                        res = new TiXmlElement("error");
                        res->LinkEndChild (new TiXmlText("Invalid transaction id"));
                        res_root->LinkEndChild (res);
                    }
                    res->SetAttribute("id",char_id);
                    res_root->LinkEndChild (res);
                }
                numofchild++;
            }
        }
        if(numofchild==0){
            // empty child
            TiXmlElement* error;
            error->LinkEndChild(new TiXmlText("Transaction tag has 0 child"));
            res_root->LinkEndChild(error);
        }
    }
    else{
        // handle error
        TiXmlElement* error = new TiXmlElement("error");
        error->LinkEndChild(new TiXmlText("Unable to handle this request"));
        res_root->LinkEndChild(error);
    }

    // send response back

    if(send(client_fd,&res_root,sizeof(res_root),0)!=sizeof(res_root)){
        std::cerr << "Error sending" << std::endl;
    }
    res_root->Clear();
    doc->Clear();
    C->disconnect();
    close(client_fd);
    delete C;
    return nullptr;
}

TiXmlDocument* recv_xml(int fd){
    char ori_msg[65535];
    memset(ori_msg,0,sizeof(ori_msg));
    int recv_len = recv(fd, &ori_msg, sizeof(ori_msg), 0);
    if(recv_len<=0){
        std::cerr << "Error receiving msg" << std::endl;;
        return nullptr;
    }

    string msg(ori_msg,recv_len);
    size_t pos = msg.find('\n');
    string slen = msg.substr(0,pos);
    int len = stoi(slen);
    msg = msg.substr(pos+1);

    TiXmlDocument* doc = new TiXmlDocument();
    doc->Parse(msg.c_str());
    return doc;
}