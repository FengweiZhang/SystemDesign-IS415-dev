#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
 
/* "BSIZE" is the size of the buffer we use to read from the socket. */
 
#define BSIZE 0x1000
 
/* Get the web page and print it to standard output. */
 
static void get_page (int s, const char * host, const char * page)
{
    char * msg;
 
    /* "format" is the format of the HTTP request we send to the web
       server. */
 
    const char * format =
        "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: fetch.c\r\n\r\n";
    asprintf (& msg, format, page, host);
    send (s, msg, strlen (msg), 0);
    while (1) {
        int bytes;
        char buf[BSIZE+10];
        bytes = recvfrom (s, buf, BSIZE, 0, 0, 0);
        if (bytes == -1) {
            fprintf (stderr, "%s\n", strerror(errno));
            exit (1);
        }
        buf[bytes] = '\0';
        printf ("%s", buf);
        if (bytes == 0) {
            break;
        }
    }
    free (msg);
}
 
int main ()
{
    struct addrinfo hints, *res, *res0;
    int error;
    /* "s" is the file descriptor of the socket. */
    //socket的文件描述符
    int s;
    /* Get one of the web pages here. */
    //www.lemoda.net
    const char * host = "110.242.68.3";
 
    memset (&hints, 0, sizeof(hints));
 
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;//套接字流方式
    //获取主机地址信息
    error = getaddrinfo (host, "http", & hints, & res0);
    if (error) {
        fprintf (stderr, "%s\n", gai_strerror(error));
        exit (1);
    }
    s = -1;
    //遍历res0 addrinfo结构体
    for (res = res0; res; res = res->ai_next) {
    	//建立socket
        s = socket (res->ai_family, res->ai_socktype, res->ai_protocol);
        if (s < 0) {
            fprintf (stderr, "socket: %s\n", strerror (errno));
            exit (1);
        }
        //链接socket
        if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
            fprintf (stderr, "connect: %s\n", strerror (errno));
            close(s);
            exit (1);
        }
        break;
    }
    if (s != -1) {
        get_page (s, host, "");
    }
    freeaddrinfo (res0);
    return 0;
}