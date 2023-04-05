#include "server.h"

void Server::run(){
    memset(&host_info, 0, sizeof(this->host_info));
    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags    = AI_PASSIVE;

    status = getaddrinfo(NULL, std::string(port).c_str(), &host_info, &host_info_list);
    if (status != 0) {
        std::cerr << "Error: cannot get address info for host" << std::endl;
        return;
    }
    socket_fd = socket(host_info_list->ai_family, 
		host_info_list->ai_socktype, 
		host_info_list->ai_protocol);
    if (socket_fd == -1) {
        std::cerr << "Error: cannot create socket" << std::endl;
        return;
    }

    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        std::cerr << "Error: cannot bind socket" << std::endl;
    } 

    // backlog = 100
    status = listen(socket_fd, 100);
    if (status == -1) {
        std::cerr << "Error: cannot listen on socket" << std::endl;
    }
}

std::pair<int,char*> Server::accept_connections(){
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int client_connection_fd;
    client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_connection_fd == -1) {
        std::cerr << "Error: cannot accept connection on socket" << std::endl;
        return std::make_pair(-1,nullptr);
    } //if
    char* ip= inet_ntoa(((struct sockaddr_in* )&socket_addr)->sin_addr);

    return std::make_pair(client_connection_fd,ip);
}