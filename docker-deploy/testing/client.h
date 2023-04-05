#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <iostream>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <sstream>


#define PORT "12345"


class Client{
public:
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    const char *hostname;
    const char *port;
    Client(const char *hostname, const char *port):
    status(0),socket_fd(0),host_info_list(NULL),hostname(hostname),port(port){
        build_client();
    }
    ~Client(){
        close(socket_fd);
        free(host_info_list);
    }
    void build_client();
};

#endif