#pragma once
#include<tinyxml.h>
#include<pqxx/pqxx> 
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "database.h"

void* handler(void* p);
TiXmlDocument* recv_xml(int client_fd);