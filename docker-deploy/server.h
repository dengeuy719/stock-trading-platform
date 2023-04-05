#pragma once

#include <iostream>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <tinyxml.h>

class Server{
public:
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    const char *hostname;
    const char *port;
    Server():status(0),socket_fd(0),host_info_list(NULL),hostname(NULL),port("12345"){}

    void run();
    std::pair<int,char*> accept_connection();

    ~Server(){
        close(socket_fd);
        free(host_info_list);
    }

};
