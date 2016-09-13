//
// Created by pinetree on 16-9-10.
//

#ifndef SC_SESSION_H
#define SC_SESSION_H

#include "client.h"

int session_init(int sid_max);

struct client_info *get_clientinfo_by_sid(char *sid);

//struct client_info *GetClientInfoBySockfd(int fd);
//
//int AddNewClient(struct client_info* client_info, int sockfd);
//
//int DelClientBySid(char* sid);
//
//int DelClientBySockfd(int fd);


#endif
