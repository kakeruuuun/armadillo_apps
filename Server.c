#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
int main(void)
{
        FILE *write_fp;
        char buf[100];
        int num_read;

        int server_sockfd, client_sockfd;
        int server_len, client_len;
        struct sockaddr_in server_address;
        struct sockaddr_in client_address;

        server_sockfd = socket(AF_INET,SOCK_STREAM,0);
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = INADDR_ANY; //inet_addr("127.0.0.1");
        server_address.sin_port = 9734;

        server_len = sizeof(server_address);
        bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

        listen(server_sockfd,5);
        while(1){

                printf("server_waiting\n");     //for test

                client_sockfd = accept(server_sockfd,
                         (struct sockaddr *)&client_address , &client_len);

                if(!(write_fp = fopen("./socketDM.txt","wb") )){
                        printf("error1\n");
                        return;
                }
                while(num_read = read(client_sockfd,buf,sizeof(buf)));
                

                if(fwrite(buf,16,1,write_fp) < num_read){
                        printf("error2\n");
                        return;
                }

                fclose(write_fp);
                close(client_sockfd);
                printf("%s",buf);       //for test
                //exit(1);
        }
}