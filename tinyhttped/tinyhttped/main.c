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
#include <sys/stat.h>
#include <ctype.h>

void tiny_httpd_exit(char* msg);
int startup(u_short* port);
void accept_request(int client);
int get_line(int, char *, int);
void not_found(int);
void unimplemented(int clent);
void execute_cgi(int, const char *, const char *, const char *);
void serve_file(int, const char *);
void cat(int, FILE *);
void headers(int, const char *);
int main(int argc, const char * argv[])
{
    int server_sock = -1;
    u_short port = 0;
    int client_sock = -1;
    struct sockaddr_in client_name;
    pthread_t newthread;
    
    server_sock = startup(&port);
    printf("httpd is running on port %d\n", port);
    fflush(stdout);

    socklen_t addr_len = sizeof(struct sockaddr_in);
    bzero(&client_name, sizeof(struct sockaddr_in));
    while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_name, &addr_len);
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
    if (-1 == (httpd = socket(PF_INET, SOCK_STREAM, 0))) {
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

void accept_request(int client){
    char buf[1024];
    int numchars;
    char method[255];
    char url[255];
    char path[512];
    size_t i, j;
    struct stat st;
    int cgi = 0;

    char* query_str = NULL;
    numchars = get_line(client, buf, sizeof(buf));
    i = j = 0;
    while (!isspace(buf[j]) && (i < sizeof(method) - 1)){
        method[i] = buf[j];
        i++;
        j++;
    }
    method[i] = '\0';
    if (strcasecmp(method, "GET") && strcasecmp(method, "POST")){
        unimplemented(client);
        return;
    }
    if (strcasecmp(method, "POST") == 0){
        cgi = 1;
    }
    i = 0;
    while (isspace(buf[j]) && (j < sizeof(buf))){
        j++;
    }
    while (!isspace(buf[j]) && (i < sizeof(url) - 1) && (j < sizeof(buf))){
        url[i] = buf[i];
        i++;
        j++;
    }
    url[i] ='\0';
    if (strcasecmp(method, "GET") == 0){
        query_str = url;
        while((*query_str != "?") && (*query_str)){
            query_str++;
        }
        if (*query_str == "?"){
            cgi = 1;
            *query_str = '\0';
            query_str++;
        }
    }

    sprintf(path, "%s", url);
    if (path[strlen(path) - 1] == '/'){
        strcat(path, "index.html");
    }
    if (stat(path, "&st") == -1){
        while((numchars > 0) && strcmp("\n", buf)){
            numchars = get_line(client, buf, sizeof(buf));
        }
        not_found(client);
    }else{
        if ((st.st_mode & S_IFMT) == S_IFDIR){
            strcat(path, "/index.html");
        }
        if ((st.st_mode & S_IXUSR) ||
                (st.st_mode & S_IXGRP) ||
                (st.st_mode & S_IXOTH)){
            cgi = 1;
        }
        if (cgi){
            execute_cgi(client, path, method, query_str);
        } else{
            serve_file(client, path);
        }
    }
    close(client);
}

int get_line(int sock, char * buf, int size){
    int i = 0;
    char c = '\0';
    int n;
    while ((i < size - 1) && (c != '\n')){
        n = recv(sock, &c, 1, 0);
        if (n > 0){
            if (c == '\r'){
                n = recv(sock, &c, 1, MSG_PEEK);
                if ((n > 0) && (c == '\n')){
                    recv(sock, &c, 1, 0);
                }else{
                    c = '\n';
                }

            }
            buf[i] = c;
            i++;
        } else{
            c = '\n';
        }
        buf[i] = '\0';
    }
    return i;
}
void unimplemented(int client){
    char buf[1024];
    sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Server: tinyHttpd\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</TITLE></HEAD>\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</BODY></HTML>\r\n");
    send(client, buf, strlen(buf), 0);
}

void not_found(int client)
{
    char buf[1024];

    sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Server: tinyHttpd\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "your request because the resource specified\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "is unavailable or nonexistent.\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</BODY></HTML>\r\n");
    send(client, buf, strlen(buf), 0);
}
void serve_file(int client, const char* filename){
    FILE* resource = NULL;
    int numchars = 1;
    char buf[1024];

    buf[0] = 'A'; buf[1] = '\0';
    while((numchars > 0) && strcmp("\n", buf)){
        numchars = get_line(client, buf, sizeof(buf));
    }

    resource = fopen(filename, "r");
    if (resource == NULL){
        not_found(client);
    }else{
        headers(client, filename);
        cat(client, resource);
    }
    fclose(resource);
}
void headers(int client, const char *filename)
{
    char buf[1024];
    (void)filename;  /* could use filename to determine file type */

    strcpy(buf, "HTTP/1.0 200 OK\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "Server: tinyHttpd\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
}

void cat(int client, FILE* resource){
    char buf[1024];
    fgets(buf, sizeof(buf), resource);
    while (!feof(resource)){
        send(client, buf, strlen(buf), 0);
        fgets(buf, sizeof(buf), resource);
    }
}

void execute_cgi(int client, const char *path,
        const char *method, const char *query_string){

}