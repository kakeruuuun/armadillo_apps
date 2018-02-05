/*
ポート番号12345
人感センサに反応したときにカメラ画像を取得し
ソケットで画像を転送する。
./capture ./jpg_conv を必要とする。
*/


#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <poll.h>
#include <fcntl.h>
#include <dirent.h>
#include <netdb.h>
#include <string.h>

int edge_capture(void);

int main(void)
{
    while (1)
    {
        edge_capture();
        send_image();
    }
    return 0;
}

int edge_capture(void)
{
    int fd;
    int i;

    fd = open("/sys/class/gpio/CON9_1/direction", O_RDWR);
    write(fd, "in", 2);
    close(fd);

    fd = open("/sys/class/gpio/CON9_1/edge", O_RDWR);
    write(fd, "rising", 7);
    close(fd);
    char val;
    struct pollfd pfd;

    fd = open("/sys/class/gpio/CON9_1/value", O_RDWR);
    read(fd, &val, 1);
    printf("waiting for interrupt...");
    fflush(stdout);
    pfd.fd = fd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    poll(&pfd, 1, -1);
    lseek(fd, 0, SEEK_SET);
    read(fd, &val, 1);
    close(fd);
    printf("run interrupt\n");
    //カメラで撮影する
    system("capture image_temp.yuv");
    char buf[1024];
    sprintf(buf, "jpg_conv image_temp.yuv image_temp.jpg");
    system(buf);
}

int send_image(void)
{
    char *service = "12345";
    struct addrinfo hints, *res0, *res;
    int err;
    int sock;
    int fd;
    char buf[65536];
    int n, ret;

    fd = open("./image_temp.jpg", O_RDONLY);

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = PF_UNSPEC;
    if ((err = getaddrinfo("10.1.69.76", service, &hints, &res0)) != 0)
    {
        printf("error %d : %s\n", err, gai_strerror(err));
        return 1;
    }

    for (res = res0; res != NULL; res = res->ai_next)
    {
        sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock < 0)
        {
            continue;
        }

        if (connect(sock, res->ai_addr, res->ai_addrlen) != 0)
        {
            close(sock);
            continue;
        }

        break;
    }

    freeaddrinfo(res0);
    if (res == NULL)
    {
        /* 有効な接続ができなかった */
        printf("failed\n");
        return 1;
    }

    while ((n = read(fd, buf, sizeof(buf))) > 0)
    {
        ret = write(sock, buf, n);
        if (ret < 1)
        {
            perror("write");
            break;
        }
    }

    close(sock);

    return 0;
}
