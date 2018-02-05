/*
ツイッターから取得した文章をテキストファイルとして転送
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
int main(void)
{
        FILE *read_fp;
        char buf[100];
        int sockfd;
        int len;
        struct sockaddr_in address;
        int result;

        sockfd = socket(AF_INET,SOCK_STREAM,0);

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = inet_addr("10.1.69.83");  //転送先IPアドレス
        address.sin_port = 9734;                            //ポートの指定

        len = sizeof(address);
        result = connect(sockfd, (struct sockaddr *)&address, len);

        if(result==-1){
                perror("oops");
                exit(1);
        }

        if(!(read_fp = fopen("./RevisionDM.txt","rb") ))    //ファイル名
                return;
        fread(buf,sizeof(buf),1,read_fp);

        while(fgets(buf,sizeof(buf),read_fp)!=NULL);

        write(sockfd,buf,sizeof(buf));
        fclose(read_fp);
        close(sockfd);
        exit(0);
}
