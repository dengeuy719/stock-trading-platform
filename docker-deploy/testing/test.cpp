#include "client.h"
#include <sstream>



void sendStr(int fd, std::string msg){
    int status = send(fd, msg.data(), sizeof(msg),0);
    if(status == -1){
        std::cerr << "Error sending" << std::endl;
    }
}

std::string reciveStr(int fd){
    char ori_msg[65535];
    memset(ori_msg, 0, sizeof(ori_msg));
    int recv_len = recv(fd, &ori_msg, sizeof(ori_msg), 0);
    if(recv_len == -1){
        std::cerr << "Error reciving" << std::endl;
    }
    std::string ans(ori_msg, recv_len);
    return ans;
}

int main() {
    Client client("127.0.0.1", PORT);
    std::cout<<"Connected!"<<std::endl;
    std::stringstream ss;
    ss<<"<?xml version=\"1.0\" encoding=\"UTF-8\"?><create><account id=\""<<123<<"\" balance=\""<<100000000<<"\"/><symbol sym=\"BTC\"><account id=\""<<123<<"\">300</account></symbol></create>";
    sendStr(client.socket_fd,ss.str());
    std::string res = reciveStr(client.socket_fd);
    std::cout<<res<<std::endl;
}