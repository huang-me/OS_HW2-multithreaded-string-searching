#include "server.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

char* headtail(char*);
char* headtail(char*);
void push(int);
int pop(void);
int isEmpty(void);
int file_exist(char*);
void mainthread_func(void*);
void threadpool_thread(void*);
int findfile(char*,char*);
int readfile(char*,char*);

char str_head[100],*search[10],*port;
struct node
{
    int data;
    struct node* next;
};
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
struct node * Q_head = NULL;
struct node * Q_tail = NULL;
int Q_num = 0;


int main(int argc, char *argv[])
{
    //get input and argv
    char *root = argv[2];
    port = argv[4];
    char *thread_num = argv[6];
    memset(str_head,0,sizeof(str_head));
    strcat(str_head,root);

    //create socket
    int sockfd = 0,forClientSockfd = 0;
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd == -1)
    {
        printf("Fail to create a socket.");
    }

    //connection of the socket
    struct sockaddr_in serverInfo,clientInfo;
    socklen_t addrlen = sizeof(clientInfo);
    memset(&serverInfo,0,sizeof(serverInfo));
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverInfo.sin_port = htons(atoi(port));

    bind(sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
    listen(sockfd,100);

    //create threadpool
    int thread_count = atoi(thread_num);
    pthread_t *thread_pool = (pthread_t *)malloc(sizeof(pthread_t)* thread_count);
    for(int i=0; i<thread_count; i++)
    {
        pthread_create(&(thread_pool[i]),NULL,threadpool_thread,(void *)&i);
        usleep(2000);
    }

    //create mainthread
    pthread_t mainthread;
    pthread_create(&mainthread,NULL,mainthread_func,NULL);
    while(1)
    {
        forClientSockfd = accept(sockfd,(struct sockaddr*)&clientInfo,&addrlen);
        // printf("sockfd = %d\n",forClientSockfd);
        pthread_mutex_lock(&lock);
        push(forClientSockfd);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);
        // printf("in server.\n");
    }
    // printf("out of while.\n");
    pthread_join(*thread_pool,NULL);

    return 0;
}

char* headtail(char* str)
{
    str++;
    str[strlen(str)-1] = 0;
    return str;
}

void push(int data)
{
    if(Q_head == NULL)
    {
        Q_head = (struct node*)malloc(sizeof(struct node));
        Q_head->data = data;
        Q_head->next = NULL;
        Q_tail = Q_head;
    }
    else
    {
        struct node* ptr = (struct node*)malloc(sizeof(struct node));
        ptr->data = data;
        ptr->next = NULL;
        Q_tail->next = ptr;
        Q_tail = ptr;
    }
    Q_num++;
}

int pop(void)
{
    struct node* ptr = Q_head;
    int result = ptr->data;
    Q_head = ptr->next;
    free(ptr);
    Q_num--;
    return result;
}

int isEmpty(void)
{
    if(Q_num == 0) return 1;
    else return 0;
}

int file_exist(char* pathname)
{
    struct stat sb;
    if(stat(pathname,&sb)==0&&S_ISDIR(sb.st_mode))
    {
        //directory
        return 0;
    }
    else if(stat(pathname,&sb)==0&&S_ISREG(sb.st_mode))
    {
        //file
        return 0;
    }
    else  //not found
        return 1;
}

void mainthread_func(void *args)
{
    // printf("in the main thread.\n");
    return 0;
}

