//
// Created by pinetree on 16-9-10.
//

#ifndef SC_SESSION_H
#define SC_SESSION_H

#define CLIENT_BUF_MAX 1024
#define CLIENT_SID_MAX 32

typedef struct client_info {
    char sid[CLIENT_SID_MAX];
    int transport;
    char read_buf[CLIENT_BUF_MAX];
    char write_buf[CLIENT_BUF_MAX];
} clientInfo;

int session_manager_init(int sid_max);

struct client_info *get_clientinfo_by_sid(char *sid);

//struct client_info *GetClientInfoBySockfd(int fd);
//
//int AddNewClient(struct client_info* client_info, int sockfd);
//
//int DelClientBySid(char* sid);
//
//int DelClientBySockfd(int fd);


#endif
