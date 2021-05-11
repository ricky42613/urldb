#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <event2/http_compat.h>
#include <event2/util.h>
#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     //for getopt, fork
#include <string.h>     //for strcat
#include <sys/queue.h>
#include <event.h> 


#define MYHTTPD_SIGNATURE   "myhttpd v 0.0.1"
url_db *db;
void insert_list(url_db *db,char *list){
    char *ptr = list;
    char buffer[BUFFER_SIZE];
    char ip_buffer[BUFFER_SIZE];
    memset(buffer,0,sizeof(char)*BUFFER_SIZE);
    memset(ip_buffer,0,sizeof(char)*BUFFER_SIZE);
    char *qtr = buffer;
    int url_len = 0,is_url = 1,over_len = 0;
    while (*ptr!='\0') {
        if(*ptr!='\n'){
            if(*ptr == ' '){
                qtr = ip_buffer;
                ptr++;
                is_url = 0;
            }
            else{
                if(url_len<BUFFER_SIZE-1){
                    *qtr++ = *ptr++;
                    if(is_url){
                        url_len++;
                    }
                }else{
                    ptr++;
                    over_len = 1;
                }
            }
        }
        else{
            if(strlen(buffer) && strlen(ip_buffer) && !over_len){
                printf("%s %s\n",buffer,ip_buffer);
                unsigned int hv = insert_url(db, buffer, ip_buffer, db->file_prefix);
                printf("%d:%d\n",hv,db->file_lines[hv]);
            }
            over_len = 0;
            is_url = 1;
            url_len = 0;
            qtr = buffer;
            memset(buffer,0,sizeof(char)*BUFFER_SIZE);
            memset(ip_buffer,0,sizeof(char)*BUFFER_SIZE);
            ptr++;
        }
    } 
}

void pop_urls(url_db *db, char *result,unsigned int rst_len){
    memset(result,0,sizeof(char)*rst_len);
    char buf[BUFFER_SIZE*2+1];
    for(int i = 0;i < MAX_HASH; i++){
        memset(buf,0,sizeof(char)*BUFFER_SIZE*2+1);
        get_from_db(db,buf,BUFFER_SIZE*2+1,i);
        strcat(result,buf);
    }
}

void insert_handler(struct evhttp_request *req, void *argv) {
    url_db *db = (url_db*)argv;
    struct evbuffer* buf = NULL;
    size_t len = 0;
    char* data = NULL;
    // get the event buffer containing POST body
    buf = evhttp_request_get_input_buffer(req);
    // get the length of POST body
    len = evbuffer_get_length(buf);
    // create a char array to extract POST body
    data = malloc(len + 1);
    memset(data,'\0',sizeof(char)*(len + 1));
    // copy POST body into your char array
    evbuffer_copyout(buf, data, len);
    insert_list(db,data);
    free(data);
    //HTTP header
    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Connection", "close");
    //输出的内容
    struct evbuffer *rsp_buf = evbuffer_new();
    evbuffer_add_printf(rsp_buf, "finish insert");
    evhttp_send_reply(req, HTTP_OK, "OK", rsp_buf);
    evbuffer_free(rsp_buf);
}

void pop_handler(struct evhttp_request *req, void *argv){
    url_db *db = (url_db*)argv;
    unsigned int buffer_len = BUFFER_SIZE*2+1;
    char *buffer = (char*)malloc(sizeof(char)*buffer_len*MAX_HASH);
    printf("start pop\n");
    pop_urls(db,buffer,buffer_len*MAX_HASH);
    //HTTP header
    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Connection", "close");
    //输出的内容
    struct evbuffer *rsp_buf = evbuffer_new();
    evbuffer_add_printf(rsp_buf, "%s",buffer);
    free(buffer);
    evhttp_send_reply(req, HTTP_OK, "OK", rsp_buf);
    evbuffer_free(rsp_buf);
}

void httpd_handler(struct evhttp_request *req, void *arg) {
    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Connection", "close");
    //输出的内容
    struct evbuffer *buf;
    buf = evbuffer_new();
    evbuffer_add_printf(buf, "It works!\n");
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}

void signal_handler(int sig) {
    switch (sig) {
        case SIGTERM:
        case SIGHUP:
        case SIGQUIT:
        case SIGINT:
            event_loopbreak();  //终止侦听event_dispatch()的事件侦听循环，执行之后的代码
            FILE *fp = fopen("url.db","w");
            if(fp){
                printf("start bkup\n");
                fwrite(db,sizeof(url_db),1,fp);
                fclose(fp);
            }
            break;
    }
}

void show_help() {
    char *help = "http://localhost:8080\n"
        "-l <ip_addr> interface to listen on, default is 0.0.0.0\n"
        "-p <num>     port number to listen on, default is 1984\n"
        "-d           run as a deamon\n"
        "-t <second>  timeout for a http request, default is 120 seconds\n"
        "-h           print this help and exit\n"
        "\n";
    fprintf(stderr,"%s",help);
}


int main(int argc,char **argv){
    FILE *fp = fopen ("url.db", "r");
    if (fp == NULL)
    {
        db = (url_db*)malloc(sizeof(url_db));
        init_db(db); 
        printf("initialize url db\n");
    }
    else{
        db = (url_db*)malloc(sizeof(url_db));
        fread(db, sizeof(url_db), 1, fp);
        printf("load url db\n");
    }
    printf("start server\n");
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);

    //默认参数
    char *httpd_option_listen = "0.0.0.0";
    int httpd_option_port = 8088;
    int httpd_option_daemon = 0;
    int httpd_option_timeout = 120; //in seconds
    //获取参数
    int c;
    while ((c = getopt(argc, argv, "l:p:dt:h")) != -1) {
        switch (c) {
            case 'l' :
                httpd_option_listen = optarg;
                break;
            case 'p' :
                httpd_option_port = atoi(optarg);
                break;
            case 'd' :
                httpd_option_daemon = 1;
                break;
            case 't' :
                httpd_option_timeout = atoi(optarg);
                break;
            case 'h' :
            default :
                show_help();
                exit(EXIT_SUCCESS);
        }
    }

    //判断是否设置了-d，以daemon运行
    if (httpd_option_daemon) {
        pid_t pid;
        pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        if (pid > 0) {
            //生成子进程成功，退出父进程
            exit(EXIT_SUCCESS);
        }
    }
    /* 使用libevent创建HTTP Server */
    printf("server is started\n");
    //初始化event API
    event_init();
   
    //创建一个http server
    struct evhttp *httpd;
    httpd = evhttp_start(httpd_option_listen, httpd_option_port);

    //指定generic callback
    evhttp_set_gencb(httpd, httpd_handler, NULL);
    //也可以为特定的URI指定callback
    evhttp_set_cb(httpd, "/pop_url",pop_handler, db);
    evhttp_set_cb(httpd, "/insert_url",insert_handler, db);
    //循环处理events
    event_dispatch();

    evhttp_free(httpd);
}