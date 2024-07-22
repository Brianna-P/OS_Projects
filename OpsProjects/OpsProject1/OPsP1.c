//Name: Brianna Patten
//Username: patt0233

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

//Defining size of shared memory and key number
#define SHMKEY ((key_t) 1497)

typedef struct
{
int value;
} shared_mem;
shared_mem *total;

//Function to increment "total" based on individual process limits
void processInc(int numToIncrement, int num) {
    for (int i = 1; i <= numToIncrement; i++) {
        total->value++;
    }
    printf("From Process %d: counter = %d.\n", num, total->value);
    exit(0);
}

int main()
{
int shmid, pid1, pid2, pid3, pid4, ID, status;
pid_t pid;
int pidLimits[4] = {100000, 200000, 300000, 500000};
char *shmadd;

shmadd = (char *) 0;

// Create a shared memory segment, throw error if shmid is not 0+
if ((shmid = shmget (SHMKEY, sizeof(int), IPC_CREAT | 0666)) < 0){
    perror ("shmget");
    exit (1);
} else {
    printf("Successfully created shared memory.\n");
}
// Connect to a shared memory segment, , throw error if shmid is -1
if ((total = (shared_mem *) shmat (shmid, shmadd, 0)) == (shared_mem *) -1) {
    perror ("shmat");
    exit (0);
} else {
    printf("Successfully connected to shared memory.\n");
}

//Initialize "total" to 0
total->value = 0; 

//Create process for each pid and increment total as needed (with function)
for (int i = 0; i < 4; i++) {
    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1); 
    }
    if (pid == 0) {
        processInc(pidLimits[i], i+1);
    }
}

//Wait for children to finish running
for (int i = 0; i < 4; i++) {
    pid_t temp_pid = wait(NULL);
    printf("Child with ID: %d has just exited.\n", temp_pid);
}

//Detatch from shared memory segment, throw error if shmdt is -1
if (shmdt(total) == -1) {
    perror("shmdt");
    exit(-1);
} else {
    printf("Successfully detatched from shared memory.\n");
}
//Delete shared memory segment, throw error if shmctl is -1
if (shmctl(shmid, IPC_RMID, NULL) == -1) {
    perror("shmctl");
    exit(1);
} else {
    printf("Successfully deleted shared memory.\n");
}

//End program
printf("End of simulation.\n");
return 0;
}

