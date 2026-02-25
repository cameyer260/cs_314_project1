/*
 * **********************************
 * CS314 Project #1 solution
 * 
 * Christopher Meyer
 * Last-three: 510
 * Course section #: 003
 *
 * Spring 2026
 *
 * Synchronized Swimming Pool Control Problem
 * Using UNIX standard semaphores and shared memory
 *
 **********************************
 ****/

#include <stdio.h>          /* printf */
#include <stdlib.h>         /* rand, exit */
#include <sys/types.h>      /* pid_t */
#include <unistd.h>         /* usleep, fork */
#include <time.h>           /* time */
#include <sys/ipc.h>        /* IPC constants */
#include <sys/shm.h>        /* shmget, shmat, shmdt, shmctl */
#include <sys/sem.h>        /* semget, semctl, semop */
#include <sys/wait.h>       /* wait */

/* ========== CONSTANTS ========== */

#define NUM_REPEAT                 50       /* number of times each boiler-man heats water */

/* Personal IPC keys (from 04_personal_keys.md) */
#define SEM_KEY_01                 8370     /* semaphore S1: pool access mutex */
#define SEM_KEY_02                 8371     /* semaphore S2: bather_count mutex */
#define SHM_KEY                    7205     /* shared memory key */

/* 
 * Timing constants (in microseconds)
 * _A = time outside critical section (waiting)
 * _B = time inside critical section (working)
 */
#define BATHER_TIME_01_A           300000   /* A1 time outside pool (~0.3 seconds) */
#define BATHER_TIME_01_B           800000   /* A1 time inside pool (~0.8 seconds) */

#define BATHER_TIME_02_A           300000   /* A2 time outside pool (~0.3 seconds) */
#define BATHER_TIME_02_B           800000   /* A2 time inside pool (~0.8 seconds) */

#define BATHER_TIME_03_A           300000   /* A3 time outside pool (~0.3 seconds) */
#define BATHER_TIME_03_B           800000   /* A3 time inside pool (~0.8 seconds) */

#define BOILERMAN_TIME_01_A        1200000  /* B1 wait between heating (~1.2 seconds) */
#define BOILERMAN_TIME_01_B        1600000  /* B1 time heating water (~1.6 seconds) */

#define BOILERMAN_TIME_02_A        1200000  /* B2 wait between heating (~1.2 seconds) */
#define BOILERMAN_TIME_02_B        1600000  /* B2 time heating water (~1.6 seconds) */

#define SAFEGUARD_TIME_A           1200000  /* S wait between inspections (~1.2 seconds) */
#define SAFEGUARD_TIME_B           1600000  /* S time inspecting (~1.6 seconds) */

/* ========== SHARED MEMORY STRUCTURE ========== */

struct shared_memory {
    int ready_count;      /* number of child processes ready to start */
    int start_flag;       /* 1 = all processes can begin work */
    int bw1, bw2;         /* boiler-man status: 1=active, 0=finished */
    int ba1, ba2, ba3;    /* bather status: 1=active, 0=finished */
    int bather_count;     /* number of bathers currently in pool */
};

/* ========== SEMAPHORE UNION ========== */
/* Required for semctl() on some systems (including SIUE Unix server) */

#ifndef __APPLE__
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};
#endif

/* ========== FUNCTION PROTOTYPES ========== */

int rand_sleep(int max_time);
void semaphore_wait(int sem_id);
void semaphore_signal(int sem_id);

/* ========== RAND_SLEEP FUNCTION ========== */
/* Returns random value between 0 and max_time microseconds */

int rand_sleep(int max_time)
{
    float temp_rand;
    float temp_sleeptime;

    temp_rand = (float)((rand() % 100)) / 100.0;
    temp_sleeptime = ((float)max_time) * temp_rand;

    return ((int)temp_sleeptime);
}

/* ========== SEMAPHORE WAIT (P OPERATION) ========== */
/* Decrements semaphore. Blocks if semaphore value is 0. */

