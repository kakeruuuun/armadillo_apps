/*
Twitterの文章をソケットで受け取る
その後、その内容によってIRを操作する
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MYROOM 0
#define LIVING 1
#define TV 0
#define AIRCON 1
#define LIGHT 2
#define ON 0
#define OFF 1
#define PIN_PUSHBUTTON 4
#define PIN_IR 12
#define LOW 0
#define HIGH 1
#define DEBOUNCEDELAY 500
#define OUT 0
#define IN 1

#define BUF_LEN 256


int gpio_set(int s_pin, int mode)
{
    int fd, fd2;
    char buff[256];
    sprintf(buff, "/sys/class/gpio/CON9_%d/direction", s_pin);
    fd = open(buff, O_WRONLY);
    if (fd == -1)
    {
        return EXIT_FAILURE;
    }
    if (mode == IN)
    {
        write(fd, "in", 2);
    }
    else if (mode == OUT)
    {
        write(fd, "out", 3);
    }
    else
    {
        perror("GPIO SET ERROR\n");
        return -1;
    }
    close(fd);
}

int gpio_read(int r_pin)
{
    int fd, ret;
    char buff[256];
    char buff2[2];
    int stat;
    sprintf(buff, "/sys/class/gpio/CON9_%d/value", r_pin);
    fd = open(buff, O_RDONLY);

    if (fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    ret = read(fd, buff2, 2);
    if (ret == -1)
    {
        perror("READ ERROR\n");
        exit(EXIT_FAILURE);
    }
    stat = strtol(buff2, NULL, 0);
    close(fd);
    return stat;
}

int gpio_write(int w_pin, int w_out)
{
    int fd;
    char buff[256];
    sprintf(buff, "/sys/class/gpio/CON9_%d/value", w_pin);
    fd = open(buff, O_WRONLY);
    if (fd == -1)
    {
        return EXIT_FAILURE;
    }
    if (w_out == 0)
    {
        write(fd, "1", 1);
    }
    else if (w_out == 1)
    {
        write(fd, "0", 1);
    }
    else
    {
        perror("WRITE ERROR\n");
        return -1;
    }
    close(fd);
}

void delayMicrosecondsHard(unsigned int howLong)
{
    struct timeval tNow, tLong, tEnd;

    gettimeofday(&tNow, NULL);
    tLong.tv_sec = howLong / 1000000;
    tLong.tv_usec = howLong % 1000000;
    timeradd(&tNow, &tLong, &tEnd);

    while (timercmp(&tNow, &tEnd, <))
        gettimeofday(&tNow, NULL);
}

void delayMicroseconds(unsigned int howLong)
{
    struct timespec sleeper;
    unsigned int uSecs = howLong % 1000000;
    unsigned int wSecs = howLong / 1000000;

    if (howLong == 0)
        return;
    else if (howLong < 100)
        delayMicrosecondsHard(howLong);
    else
    {
        sleeper.tv_sec = wSecs;
        sleeper.tv_nsec = (long)(uSecs * 1000L);
        nanosleep(&sleeper, NULL);
    }
}

void sendModulatedData(int modulationTime)
{
    int i;
    struct timespec time1;
    int fd;
    fd = open("/sys/class/gpio/CON9_11/value", O_WRONLY);
    time1.tv_sec = 0;
    time1.tv_nsec = 500;

    struct timespec time2;
    time2.tv_sec = 0;
    time2.tv_nsec = 1000;

    for (i = 0; i < modulationTime / 30; i++)
    {
        //ONタイム(9us)
        write(fd, "1", 1);
        write(fd, "1", 1);

        //OFFタイム(17us)
        write(fd, "0", 1);
        nanosleep(&time2, NULL);
        nanosleep(&time2, NULL);
        nanosleep(&time2, NULL);
    }
}

void sendData(int onTime, int offTime)
{
    //on
    sendModulatedData(onTime);

    //off
    write(fd, "0", 1);
    usleep(offTime);
}

int ir_send(int rm, int obj, int stat)
{
    int *onTime, *offTime;
    int i, j, length = 0;

    char fileName[100] = "ir_data.txt";
    char buf[BUF_LEN];
    int loop = 3;
    FILE *fp;

    if(rm == MYROOM)
    {
        if(obj == TV){
            if(stat == ON)  strcpy(fileName, "/home/guest/ir_data/ir_myroom_tv_on.txt");
            else            strcpy(fileName, "/home/guest/ir_data/ir_myroom_tv_off.txt");
        }else if(obj == AIRCON){
            if(stat == ON)  strcpy(fileName, "/home/guest/ir_myroom_aircon_on.txt");
            else            strcpy(fileName, "/home/guest/ir_data/ir_myroom_aircon_off.txt");
        }else if(obj == LIGHT){
            if(stat == ON)  strcpy(fileName, "/home/guest/ir_data/ir_myroom_light_on.txt");
            else            strcpy(fileName, "/home/guest/ir_data/ir_myroom_light_off.txt");
        }
    }else if(rm == LIVING){
        if(obj == TV){
            if(stat == ON)  strcpy(fileName, "/home/guest/ir_data/ir_living_tv_on.txt");
            else            strcpy(fileName, "/home/guest/ir_data/ir_living_tv_off.txt");
        }else if(obj == AIRCON){
            if(stat == ON)  strcpy(fileName, "/home/guest/ir_data/ir_living_aircon_on.txt");
            else            strcpy(fileName, "/home/guest/ir_data/ir_living_aircon_off.txt");
        }else if(obj == LIGHT){
            if(stat == ON)  strcpy(fileName, "/home/guest/ir_data/ir_living_light_on.txt");
            else            strcpy(fileName, "/home/guest/ir_data/ir_living_light_off.txt");
        }
    }

    if ((fp = fopen(fileName, "r")) == NULL)
    {
        printf("can't open file : %s\n", fileName);
        return -1;
    }

    gpio_set(11, OUT);

    //solve file Length
    while (fgets(buf, BUF_LEN, fp) != NULL)
    {
        length++;
    }

    onTime = (int *)calloc(length, sizeof(int));
    offTime = (int *)calloc(length, sizeof(int));

    //read data from file
    rewind(fp);
    for (i = 0; i < length; i++)
    {
        fscanf(fp, "%d %d", &onTime[i], &offTime[i]);
    }

    //send data
    for (j = 0; j < 5; j++)
    {
        for (i = 0; i < length; i++)
        {
            sendData(onTime[i], offTime[i]);
        }
    }

    fclose(fp);
    free(onTime);
    free(offTime);
    return 0;
}


int main(void)
{
    FILE *write_fp;
    char buf[100];
    int num_read;
    FILE *fp;
    int fd;

    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY; //inet_addr("127.0.0.1");
    server_address.sin_port = 9734;

    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

    listen(server_sockfd, 5);
    while (1)
    {
        int room, obj, status;
        client_sockfd = accept(server_sockfd,
                               (struct sockaddr *)&client_address, &client_len);

        if (!(write_fp = fopen("./socketDM.txt", "wb")))
        {
            printf("error1\n");
            return;
        }
        while (num_read = read(client_sockfd, buf, sizeof(buf)))
            ;

        if (fwrite(buf, 16, 1, write_fp) < num_read) //バイト数を指定している
        {
            printf("error2\n");
            return;
        }
        fclose(write_fp);
        close(client_sockfd);

        if(!(fp = fopen("./socketDM.txt", "r"))) continue;
        fread(buf, 1, 16, fp);

        if(strstr(buf, "myroom") != NULL)       room = MYROOM;
        else if(strstr(buf, "living") != NULL)  room = LIVING;

        if(strstr(buf, "tv____") != NULL)       obj = TV;
        else if(strstr(buf, "aircon") != NULL)  obj = AIRCON;
        else if(strstr(buf, "light_") != NULL)  obj = LIGHT;

        if(strstr(buf, "on_") != NULL)       status = ON;
        else if(strstr(buf, "off") != NULL)  status = OFF;

        fclose(fp);
        ir_send(room, obj, status);
    }
}
