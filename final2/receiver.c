#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <mqueue.h>
#include <pthread.h>

void* Send(void *arg)//hilo que se encarga de crear la cola para que el pueda enviar los mensajes
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

int main(int argc, char *argv[])//se encarga de recibir todos los mensajes y aparte de crear la cola
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
    pthread_create(&threadID1,NULL,&Send,NULL);

    while (1)
    {
        mq_receive(mq, buff, 64, NULL);
        printf("Sender Message: %s\n", buff);

        char* token2 = malloc(100);
        strcpy(token2, buff); //Se copia el contenido de buff en un puntero al que luego se le cambia el contenido
        //Se separan los parámetros con un "#" intermedio (Ejemplo: archivo.txt#UpDown)
        char *token1 = strsep(&token2,"#"); //Tras hacer el strsep() el contenido de lo que había en el buff (que se guardó en token2) pasa a ser el del token siguiente al guardado en token1 
        //token1 contiene la información de la ruta (archivo.txt)
        //token2 contiene el parámetro para el orden del archivo (UpDown) (DownUp)
        
        FILE *file;
        file = fopen(token1, "r");        

        char * fileContent = malloc(100);

        if (strcmp(token2, "UpDown") == 0)
        {
            while (fgets(fileContent, 100, file) != NULL)
            {
                if(fileContent[strlen(fileContent) - 1 ] == '\n') fileContent[strlen(fileContent) - 1 ] = 0;
                printf("Enviado al Sender: %s\n",fileContent);
                mq_send(mq1, fileContent, strlen(fileContent)+1, 0);
            }  
        }
        else if (strcmp(token2, "DownUp") == 0)
        {            
            int counter = 0;            
            char inverseContent[counter][100];
            
            while (fgets(fileContent, 100, file) != NULL)
            {
                if(fileContent[strlen(fileContent) - 1 ] == '\n') fileContent[strlen(fileContent) - 1 ] = 0;
                strcpy(inverseContent[counter], fileContent);                
                counter++;
            }
            for (size_t i = 0; i <= counter ; i++)
            {
                if(strcmp(inverseContent[counter-i], "") == 0) break;
                printf("Enviado al Sender: %s\n",inverseContent[counter-i]);
                mq_send(mq1, inverseContent[counter-i], strlen(inverseContent[counter-i])+1, 0);
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