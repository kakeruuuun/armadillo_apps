/*





*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <dirent.h>



int edge_capture(unsigned int number);

int main(void)
{
        system("./capture image/image_temp.yuv");
        int file_number = 1;
        while (1)
        {
                edge_capture(file_number);
                if(file_number > 20)      file_number = 1;
                else                      file_number++;
        }
        return 0;
}

int edge_capture(unsigned int number)
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

        //カメラで撮影する
        system("./capture image/image_temp.yuv");
        char buf[1024];
        sprintf(buf, "./jpg_conv image/image_temp.yuv image/image%d.jpg", number);
        system(buf);
        sprintf(buf, "./jpeg_fbdisp image/image%d.jpg", number);
        system(buf);

}