void threadpool_thread(void* args)
{
    int num = *(int*)args,searchnum;
    while(1)
    {
        pthread_mutex_lock(&lock);
        while(isEmpty())
        {
            pthread_cond_wait(&cond,&lock);
        }
        int inputsockfd = pop();
        char inputBuffer[1000] = {};
        memset(inputBuffer,0,sizeof(inputBuffer));
        recv(inputsockfd,inputBuffer,sizeof(inputBuffer),0);
        pthread_mutex_unlock(&lock);
        // printf("inputbuff=%s\n",inputBuffer);

        //copy inputBuffer to input
        char buffer_tmp[256],*temp,*buffer;
        memset(buffer_tmp,0,sizeof(buffer_tmp));
        strcat(buffer_tmp,"Query ");
        strcat(buffer_tmp,inputBuffer);
        // printf("%s\n",buffer_tmp);
        buffer = strtok(buffer_tmp,"\"");
        temp = strtok(NULL,"\"");
        // temp = headtail(temp);
        // printf("%s\n",temp);
        search[0] = temp;
        int i=1;
        while(search[i]=strtok(NULL,"\""))
        {
            // search[i] = headtail(search[i]);
            // printf("%s.\n",search[i]);
            i++;
        }
        searchnum = i;
        printf("Query ");
        for(i=0; i<searchnum; i++)
        {
            printf("\"%s\" ",search[i]);
        }
        printf("\n");

        //create output msg
        char output[1000],*newpath,toClient[30][500];
        int cnt = 0;
        memset(output,0,sizeof(output));
        memset(toClient,0,sizeof(toClient));

        //str_head
        // printf("str_head = %s\n",str_head);

        char paths[20][200];
        int path_num = 0,startnum = 0;
        char pathINfunc[1000],inttochar[1000];
        // strcpy(pathINfunc,str_head);
        //find files
        struct stat sb;
        for(i=0; i<searchnum; i++)
        {
            path_num = 0;
            startnum = 0;
            // printf("%s\n",search[i]);
            strcpy(pathINfunc,str_head);
            while(startnum<=path_num)
            {
                memset(output,0,sizeof(output));
                if(stat(pathINfunc,&sb)==0&&S_ISDIR(sb.st_mode))
                {
                    //directory
                    findfile(pathINfunc,output);
                    newpath = strtok(output," ");
                    strcat(paths[path_num],pathINfunc);
                    strcat(paths[path_num],"/");
                    strcat(paths[path_num],newpath);
                    path_num++;
                    while(newpath = strtok(NULL," "))
                    {
                        strcat(paths[path_num],pathINfunc);
                        strcat(paths[path_num],"/");
                        strcat(paths[path_num],newpath);
                        path_num++;
                    }
                    // printf("paths[%d] = %s\n",startnum,pathINfunc);
                }
                else if(stat(pathINfunc,&sb)==0&&S_ISREG(sb.st_mode))
                {
                    //file
                    int count=0;
                    count = readfile(pathINfunc,search[i]);
                    // printf("file = %s\n",pathINfunc);
                    strcat(toClient[cnt],search[i]);
                    strcat(toClient[cnt],"~");
                    strcat(toClient[cnt],pathINfunc);
                    strcat(toClient[cnt],"~");
                    if(count == 0)
                    {
                        // printf("Not found\n");
                        strcat(toClient[cnt],"Notfound");
                    }
                    else
                    {
                        // printf("%d\n",count);
                        sprintf(inttochar,"%d",count);
                        strcat(toClient[cnt],inttochar);
                    }
                    // printf("toclient[%d] = %s\n",cnt,toClient[cnt]);
                    cnt++;
                }
                strcpy(pathINfunc,paths[startnum]);
                startnum++;
            }
            for(int j=0; j<path_num; j++)
            {
                memset(paths[j],0,sizeof(paths[j]));
            }
        }
        char backmsg[10000];
        memset(backmsg,0,sizeof(backmsg));
        for(i=0; i<cnt; i++)
        {
            strcat(backmsg,toClient[i]);
            strcat(backmsg,"~");
        }
        // printf("*%s*\n",backmsg);
        send(inputsockfd,backmsg,sizeof(backmsg),0);
    }
    pthread_exit(NULL);
}

int findfile(char* path,char* output)
{
    DIR *d;
    struct dirent* dir;
    char file_path[200];
    memset(file_path,0,sizeof(file_path));
    strcat(file_path,"./");
    strcat(file_path,path);
    d = opendir(file_path);
    if(d)
    {
        while((dir = readdir(d))!=NULL)
        {
            if(strcmp(dir->d_name,".")&&strcmp(dir->d_name,".."))
            {
                // printf("dir = %s\n",dir->d_name);
                strcat(output,dir->d_name);
                strcat(output," ");
            }
        }
        closedir(d);
        return 1;
    }
    else
        return 0;
}

int readfile(char *path,char *key)
{
    char buffer[7000],tmpkey[130],queue[100][130];
    memset(buffer,0,sizeof(buffer));
    strcpy(tmpkey,key);
    char ch,*index;
    int count = 0,i = 0,size = strlen(tmpkey),j,same;
    FILE *file;
    file = fopen(path,"r");
    while((ch=fgetc(file))!=EOF)
    {
        // index[i] = ch;
        buffer[i] = ch;
        i++;
    }
    // printf("%s\n",buffer);
    for(i=0; i<(strlen(buffer)-strlen(tmpkey))+1; i++)
    {
        same = 0;
        for(j=0; j<strlen(tmpkey); j++)
        {
            if(buffer[i+j]==tmpkey[j])
            {
                same++;
            }
            else
            {
                continue;
            }
        }
        if(same == strlen(tmpkey))
        {
            count++;
        }
    }
    return count;
}