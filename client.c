#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#define max_buffer_size 10000

char* port;

int check(char *);

int main(int argc, char *argv[])
{
    char* server = argv[2];
    port = argv[4];
    int test = atoi(argv[4]);
    // printf("server = %s, port = %d\n",server,test);
    int sockfd = 0;
    char msg[1000],sendtoserver[7000];
    char *tmp[10],*useless;
    int i = 0,j,numofquo;

    while(1)
    {
        //input message
        gets(msg);
        if(strlen(msg)>128)
        {
            printf("The input is too long\n");
            continue;
        }
        if(msg[strlen(msg)-1]!='"')
        {
            printf("The string format is not correct.\n");
            continue;
        }
        // strcat(msg," ");
        tmp[i++] = strtok(msg,"\"");
        numofquo = 0;
        // printf("%s.\n",tmp[0]);
        if(!(!strcmp(tmp[0],"Query ")||!strcmp(tmp[0],"query ")))
        {
            printf("The string fomat is not correct.\n");
            continue;
        }
        memset(sendtoserver,0,sizeof(sendtoserver));
        while(tmp[i] = strtok(NULL,"\""))
        {
            numofquo++;
            // printf("%s.\n",tmp[i]);
            strcat(sendtoserver,"\"");
            strcat(sendtoserver,tmp[i]);
            strcat(sendtoserver,"\"");
            // strcat(sendtoserver," ");
            if(useless = strtok(NULL,"\""))
                numofquo++;
            i++;
        }
        if((numofquo%2)!=1)
        {
            printf("The strings format is not correct\n");
            continue;
        }

        //setting sockfd
        sockfd = socket(AF_INET,SOCK_STREAM,0);
        if(sockfd == -1)
        {
            printf("Fail to create a new socket.\n");
            return 0;
        }

        //connection of the socket
        struct sockaddr_in info = {0};
        memset(&info,0,sizeof(info));
        info.sin_family = AF_INET;

        //local host test
        info.sin_addr.s_addr = inet_addr("127.0.0.1");
        info.sin_port = htons(atoi(port));

        int err = connect(sockfd,(struct sockaddr*)&info,sizeof(info));
        if(err == -1)
        {
            printf("Connection error.\n");
            return 0;
        }


        // printf("%s\n",sendtoserver);
        send(sockfd, sendtoserver, sizeof(sendtoserver), 0);

        //recv msg
        char receivemessage[10000];
        memset(receivemessage,0,sizeof(receivemessage));
        recv(sockfd, receivemessage, sizeof(receivemessage), 0);
        // printf("%s\n",receivemessage);

        //deal the bcakmsg and print
        char *out[100];
        out[0] = strtok(receivemessage,"~");
        out[1] = strtok(NULL,"~");
        out[2] = strtok(NULL,"~");
        i=3;
        int count,start = 0,files = 1,notfound = 0;
        while(out[i] = strtok(NULL,"~"))
        {
            // printf("%s\n",out[i]);
            i++;
        }
        count = i;
        for(i=0; i<count;)
        {
            if(!out[i+3])
            {
                printf("String: \"%s\"\n",out[start]);
                for(j=0; j<files; j++)
                {
                    if(!strcmp(out[start+2+(3*j)],"Notfound"))
                        notfound++;
                }
                // printf("si\n");
                // printf("%d,%d",notfound,files);
                if(notfound==files)
                    printf("Not found\n");
                else
                {
                    // printf("pp\n");
                    for(j=0; j<files; j++)
                    {
                        if(strcmp(out[start+2+(3*j)],"Notfound"))
                        {
                            printf("File: ./%s, Count: %s\n",out[start+1+(3*j)],out[start+2+(3*j)]);
                        }
                    }
                }
            }
            else if(!strcmp(out[i],out[i+3]))
            {
                files++;
            }
            else
            {
                // printf("ccc\n");
                printf("String: \"%s\"\n",out[start]);
                for(j=0; j<files; j++)
                {
                    if(!strcmp(out[start+2+(3*j)],"Notfound"))
                        notfound++;
                }
                if(notfound==files)
                    printf("Not found\n");
                else
                {
                    for(j=0; j<files; j++)
                    {
                        if(strcmp(out[start+2+(3*j)],"Notfound"))
                        {
                            printf("File: ./%s, Count: %s\n",out[start+1+(3*j)],out[start+2+(3*j)]);
                        }
                    }
                }
                start = start + (3*files);
                files = 1;
                notfound = 0;
            }
            i+=3;

        }
        //end the socket
        close(sockfd);
    }
}

int check(char *str)
{
    char* temp;
    char* position = strchr(str,'"');
    int pos = (position == NULL ? -1 : position - str);
    char* r_position = strrchr(str,'"');
    int r_pos = (r_position == NULL ? -1 : r_position - str);
    if((pos==0)&&(r_pos==strlen(str)-1))
        return 0;
    else return 1;
}

