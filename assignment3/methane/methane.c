#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>


#define SEMKEY 77
#define SHMKEY 77
#define K 1024//used for shared memory creation

#define NUM_SEMS 3
#define MUTEX 0
#define SC 1
#define SH 2

#define NUM_ATOMS 125

union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
};

struct sharedData {
    int waiting_H;
    int waiting_C;
};

void fork_carbon();
void fork_hydrogen();
void semWait(int semid, int semaphore);
void semSignal(int semid, int semaphore);
int get_shmid(key_t shmkey);
int get_semid(key_t semkey);
void do_carbon();
void do_hydrogen();

int main() {
    int shm_id, sem_id;
    char atoms[NUM_ATOMS];
    struct sharedData *shared_resources;

    // populate the atoms array with 100 Hydrogen and 25 Carbons.
    int k;
    for(k = 0; k < NUM_ATOMS; k++) {
        if(k < 25) {
            atoms[k] = 'c';
        }
        else {
            atoms[k] = 'h';
        }
    }

    //used to initialize semaphores
    unsigned short seminit[NUM_SEMS];
    union semun semctlarg;

    //get semaphore memory id using wrapper function
    sem_id = get_semid((key_t) SEMKEY);


    // set initial values for semaphores
    seminit[MUTEX] = 1;
    seminit[SH] = 0;
    seminit[SC] = 0;
    semctlarg.array = seminit;

    //apply initialization
    if ((semctl(sem_id, NUM_SEMS, SETALL, semctlarg)) < 0) {
        perror("semctl");
        exit(EXIT_FAILURE);
    }

    //get shared memory id using wrapper function
    shm_id = get_shmid((key_t) SHMKEY);

    //retrieve pointer to shared data structure
    if ((shared_resources = (struct sharedData *)shmat(shm_id, 0, 0)) < 0) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // initialize values of shared memory
    shared_resources->waiting_C = 0;
    shared_resources->waiting_H = 0;

    printf("I am the parent. PID: %d.\n\n", getpid());

    int i;
    for(i = 0; i < NUM_ATOMS; i++) {
        switch(atoms[i]) {
            case 'c':
                fork_carbon();
                break;
            case 'h':
                fork_hydrogen();
                break;
            default:
                perror("ERROR: Element not recognized for this compound");
                break;
        }
    }

    int j;
    for(j = 0; j < NUM_ATOMS; j++) {
        wait(NULL);
    }

    // Clean up memory
    if (shmdt(shared_resources) == -1) {
        perror("ERROR: shmdt");
        exit(EXIT_FAILURE);
    }

    if (shmctl(shm_id, IPC_RMID, NULL) < 0) {
        perror("ERROR: shmctrl");
        exit(EXIT_FAILURE);
    }

    if (semctl(sem_id, MUTEX, IPC_RMID, semctlarg) == -1) {
        perror("ERROR: semctl");
        exit(EXIT_FAILURE);
    }

    printf("Done!\n");
    return EXIT_SUCCESS;	

}

void fork_carbon() {
    pid_t child;
    child = fork();

    if(child == -1) {
        perror("An error occurred while forking carbon.");
        exit(EXIT_FAILURE);
    }
    else if(child == 0) {
        // child process
        printf("Created CARBON: %d\n", getpid());
        do_carbon();
    }
    else {
        // parent
        return;
    }
}

void fork_hydrogen() {
    pid_t child;
    child = fork();

    if(child == -1) {
        perror("An error occurred while forking hydrogen.");
        exit(EXIT_FAILURE);
    }
    else if(child == 0) {
        // child process
        printf("Created HYDROGEN: %d\n", getpid());
        do_hydrogen();
    }
    else {
        // parent
        return;
    }
}

void do_carbon() {
    int shared_mem_id = get_shmid((key_t)SHMKEY);
    int semaphore_id = get_semid((key_t)SEMKEY);
    struct sharedData *sharedResources = shmat(shared_mem_id, 0, 0);

    // acquire lock on mutex before accessing shared memory
    semWait(semaphore_id, MUTEX);

    // if enough H is waiting, continue past barrier
    printf("CARBON %d reached barrier.\n", getpid());
    if (sharedResources->waiting_H >= 4) {
        printf("~~~~ Crossed barrier, created a Methane!\n\n");
        // release 4 H
        int i;
        for(i = 0; i < 4; i++) {
            semSignal(semaphore_id, SH);
        }
        sharedResources->waiting_H -= 4;
        // release lock on mutex
        semSignal(semaphore_id, MUTEX);
    }
    else { // not enough H is waiting, so wait at barrier
        sharedResources->waiting_C++;
        printf("Waiting Atoms -- C: %d | H: %d\n", sharedResources->waiting_C, sharedResources->waiting_H);
        // release lock on mutex
        semSignal(semaphore_id, MUTEX);
        semWait(semaphore_id, SC);
    }

    if (shmdt(sharedResources) == -1) {
        perror("shmdt failed");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

void do_hydrogen() {
    int shared_mem_id = get_shmid((key_t)SHMKEY);
    int semaphore_id = get_semid((key_t)SEMKEY);
    struct sharedData *sharedResources = shmat(shared_mem_id, 0, 0);

    // acquire lock on mutex before accessing shared memory
    semWait(semaphore_id, MUTEX);

    // if enough C is waiting, continue past barrier
    printf("HYDROGEN %d reached barrier.\n", getpid());
    if (sharedResources->waiting_H >= 3 && sharedResources->waiting_C >= 1) {
        printf("~~~~ Crossed barrier, created a Methane!\n\n");
        // release 4 H
        int i;
        for(i = 0; i < 3; i++) {
            semSignal(semaphore_id, SH);
        }
        sharedResources->waiting_H -= 3;
        semSignal(semaphore_id, SC);
        sharedResources->waiting_C--;
        // release lock on mutex
        semSignal(semaphore_id, MUTEX);
    }
    else { // not enough H or C is waiting, so wait at barrier
        sharedResources->waiting_H++;
        printf("Waiting Atoms -- C: %d | H: %d\n", sharedResources->waiting_C, sharedResources->waiting_H);
        // release lock on mutex
        semSignal(semaphore_id, MUTEX);
        semWait(semaphore_id, SH);
    }

    if (shmdt(sharedResources) == -1) {
        perror("shmdt failed");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

void semWait(int semid, int semaphore) {
    struct sembuf psembuf;

    psembuf.sem_op = -1;
    psembuf.sem_flg = 0;
    psembuf.sem_num = semaphore;
    semop(semid,&psembuf,1);
    return;
}

void semSignal(int semid, int semaphore) {
    struct sembuf vsembuf;

    vsembuf.sem_op = 1;
    vsembuf.sem_flg = 0;
    vsembuf.sem_num = semaphore;
    semop(semid,&vsembuf,1);
    return;
}

// wrapper function: get semaphore id for semaphore
int get_semid(key_t semkey) {
    int value = semget(semkey, NUM_SEMS, 0777 | IPC_CREAT);
    if (value == -1) {
        perror("ERROR: semget() failed");
        exit(EXIT_FAILURE);
    }
    return value;
}

// wrapper function: get shared memory id for shared memory
int get_shmid(key_t shmkey) {
    int value = shmget(shmkey, sizeof(struct sharedData), 0777 | IPC_CREAT);
    if (value == -1) {
        perror("ERROR: shmget() failed");
        exit(EXIT_FAILURE);
    }
    return value;
}
