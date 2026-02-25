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
 *
 * Synchronized Swimming Pool Control Problem
 * Using UNIX standard semaphores and shared memory
 *
 **********************************
 ****/

#include <stdio.h>          // for printf
#include <stdlib.h>         // for rand, exit
#include <sys/types.h>      // for pid_t
#include <unistd.h>         // for usleep, fork
#include <time.h>           // for time
#include <sys/ipc.h>        // for IPC
#include <sys/shm.h>        // for shared memory
#include <sys/sem.h>        // for semaphores
#include <sys/wait.h>       // for wait

/* ========== CONSTANTS (Task 1.3) ========== */

#define NUM_REPEAT                 50       // each boiler-man repeats

/* Personal keys from 04_personal_keys.md */
#define SEM_KEY_01                 8370     // semaphore key (MUTEX/S1)
#define SEM_KEY_02                 8371     // semaphore key (S2)
#define SHM_KEY                    7205     // shared memory key

/* Timing constants */
#define BATHER_TIME_01_A           300000   // 300ms = 0.3 seconds
#define BATHER_TIME_01_B           800000   // 800ms = 0.8 seconds

#define BATHER_TIME_02_A           300000   // 300ms = 0.3 seconds
#define BATHER_TIME_02_B           800000   // 800ms = 0.8 seconds

#define BATHER_TIME_03_A           300000   // 300ms = 0.3 seconds
#define BATHER_TIME_03_B           800000   // 800ms = 0.8 seconds

#define BOILERMAN_TIME_01_A        1200000  // 1200ms = 1.2 seconds
#define BOILERMAN_TIME_01_B        1600000  // 1600ms = 1.6 seconds

#define BOILERMAN_TIME_02_A        1200000  // 1200ms = 1.2 seconds
#define BOILERMAN_TIME_02_B        1600000  // 1600ms = 1.6 seconds

#define SAFEGUARD_TIME_A           1200000  // 1200ms = 1.2 seconds
#define SAFEGUARD_TIME_B           1600000  // 1600ms = 1.6 seconds

/* ========== SHARED MEMORY STRUCTURE ========== */

struct shared_memory {
    int ready_count;      // # of processes ready
    int start_flag;       // 1 = all processes can start
    int bw1, bw2;         // boiler-man status (1=active, 0=done)
    int ba1, ba2, ba3;    // bather status (1=active, 0=done)
    int bather_count;     // # of bathers in pool
};

/* ========== SEMAPHORE UNION (required for semctl) ========== */
/* NOTE: This #ifndef block is only for local macOS compilation. */
/* The SIUE Unix server (os.cs.siue.edu) does NOT define semun in headers, */
/* so the union must be defined. This block will be removed before submission. */
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

/* ========== RAND_SLEEP FUNCTION (Task 1.4) ========== */

int rand_sleep(int max_time)
{
    float temp_rand;
    float temp_sleeptime;

    temp_rand = (float)((rand() % 100)) / 100.0;
    temp_sleeptime = ((float)max_time) * temp_rand;

    return ((int)temp_sleeptime);
}

/* ========== SEMAPHORE OPERATIONS (Phase 4) ========== */

void semaphore_wait(int sem_id)
{
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = -1;      /* decrement (P operation) */
    op.sem_flg = 0;      /* blocking */
    semop(sem_id, &op, 1);
}

void semaphore_signal(int sem_id)
{
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op = 1;       /* increment (V operation) */
    op.sem_flg = 0;
    semop(sem_id, &op, 1);
}

/* ========== MAIN FUNCTION ========== */

int main(void)
{
    int i;
    pid_t pid;
    int status;
    
    int sem_id_01;
    int shm_id;
    struct shared_memory *p_shm;
    int shm_size;
    union semun argument;
    int ret_val;
    
    int expected_total;

    srand((unsigned int)time(NULL));

    printf("=== PHASE 4: Synchronization with Semaphores ===\n\n");

    /* ===== Create IPC Resources ===== */
    printf("Creating IPC resources...\n");

    shm_size = sizeof(struct shared_memory);
    shm_id = shmget(SHM_KEY, shm_size, 0666 | IPC_CREAT);
    if (shm_id < 0)
    {
        fprintf(stderr, "Failed to create shared memory. Terminating...\n");
        exit(1);
    }
    printf("  Created shared memory (key: %d, id: %d)\n", SHM_KEY, shm_id);

    p_shm = (struct shared_memory *)shmat(shm_id, NULL, 0);
    if (p_shm == (struct shared_memory *)-1)
    {
        fprintf(stderr, "Failed to attach shared memory. Terminating...\n");
        exit(1);
    }
    printf("  Attached shared memory\n");

    /* Initialize shared memory */
    p_shm->ready_count = 0;
    p_shm->start_flag = 0;
    p_shm->bw1 = 0;
    p_shm->bw2 = 0;
    p_shm->ba1 = 0;
    p_shm->ba2 = 0;
    p_shm->ba3 = 0;
    p_shm->bather_count = 0;    /* use this field as test counter */

    /* Create semaphore S1 (mutex) */
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
    printf("  Created semaphore S1 (key: %d, id: %d, init: 1)\n\n", SEM_KEY_01, sem_id_01);

    /* 
     * ===== Task 4.3: Test Mutual Exclusion =====
     * Both parent and child increment a counter 100000 times each.
     * With semaphore protection, final value should be exactly 200000.
     * Without protection, race conditions would cause incorrect value.
     */
    printf("Task 4.3: Testing mutual exclusion...\n");
    printf("Parent and child will each increment counter 100000 times.\n");
    printf("Expected final value: 200000\n\n");

    p_shm->bather_count = 0;
    expected_total = 200000;

    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Fork failed. Terminating...\n");
        exit(1);
    }
    else if (pid == 0)
    {
        /* --- CHILD PROCESS --- */
        for (i = 0; i < 100000; i++)
        {
            semaphore_wait(sem_id_01);
            p_shm->bather_count++;
            semaphore_signal(sem_id_01);
        }
        printf("Child finished incrementing.\n");
        exit(0);
    }

    /* --- PARENT PROCESS --- */
    for (i = 0; i < 100000; i++)
    {
        semaphore_wait(sem_id_01);
        p_shm->bather_count++;
        semaphore_signal(sem_id_01);
    }
    printf("Parent finished incrementing.\n");

    /* Wait for child to finish */
    wait(&status);

    printf("\nFinal counter value: %d\n", p_shm->bather_count);
    if (p_shm->bather_count == expected_total)
    {
        printf("SUCCESS: Counter reached expected value (no race condition).\n");
    }
    else
    {
        printf("FAILURE: Counter mismatch (race condition detected).\n");
    }

    /* ===== Cleanup IPC Resources ===== */
    printf("\nCleaning up IPC resources...\n");
    
    ret_val = shmdt(p_shm);
    if (ret_val != 0)
    {
        fprintf(stderr, "Failed to detach shared memory.\n");
    }
    else
    {
        printf("  Detached shared memory\n");
    }

    ret_val = shmctl(shm_id, IPC_RMID, NULL);
    if (ret_val != 0)
    {
        fprintf(stderr, "Failed to delete shared memory.\n");
    }
    else
    {
        printf("  Deleted shared memory\n");
    }

    ret_val = semctl(sem_id_01, 0, IPC_RMID);
    if (ret_val != 0)
    {
        fprintf(stderr, "Failed to delete semaphore S1.\n");
    }
    else
    {
        printf("  Deleted semaphore S1\n");
    }

    printf("\n=== PHASE 4 Complete ===\n");

    return 0;
}
