/*
ポート番号12345
画像をソケットで取得し、日付ごとのディレクトリに保存していく。
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>


int main(void)
{
    while (1)
    {
        recv_image();
    }
    return 0;
}

int recv_image(void)
{
    time_t now = time(NULL);
    struct tm *pnow = localtime(&now);
    char time_buf[128];
    char image_direct[256];
    char command_buf[128];
    char dir_buf[256];
    int num_read;
    int sock0;
    struct sockaddr_in client;
    socklen_t len;
    int sock;
    struct addrinfo hints, *res;
    int err;
    int fd;
    int n, ret;
    char buf[65536];

    

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    err = getaddrinfo(NULL, "12345", &hints, &res);
    if (err != 0)
    {
        printf("getaddrinfo : %s\n", gai_strerror(err));
        return 1;
    }

    /* ソケットの作成 */
    sock0 = socket(res->ai_family, res->ai_socktype, 0);
    if (sock0 < 0)
    {
        perror("socket");
        return 1;
    }

    if (bind(sock0, res->ai_addr, res->ai_addrlen) != 0)
    {
        perror("bind");
        return 1;
    }

    freeaddrinfo(res); /* addrinfo構造体を解放 */

    /* TCPクライアントからの接続要求を待てる状態にする */
    listen(sock0, 5);

    /* TCPクライアントからの接続要求を受け付ける */
    len = sizeof(client);
    sock = accept(sock0, (struct sockaddr *)&client, &len);
    if (sock < 0)
    {
        perror("accept");
        return 1;
    }

    sprintf(time_buf, "%04d%02d%02d%02d%02d",
            pnow->tm_year + 1900,
            pnow->tm_mon + 1,
            pnow->tm_mday,
            pnow->tm_hour,
            pnow->tm_min);
    sprintf(image_direct, "./image/%04d年%02d月%02d日",
            pnow->tm_year + 1900,
            pnow->tm_mon + 1,
            pnow->tm_mday);
            
    mkdir ("./image", 0755);
    sprintf(command_buf, "sudo mkdir %s", image_direct);
    mkdir(command_buf, 0755);
    sprintf(dir_buf, "%s/%s.jpg", image_direct, time_buf);
    fd = open(dir_buf, O_WRONLY | O_CREAT, 0600);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }
    while ((n = read(sock, buf, sizeof(buf))) > 0)
    {
        ret = write(fd, buf, n);
        if (ret < 1)
        {
            perror("write");
            break;
        }
    }

    /* TCPセッションの終了 */
    close(sock);

    /* listen するsocketの終了 */
    close(sock0);

    return 0;
}