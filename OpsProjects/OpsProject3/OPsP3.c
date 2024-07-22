//Brianna Patten 
#define _REENTRANT
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 15
#define FILENAME "mytest.dat"

char buffer[BUFFER_SIZE];
sem_t mutex, empty, full;

//Producer function: read from file and write to buffer
void *producer() {
    char newChar;
    int count = 0;
    FILE* fp;
    fp= fopen(FILENAME, "r");
    //Check if the file is opened successfully or not
    if (fp == NULL) {
        perror("Error opening file");
        return NULL;
    }
    while(fscanf(fp,"%c",&newChar) != EOF){
        sem_wait(&empty);
        sem_wait(&mutex);
        buffer[count] = newChar;
        sem_post(&mutex);
        sem_post(&full);
        //Increment the count
        count = (count + 1)  % BUFFER_SIZE; 

    }
    //Add a '*' to the buffer to indicate the end of the file
    sem_wait(&empty);
    sem_wait(&mutex);
    buffer[count] = '*';
    sem_post(&mutex);
    sem_post(&full);

    //Close the file
    fclose(fp);
    return NULL;
}

//Consumer function: read from buffer and print to the screen
void *consumer() {
    char newChar;
    int count = 0;
    //Loop until the '*' is read from the buffer
    while(1){
        sleep(1);
        sem_wait(&full);
        sem_wait(&mutex);
        newChar = buffer[count];
        sem_post(&mutex);
        sem_post(&empty);
        //If the '*' is read, break the loop
        if(newChar == '*'){
            break;
        }
        printf("%c",newChar);
        fflush(stdout);
        //Increment the count and sleep for 1 second
        count = (count + 1) % BUFFER_SIZE;
    }
    return NULL;
}

int main(void) {
    //Initialize the semaphores and create 2 threads
    pthread_t tid1, tid2;
    sem_init(&mutex, 0, 1);
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    //Create the threads and wait for them to finish
    pthread_create(&tid1, NULL, producer, NULL);
    pthread_create(&tid2, NULL, consumer, NULL);
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
    //Destroy the semaphores
    sem_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);
    //End of program
    return 0;
}