void semaphore_wait(int sem_id)
{
    struct sembuf op;
    op.sem_num = 0;      /* semaphore index in set */
    op.sem_op = -1;      /* decrement operation */
    op.sem_flg = 0;      /* blocking mode */
    semop(sem_id, &op, 1);
}

/* ========== SEMAPHORE SIGNAL (V OPERATION) ========== */
/* Increments semaphore. Wakes up waiting processes. */

void semaphore_signal(int sem_id)
{
    struct sembuf op;
    op.sem_num = 0;      /* semaphore index in set */
    op.sem_op = 1;       /* increment operation */
    op.sem_flg = 0;
    semop(sem_id, &op, 1);
}

/* ========== MAIN FUNCTION ========== */

int main(void)
{
    int i;
    int sleep_time;
    pid_t pid;
    int status;
    int my_id;
    
    int sem_id_01, sem_id_02;
    int shm_id;
    struct shared_memory *p_shm;
    int shm_size;
    union semun argument;
    int ret_val;

    srand((unsigned int)time(NULL));

    /* 
     * ===== CREATE IPC RESOURCES =====
     * Must be done BEFORE forking so all processes inherit access
     */

    shm_size = sizeof(struct shared_memory);
    shm_id = shmget(SHM_KEY, shm_size, 0666 | IPC_CREAT);
    if (shm_id < 0)
    {
        fprintf(stderr, "Failed to create shared memory. Terminating...\n");
        exit(1);
    }

    p_shm = (struct shared_memory *)shmat(shm_id, NULL, 0);
    if (p_shm == (struct shared_memory *)-1)
    {
        fprintf(stderr, "Failed to attach shared memory. Terminating...\n");
        exit(1);
    }

    /* Initialize shared memory values */
    p_shm->ready_count = 0;     /* no children ready yet */
    p_shm->start_flag = 0;      /* children must wait */
    p_shm->bw1 = 1;             /* B1 is active */
    p_shm->bw2 = 1;             /* B2 is active */
    p_shm->ba1 = 1;             /* A1 is active */
    p_shm->ba2 = 1;             /* A2 is active */
    p_shm->ba3 = 1;             /* A3 is active */
    p_shm->bather_count = 0;    /* no bathers in pool */

    /* Create semaphore S1: mutex for pool access */
    sem_id_01 = semget(SEM_KEY_01, 1, 0666 | IPC_CREAT);
    if (sem_id_01 < 0)
    {
        fprintf(stderr, "Failed to create semaphore S1. Terminating...\n");
        exit(1);
    }
    argument.val = 1;
    if (semctl(sem_id_01, 0, SETVAL, argument) < 0)
    {
        fprintf(stderr, "Failed to initialize semaphore S1. Terminating...\n");
        exit(1);
    }

    /* Create semaphore S2: mutex for bather_count variable */
    sem_id_02 = semget(SEM_KEY_02, 1, 0666 | IPC_CREAT);
    if (sem_id_02 < 0)
    {
        fprintf(stderr, "Failed to create semaphore S2. Terminating...\n");
        exit(1);
    }
    argument.val = 1;
    if (semctl(sem_id_02, 0, SETVAL, argument) < 0)
    {
        fprintf(stderr, "Failed to initialize semaphore S2. Terminating...\n");
        exit(1);
    }

    /* 
     * ===== CREATE 5 CHILD PROCESSES =====
     * Order: B1, B2, A1, A2, A3
     * Each child: signals ready, waits for start_flag, does work, exits
     */

    /* ===== FORK B1 (Boiler-man 1) ===== */
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Fork B1 failed. Terminating...\n");
        exit(1);
    }
    else if (pid == 0)
    {
        /* --- B1 CHILD PROCESS --- */
        my_id = 1;
        p_shm->ready_count++;               /* signal parent: ready */
        while (p_shm->start_flag == 0)      /* wait for all processes ready */
        {
            usleep(1000);
        }

        /* Main loop: heat water NUM_REPEAT times */
        for (i = 0; i < NUM_REPEAT; i++)
        {
            sleep_time = rand_sleep(BOILERMAN_TIME_01_A);
            usleep(sleep_time);             /* wait outside critical section */

            semaphore_wait(sem_id_01);      /* enter critical section */

            printf("B%d starts his boiler ...\n", my_id);
            sleep_time = rand_sleep(BOILERMAN_TIME_01_B);
            usleep(sleep_time);             /* time heating water */
            printf("B%d is leaving the bathing area ..\n", my_id);

            semaphore_signal(sem_id_01);    /* exit critical section */
        }

        p_shm->bw1 = 0;                     /* mark B1 as finished */
        exit(0);
    }

    /* ===== FORK B2 (Boiler-man 2) ===== */
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Fork B2 failed. Terminating...\n");
        exit(1);
    }
    else if (pid == 0)
    {
        /* --- B2 CHILD PROCESS --- */
        my_id = 2;
        p_shm->ready_count++;
        while (p_shm->start_flag == 0)
        {
            usleep(1000);
        }

        for (i = 0; i < NUM_REPEAT; i++)
        {
            sleep_time = rand_sleep(BOILERMAN_TIME_02_A);
            usleep(sleep_time);

            semaphore_wait(sem_id_01);

            printf("B%d starts his boiler ...\n", my_id);
            sleep_time = rand_sleep(BOILERMAN_TIME_02_B);
            usleep(sleep_time);
            printf("B%d is leaving the bathing area ..\n", my_id);

            semaphore_signal(sem_id_01);
        }

        p_shm->bw2 = 0;
        exit(0);
    }

    /* ===== FORK A1 (Bather 1) ===== */
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Fork A1 failed. Terminating...\n");
        exit(1);
    }
    else if (pid == 0)
    {
        /* --- A1 CHILD PROCESS --- */
        my_id = 1;
        p_shm->ready_count++;
        while (p_shm->start_flag == 0)
        {
            usleep(1000);
        }

        /* 
         * Main loop: swim while boiler-men are active
         * Uses readers-writer pattern: multiple bathers can enter pool
         */
        while (p_shm->bw1 == 1 || p_shm->bw2 == 1)
        {
            sleep_time = rand_sleep(BATHER_TIME_01_A);
            usleep(sleep_time);             /* wait outside pool */

            /* Entry protocol: first bather locks pool */
            semaphore_wait(sem_id_02);
            p_shm->bather_count++;
            if (p_shm->bather_count == 1)
            {
                semaphore_wait(sem_id_01);  /* first bather locks S1 */
            }
            semaphore_signal(sem_id_02);

            /* Critical section: in the pool */
            printf("A%d is entering the swimming pool\n", my_id);
            sleep_time = rand_sleep(BATHER_TIME_01_B);
            usleep(sleep_time);
            printf("A%d is leaving the swimming pool\n", my_id);

            /* Exit protocol: last bather unlocks pool */
            semaphore_wait(sem_id_02);
            p_shm->bather_count--;
            if (p_shm->bather_count == 0)
            {
                semaphore_signal(sem_id_01); /* last bather unlocks S1 */
            }
            semaphore_signal(sem_id_02);
        }

        p_shm->ba1 = 0;
        exit(0);
    }

    /* ===== FORK A2 (Bather 2) ===== */
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Fork A2 failed. Terminating...\n");
        exit(1);
    }
    else if (pid == 0)
    {
        /* --- A2 CHILD PROCESS --- */
        my_id = 2;
        p_shm->ready_count++;
        while (p_shm->start_flag == 0)
        {
            usleep(1000);
        }

        while (p_shm->bw1 == 1 || p_shm->bw2 == 1)
        {
            sleep_time = rand_sleep(BATHER_TIME_02_A);
            usleep(sleep_time);

            semaphore_wait(sem_id_02);
            p_shm->bather_count++;
            if (p_shm->bather_count == 1)
            {
                semaphore_wait(sem_id_01);
            }
            semaphore_signal(sem_id_02);

            printf("A%d is entering the swimming pool\n", my_id);
            sleep_time = rand_sleep(BATHER_TIME_02_B);
            usleep(sleep_time);
            printf("A%d is leaving the swimming pool\n", my_id);

            semaphore_wait(sem_id_02);
            p_shm->bather_count--;
            if (p_shm->bather_count == 0)
            {
                semaphore_signal(sem_id_01);
            }
            semaphore_signal(sem_id_02);
        }

        p_shm->ba2 = 0;
        exit(0);
    }

    /* ===== FORK A3 (Bather 3) ===== */
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Fork A3 failed. Terminating...\n");
        exit(1);
    }
    else if (pid == 0)
    {
        /* --- A3 CHILD PROCESS --- */
        my_id = 3;
        p_shm->ready_count++;
        while (p_shm->start_flag == 0)
        {
            usleep(1000);
        }

        while (p_shm->bw1 == 1 || p_shm->bw2 == 1)
        {
            sleep_time = rand_sleep(BATHER_TIME_03_A);
            usleep(sleep_time);

            semaphore_wait(sem_id_02);
            p_shm->bather_count++;
            if (p_shm->bather_count == 1)
            {
                semaphore_wait(sem_id_01);
            }
            semaphore_signal(sem_id_02);

            printf("A%d is entering the swimming pool\n", my_id);
            sleep_time = rand_sleep(BATHER_TIME_03_B);
            usleep(sleep_time);
            printf("A%d is leaving the swimming pool\n", my_id);

            semaphore_wait(sem_id_02);
            p_shm->bather_count--;
            if (p_shm->bather_count == 0)
            {
                semaphore_signal(sem_id_01);
            }
            semaphore_signal(sem_id_02);
        }

        p_shm->ba3 = 0;
        exit(0);
    }

    /* 
     * ===== PARENT PROCESS (SAFEGUARD S) =====
     * Parent becomes the safeguard after creating all children
     */

    /* Wait for all 5 children to be ready */
    while (p_shm->ready_count < 5)
    {
        usleep(1000);
    }
    p_shm->start_flag = 1;      /* tell all children to start */

    /* Main loop: inspect while bathers are active */
    while (p_shm->ba1 == 1 || p_shm->ba2 == 1 || p_shm->ba3 == 1)
    {
        sleep_time = rand_sleep(SAFEGUARD_TIME_A);
        usleep(sleep_time);             /* wait between inspections */

        semaphore_wait(sem_id_01);      /* enter critical section */

        printf("S starts inspection ...\n");
        sleep_time = rand_sleep(SAFEGUARD_TIME_B);
        usleep(sleep_time);             /* time inspecting */
        printf("S is leaving the bathing area ..\n");

        semaphore_signal(sem_id_01);    /* exit critical section */
    }

    /* Wait for all children to terminate */
    for (i = 0; i < 5; i++)
    {
        wait(&status);
    }

    /* 
     * ===== CLEANUP IPC RESOURCES =====
     * Safeguard is responsible for deleting all IPC objects
     */
    ret_val = shmdt(p_shm);
    if (ret_val != 0)
    {
        fprintf(stderr, "Failed to detach shared memory.\n");
    }

    ret_val = shmctl(shm_id, IPC_RMID, NULL);
    if (ret_val != 0)
    {
        fprintf(stderr, "Failed to delete shared memory.\n");
    }

    ret_val = semctl(sem_id_01, 0, IPC_RMID);
    if (ret_val != 0)
    {
        fprintf(stderr, "Failed to delete semaphore S1.\n");
    }

    ret_val = semctl(sem_id_02, 0, IPC_RMID);
    if (ret_val != 0)
    {
        fprintf(stderr, "Failed to delete semaphore S2.\n");
    }

    return 0;
}
