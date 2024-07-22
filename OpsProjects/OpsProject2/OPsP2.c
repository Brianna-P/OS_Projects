//Name: Brianna Patten
//Username: patt0233

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>

//Defining size of semaphore and key number
#define SEMKEY ((key_t) 400L)
#define NSEMS 1

//Defining size of shared memory and key number
#define SHMKEY ((key_t) 1497)

//Global variables for semaphore
int sem_id;
static struct sembuf OP = {0,-1,0};
static struct sembuf OV = {0,1,0};
struct sembuf *P =&OP;
struct sembuf *V =&OV;

//Union for semaphore operations
typedef union{
    int val;
    struct semid_ds *buf;
    ushort *array;
} semunion;
// POP function to acquire semaphore before critical section
int POP(void){
    int status;
    status = semop(sem_id, P,1);
    return status;
}
// VOP function to release semaphore after critical section
int VOP(void){
    int status;
    status = semop(sem_id, V,1);
    return status;
}

typedef struct{
    int value;
} shared_mem;
shared_mem *total;

//Function to increment "total" based on individual process limits
void processInc(int numToIncrement, int num) {
    for (int i = 1; i <= numToIncrement; i++) {
        // Acquire the semaphore to protect the critical section
        if (POP() == -1) {
            perror("Error in acquiring semaphore");
            exit(1);
        }
        total->value++; // Critical section
        
        // Release the semaphore after the critical section
        if (VOP() == -1) {
            perror("Error in releasing semaphore");
            exit(1);
        }
    }

}

int main(void)
{
//SEMAPHORE SECTION
int value, value1;
semunion semctl_arg;
semctl_arg.val = 1;

//Creating semaphore
sem_id = semget(SEMKEY, NSEMS, IPC_CREAT | 0666);
if(sem_id < 0){
    printf("Error in creating the semaphore./n");
}else{
    printf("Successfully created semaphore.\n");
}
//Initializing semaphore
value1 = semctl(sem_id, 0, SETVAL, semctl_arg);
value = semctl(sem_id, 0, GETVAL, semctl_arg);
if (value < 0){
    printf("Error detected in SETVAL.\n");
} else{
    printf("Successfully initialized semaphore.\n");
}

//SHARED MEMORY SECTION
int shmid, status;
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
    exit (1);
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
        printf("From Process %d: counter = %d.\n", i+1, total->value);
        exit(0);
    }
}

// Wait for children to finish running
    for (int i = 0; i < 4; i++) {
        pid_t temp_pid = wait(NULL);
        printf("Child with ID: %d has just exited.\n", temp_pid);
    }

    // De-allocate semaphore
    semctl_arg.val = 0;
    status = semctl(sem_id, 0, IPC_RMID, semctl_arg);
    if (status < 0) {
        printf("Error in removing the semaphore.\n");
    } else {
        printf("Successfully removed semaphore.\n");
    }

    // Detach from shared memory segment, throw error if shmdt is -1
    if (shmdt(total) == -1) {
        perror("shmdt");
        exit(-1);
    } else {
        printf("Successfully detached from shared memory.\n");
    }

    // Delete shared memory segment, throw error if shmctl is -1
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    } else {
        printf("Successfully deleted shared memory.\n");
    }

    // End program
    printf("End of simulation.\n");
    return 0;
}

