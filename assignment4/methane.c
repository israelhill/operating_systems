#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 125

// Semaphores
sem_t sem_h, sem_c, mutex;
// Shared variables
int waiting_h, waiting_c;

// struct to pass as param to thread
typedef struct thread_data_t {
    int t_id;
} thread_data;

// carbon thread routine
void *carbon_routine(void *arg);

// hydrogen thread routine
void *hydrogen_routine(void *arg);

// error checked sem_signal function
void semsignal(sem_t *sem);

// error checked sem_wait function
void semwait(sem_t *sem);


int main() {
    pthread_t threads[NUM_THREADS];
    thread_data t_data[NUM_THREADS];
    int ret_code;

    // initialize semaphores
    if(sem_init(&mutex, 0, (unsigned int)1) < 0 || sem_init(&sem_h, 0, (unsigned int)0) < 0
            || sem_init(&sem_c, 0, (unsigned int)0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    // create the carbon and hydrogen threads
    int i;
    for(i = 0; i < NUM_THREADS; i++) {
        // the routine each thread will be passed
        void *thread_function;

        if(i < 25) {
            t_data[i].t_id = i;
            thread_function = carbon_routine;
            ret_code = pthread_create(&threads[i], NULL, thread_function, &t_data[i]);
            printf("Main: Created Carbon thread #%d\n", i);
            if(ret_code != 0) {
                perror("pthread_create");
                exit(EXIT_FAILURE);
            }
        }
        else {
            t_data[i].t_id = i;
            thread_function = hydrogen_routine;
            ret_code = pthread_create(&threads[i], NULL, thread_function, &t_data[i]);
            printf("Main: Created Hydrogen thread #%d\n", i);
            if(ret_code != 0) {
                perror("pthread_create");
                exit(EXIT_FAILURE);
            }
        }
    }

    // wait for all the threads to terminate
    pthread_exit(NULL);
}

void *hydrogen_routine(void *arg) {
    thread_data *t_data = (thread_data *)arg;
    int thread_id = t_data->t_id;

    // acquire lock on mutex before accessing shared memory
    semwait(&mutex);

    // if enough C is waiting, continue past barrier
    printf("HYDROGEN %d reached barrier.\n", thread_id);
    if (waiting_h >= 3 && waiting_c >= 1) {
        printf("~~~~ Crossed barrier, created a Methane!\n\n");
        // release 4 H
        int i;
        for(i = 0; i < 3; i++) {
            semsignal(&sem_h);
        }
        waiting_h -= 3;
        semsignal(&sem_c);
        waiting_c--;
        // release lock on mutex
        semsignal(&mutex);
    }
    else {
        // not enough H or C is waiting, so wait at barrier
        waiting_h++;
        printf("Waiting Atoms -- C: %d | H: %d\n", waiting_c, waiting_h);
        // release lock on mutex
        semsignal(&mutex);
        semwait(&sem_h);
    }

    pthread_exit(EXIT_SUCCESS);

}

void *carbon_routine(void *arg) {
    thread_data *t_data = (thread_data *)arg;
    int thread_id = t_data->t_id;

    // acquire lock on mutex before accessing shared memory
    semwait(&mutex);

    // if enough H is waiting, continue past barrier
    printf("CARBON %d reached barrier.\n", thread_id);
    if (waiting_h >= 4) {
        printf("~~~~ Crossed barrier, created a Methane!\n\n");
        // release 4 H
        int i;
        for(i = 0; i < 4; i++) {
            semsignal(&sem_h);
        }
        waiting_h -= 4;
        // release lock on mutex
        semsignal(&mutex);
    }
    else {
        // not enough H is waiting, so wait at barrier
        waiting_c++;
        printf("Waiting Atoms -- C: %d | H: %d\n", waiting_c, waiting_h);
        // release lock on mutex
        semsignal(&mutex);
        semwait(&sem_c);
    }

    pthread_exit(EXIT_SUCCESS);
}

/*
*	Error-checked semaphore wait.
*/
void semwait(sem_t *sem) {
    if (sem_wait(sem) < 0) {
        perror("sem_wait");
        exit(EXIT_FAILURE);
    }
}

/*
*	Error-checked semaphore signal.
*/
void semsignal(sem_t *sem) {
    if (sem_post(sem) < 0) {
        perror("sem_post");
        exit(EXIT_FAILURE);
    }
}
