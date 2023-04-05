#include "client.h"

using namespace std;

void Client::build_client(){
    memset(&this->host_info, 0, sizeof(this->host_info));
    this->host_info.ai_family   = AF_UNSPEC;
    this->host_info.ai_socktype = SOCK_STREAM;
    //status: zero on success, or a non-zero error code on failure
    this->status = getaddrinfo(this->hostname, this->port, &this->host_info, &this->host_info_list);
    if (this->status != 0) {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << this->hostname << "," << this->port << ")" << endl;
        exit(EXIT_FAILURE);
    } //if
    //socket() three arguments：what kind of socket you want (IPv4 or IPv6, stream or datagram, and TCP or UDP)
    //socket_fd: a unique identifier that is used by the operating system to represent an open socket
    this->socket_fd = socket(this->host_info_list->ai_family, 
                this->host_info_list->ai_socktype, 
                this->host_info_list->ai_protocol);
    if (this->socket_fd == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << this->hostname << "," << this->port << ")" << endl;
        exit(EXIT_FAILURE);
    } //if
    //connect() arguments: info of the destination that connecet to
    this->status = connect(this->socket_fd, this->host_info_list->ai_addr, this->host_info_list->ai_addrlen);
    if (this->status == -1) {
        cerr << "Error: cannot connect to socket" << endl;
        cerr << "  (" << this->hostname << "," << this->port << ")" << endl;
        exit(EXIT_FAILURE);
    } //if
    //two steps to end
    // freeaddrinfo(host_info_list);
    // return socket_fd;
}