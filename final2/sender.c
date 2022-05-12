#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mqueue.h>
#include <pthread.h>

void* Receiver(void *arg){ 
    mqd_t mq1 = mq_open("/mq1",O_RDONLY);

    char buff[64];

    while (1){
        mq_receive(mq1, buff, 64, NULL);
        printf("Receiver Message: %s\n", buff);
        if (strncmp(buff, "exit", strlen("exit")) == 0){
            break;
        }
    }

    mq_close(mq1);
    mq_unlink("/mq0");
    exit(EXIT_SUCCESS);

}

int main(int argc, char* argv[]){ 
    mqd_t mq = mq_open("/mq0", O_WRONLY);
    char str[64];
    pthread_t threadID1;
    pthread_create(&threadID1,NULL,&Receiver,NULL);

    while (1){
        fgets(str, sizeof(str), stdin);
        if (str[strlen(str) - 1] == '\n') str[strlen(str) - 1] = 0;
        mq_send(mq, str, strlen(str) + 1, 0);
        if (strncmp(str, "exit", strlen("exit")) == 0){
            break;
        }
    }

    mq_close(mq);
    exit(EXIT_FAILURE);
}
