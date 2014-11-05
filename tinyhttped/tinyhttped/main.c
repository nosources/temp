//
//  main.c
//  tinyhttped
//
//  Created by nosources on 14-10-23.
//  Copyright (c) 2014年 nosources. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
void tiny_httpd_exit(char* msg);
int startup(u_short* port);
void* accept_request(int client);
int get_line(int, char *, int);
int main(int argc, const char * argv[])
{
    int server_sock = -1;
    u_short port = 0;
    int client_sock = -1;
    struct sockaddr_in client_name;
    int client_name_len = sizeof(client_name);
    pthread_t newthread;
    
    server_sock = startup(&port);
    printf("httpd is running on port %d\n.", port);
    
    while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_sock, &client_name_len);
        if (-1 == client_sock) {
            tiny_httpd_exit("accept error");
        }
        if (0 != pthread_create(&newthread, NULL, accept_request, client_sock)) {
            tiny_httpd_exit("thread create error");
        }
        close(server_sock);
    }
    return 0;
}

void tiny_httpd_exit(char* msg){
    perror(msg);
    perror("\n");
    exit(1);
}
int startup(u_short* port){
    int httpd = 0;
    struct sockaddr_in name;
    //建立一个socket通信
    //
    int domain = PF_INET;
    int type = SOCK_STREAM;
    int protocol = 0;
    if (-1 == (httpd = socket(domain, type, protocol))) {
        tiny_httpd_exit("socket error");
    }
    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_port = htons(*port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    if (-1 == bind(httpd, (struct sockaddr*)&name, sizeof(name))){
        tiny_httpd_exit("bind error");
    }
    if (*port == 0) {
        int name_len = sizeof(name);
        if (-1 == getsockname(httpd, (struct sockaddr*)&name, &name_len)) {
            tiny_httpd_exit("get sock name error");
        }
        *port = ntohs(name.sin_port);
    }
    
    if (listen(httpd, 5) < 0) {
        tiny_httpd_exit("listen error");
    }
    return httpd;
}

void* accept_request(int client){
    char buf[1024];
    int numchars;
    char method[255];
    char url[255];
    char path[512];
    size_t i, j;
    struct stat st;
    int cgi = 0;
    
}

int get_line(int sock, char * buf, int size){
    
}