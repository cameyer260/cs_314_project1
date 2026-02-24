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

/* ========== RAND_SLEEP FUNCTION (Task 1.4) ========== */

int rand_sleep(int max_time)
{
    float temp_rand;
    float temp_sleeptime;

    temp_rand = (float)((rand() % 100)) / 100.0;
    temp_sleeptime = ((float)max_time) * temp_rand;

    return ((int)temp_sleeptime);
}

/* ========== MAIN FUNCTION ========== */

int main(void)
{
    int i;
    int sleep_time;
    int sem_id_01, sem_id_02;
    int shm_id;
    struct shared_memory *p_shm;
    int shm_size;
    union semun argument;
    int ret_val;

    /* Initialize random seed */
    srand((unsigned int)time(NULL));

    printf("=== PHASE 1: IPC Setup Test ===\n\n");

    /* ===== Task 1.5: Test Semaphore Creation ===== */
    printf("Task 1.5: Testing semaphore creation...\n");

    /* Create semaphore S1 (key 8370) */
    sem_id_01 = semget(SEM_KEY_01, 1, 0666 | IPC_CREAT);
    if (sem_id_01 < 0)
    {
        fprintf(stderr, "Failed to create semaphore S1. Terminating...\n");
        exit(1);
    }
    printf("  Created semaphore S1 with key %d, id %d\n", SEM_KEY_01, sem_id_01);

    /* Initialize semaphore S1 to 1 */
    argument.val = 1;
    if (semctl(sem_id_01, 0, SETVAL, argument) < 0)
    {
        fprintf(stderr, "Failed to initialize semaphore S1. Terminating...\n");
        exit(1);
    }
    printf("  Initialized semaphore S1 to value 1\n");

    /* Create semaphore S2 (key 8371) */
    sem_id_02 = semget(SEM_KEY_02, 1, 0666 | IPC_CREAT);
    if (sem_id_02 < 0)
    {
        fprintf(stderr, "Failed to create semaphore S2. Terminating...\n");
        exit(1);
    }
    printf("  Created semaphore S2 with key %d, id %d\n", SEM_KEY_02, sem_id_02);

    /* Initialize semaphore S2 to 1 */
    argument.val = 1;
    if (semctl(sem_id_02, 0, SETVAL, argument) < 0)
    {
        fprintf(stderr, "Failed to initialize semaphore S2. Terminating...\n");
        exit(1);
    }
    printf("  Initialized semaphore S2 to value 1\n");

    /* ===== Task 1.6: Test Shared Memory Creation ===== */
    printf("\nTask 1.6: Testing shared memory creation...\n");

    /* Get size of shared memory structure */
    shm_size = sizeof(struct shared_memory);
    if (shm_size <= 0)
    {
        fprintf(stderr, "sizeof error in acquiring shared memory size. Terminating...\n");
        exit(1);
    }

    /* Create shared memory segment */
    shm_id = shmget(SHM_KEY, shm_size, 0666 | IPC_CREAT);
    if (shm_id < 0)
    {
        fprintf(stderr, "Failed to create shared memory. Terminating...\n");
        exit(1);
    }
    printf("  Created shared memory with key %d, id %d, size %d bytes\n", SHM_KEY, shm_id, shm_size);

    /* Attach shared memory */
    p_shm = (struct shared_memory *)shmat(shm_id, NULL, 0);
    if (p_shm == (struct shared_memory *)-1)
    {
        fprintf(stderr, "Failed to attach shared memory. Terminating...\n");
        exit(1);
    }
    printf("  Attached shared memory to process\n");

    /* Initialize shared memory values */
    p_shm->ready_count = 0;
    p_shm->start_flag = 0;
    p_shm->bw1 = 0;
    p_shm->bw2 = 0;
    p_shm->ba1 = 0;
    p_shm->ba2 = 0;
    p_shm->ba3 = 0;
    p_shm->bather_count = 0;
    printf("  Initialized shared memory values to 0\n");

    /* ===== Task 1.4: Test rand_sleep function ===== */
    printf("\nTask 1.4: Testing rand_sleep function...\n");
    for (i = 0; i < 5; i++)
    {
        sleep_time = rand_sleep(BOILERMAN_TIME_01_A);
        printf("  Random sleep time %d: %d microseconds (%.3f seconds)\n", 
               i + 1, sleep_time, (float)sleep_time / 1000000.0);
    }

    /* ===== Cleanup: Delete IPC resources ===== */
    printf("\nCleaning up IPC resources...\n");

    /* Detach shared memory */
    ret_val = shmdt(p_shm);
    if (ret_val != 0)
    {
        fprintf(stderr, "Failed to detach shared memory.\n");
    }
    else
    {
        printf("  Detached shared memory\n");
    }

    /* Delete shared memory */
    ret_val = shmctl(shm_id, IPC_RMID, NULL);
    if (ret_val != 0)
    {
        fprintf(stderr, "Failed to delete shared memory.\n");
    }
    else
    {
        printf("  Deleted shared memory\n");
    }

    /* Delete semaphore S1 */
    ret_val = semctl(sem_id_01, 0, IPC_RMID);
    if (ret_val != 0)
    {
        fprintf(stderr, "Failed to delete semaphore S1.\n");
    }
    else
    {
        printf("  Deleted semaphore S1\n");
    }

    /* Delete semaphore S2 */
    ret_val = semctl(sem_id_02, 0, IPC_RMID);
    if (ret_val != 0)
    {
        fprintf(stderr, "Failed to delete semaphore S2.\n");
    }
    else
    {
        printf("  Deleted semaphore S2\n");
    }

    printf("\n=== PHASE 1 Complete ===\n");
    printf("All IPC resources created, tested, and cleaned up successfully.\n");

    return 0;
}
