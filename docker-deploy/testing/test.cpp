#include "client.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>





void sendStr(int fd, std::string msg){
    int status = send(fd, msg.data(), msg.size(),0);
    std::cerr << msg.size() << std::endl;
    if(status == -1){
        std::cerr << "Error sending" << std::endl;
    }
}

std::string reciveStr(int fd){
    char ori_msg[65535];
    memset(ori_msg, 0, sizeof(ori_msg));
    int recv_len = recv(fd, &ori_msg, sizeof(ori_msg), 0);
    std::cout << recv_len << std::endl;
    if(recv_len == -1){
        std::cerr << "Error reciving" << std::endl;
    }
    std::string ans(ori_msg, recv_len);
    return ans;
}

std::string readXMLFileToString(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return buffer.str();
}


int main() {
    Client client("127.0.0.1", PORT);
    std::cout<<"Connected!"<<std::endl;
    const std::string filename = "input.xml";
    std::string xmlContent = readXMLFileToString(filename);
    if (xmlContent.empty()) {
        std::cerr << "Failed to read XML file" << std::endl;
    }
    std::cout <<  xmlContent << std::endl;
    sendStr(client.socket_fd, xmlContent);
    std::cout << "finish send" << std::endl;
    std::string res = reciveStr(client.socket_fd);
    std::cout<<res<<std::endl;
    std::cout << "finish recive" << std::endl;
}