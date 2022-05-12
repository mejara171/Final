#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mqueue.h>
#include <pthread.h>

void* Sender(void *arg)
{
    mqd_t mq1;

    struct mq_attr attr1;
    attr1.mq_flags = 0;
    attr1.mq_maxmsg = 10;
    attr1.mq_msgsize = 64;
    attr1.mq_curmsgs = 0;

    mq1 = mq_open("/mq1", O_WRONLY | O_CREAT, 0644, &attr1);
    char str[64];

    while (1)
    {
        fgets(str, sizeof(str), stdin);
        if(str[strlen(str) - 1 ] == '\n') str[strlen(str) - 1 ] = 0;
        mq_send(mq1, str, strlen(str) + 1, 0);
        if (strncmp(str, "exit", strlen("exit")) == 0)
        {
            break;
        }
    }

    mq_close(mq1);
    mq_unlink("/mq1");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    mqd_t mq;
    mqd_t mq1;

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 64;
    attr.mq_curmsgs = 0;

    mq = mq_open("/mq0", O_RDONLY | O_CREAT, 0644, &attr);
    mq1 = mq_open("/mq1", O_WRONLY | O_CREAT, 0644, &attr);

    char buff[32];

    pthread_t threadID1;
    pthread_create(&threadID1,NULL,&Sender,NULL);

    while (1)
    {
        mq_receive(mq, buff, 64, NULL);
        printf("sent: %s\n", buff);

        char* param1 = malloc(100);
        strcpy(param1, buff); 
        char *param0 = strsep(&param1,"$"); 

        
        FILE *file;
        file = fopen(param0, "r");        

        char * Content = malloc(100);

        if (strcmp(param1, "above") == 0)
        {
            while (fgets(Content, 100, file) != NULL)
            {
                if(Content[strlen(Content) - 1 ] == '\n') Content[strlen(Content) - 1 ] = 0;
                printf("Sent: %s\n",Content);
                mq_send(mq1, Content, strlen(Content)+1, 0);
            }  
        }
        else if (strcmp(param1, "below") == 0)
        {            
            int counter = 0;            
            char InverseContent[counter][100];
            
            while (fgets(Content, 100, file) != NULL)
            {
                if(Content[strlen(Content) - 1 ] == '\n') Content[strlen(Content) - 1 ] = 0;
                strcpy(InverseContent[counter], Content);                
                counter++;
            }
            for (size_t i = 0; i <= counter ; i++)
            {
                if(strcmp(InverseContent[counter-i], "") == 0) break;
                printf("Enviado al Sender: %s\n",InverseContent[counter-i]);
                mq_send(mq1, InverseContent[counter-i], strlen(InverseContent[counter-i])+1, 0);
            }            
        }     
                
        if( strncmp(buff, "exit", strlen("exit")) == 0){
            break;
        }
    }

    mq_close(mq);
    mq_unlink("/mq0");
    exit(EXIT_SUCCESS);
}
