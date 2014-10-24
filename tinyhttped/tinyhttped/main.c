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
int startup(u_short* port);
void tiny_httpd_exit(char* msg);
int main(int argc, const char * argv[])
{
    int server_sock = -1;
    u_short port = 0;
    int client_sock = -1;
    struct sockaddr_in client_name;
    int client_name_len = sizeof(client_name);
    pthread_t newthread;
    
    server_sock = startup(&port);
//    printf("httpd running on port %s.\n", "\xe5\x85\xb5\xe7\xa7\x8d\xe9\x80\x82\xe6\x80\xa7");
    return 0;
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
    
    return 0;
}

void tiny_httpd_exit(char* msg){
    perror(msg);
    exit(1);
}