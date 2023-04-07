#include "client.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>


void sendStr(int fd, std::string msg){
    //std::cout << msg.size() << std::endl;
    int status = send(fd, msg.data(), msg.size(),0);
    //std::cout << "status: "<<status << std::endl;
    if(status == -1){
        std::cerr << "Error sending" << std::endl;
    }
}

std::string reciveStr(int fd){
    char ori_msg[65535];
    memset(ori_msg, 0, sizeof(ori_msg));
    int recv_len = recv(fd, &ori_msg, sizeof(ori_msg), 0);
    //std::cout << recv_len << std::endl;
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

void handler(void* argv) {
    Client client("127.0.0.1", PORT);
    //std::cout<<"Connected!"<<std::endl;
    const std::string filename = (char*)argv;
    std::string xmlContent = readXMLFileToString(filename);
    if (xmlContent.empty()) {
        std::cerr << "Failed to read XML file" << std::endl;
    }
    //std::cout <<  xmlContent << std::endl;
    sendStr(client.socket_fd, xmlContent);
    //std::cout << "finish send" << std::endl;
    std::string res = reciveStr(client.socket_fd);
    //std::cout<<res<<std::endl;
    //std::cout << "finish recive" << std::endl;
    //return nullptr;
}

int main(int argc, char **argv) {

    // Record the start time
    struct timeval start, end;
    gettimeofday(&start, NULL);
    int num = atoi(argv[2]);
    while(num) {
        // pthread_t thread;
        // pthread_create(&thread,NULL,handler,argv[1]);
        handler(argv[1]);
        std::cout << num << std::endl;
        --num;
    }
    //std::cout << num << std::endl;
    // Record the end time
    gettimeofday(&end, NULL);

    // Calculate the elapsed time
    long elapsed_time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
    // Print the execution time in microseconds
    // printf("Execution time: %ld microseconds\n", elapsed_time);
    std::cout << elapsed_time << std::endl;
    return 0;
}