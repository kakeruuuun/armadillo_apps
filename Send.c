/*
  SendInfraredData.c
*/

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

#define PIN_PUSHBUTTON 4
#define PIN_IR 12
#define LOW 0
#define HIGH 1
#define DEBOUNCEDELAY 500
#define OUT 0
#define IN 1

#define BUF_LEN 256

int fd;
int ret;

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

int main(int argc, char *argv[])
{
    int *onTime, *offTime;
    int i, j, length = 0;

    char *fileName = "ir_data.txt";
    char buf[BUF_LEN];
    int loop = 3;
    FILE *fp;


    if (argc == 2)
    {
        fileName = argv[1];
    }
    else if (argc == 3)
    {
        fileName = argv[1];
        loop = atoi(argv[2]);
    }
    if ((fp = fopen(fileName, "r")) == NULL)
    {
        printf("can't open file : %s\n", fileName);
        exit(1);
    }

    gpio_set(11, OUT);
    fd = open("/sys/class/gpio/CON9_11/value", O_WRONLY);


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
    for (j = 0; j < loop; j++)
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
