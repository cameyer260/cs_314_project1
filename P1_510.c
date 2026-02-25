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
    pid_t pid;
    int status;
    int child_id;
    int my_id;
    
    int sem_id_01, sem_id_02;
    int shm_id;
    struct shared_memory *p_shm;
    int shm_size;
    union semun argument;
    int ret_val;

    srand((unsigned int)time(NULL));

    printf("=== PHASE 3: Create Five Child Processes ===\n\n");

    /* 
     * ===== Create IPC Resources =====
     * Parent creates shared memory and semaphores BEFORE forking.
     * All child processes will inherit access to these IPC resources.
     */

    printf("Creating IPC resources...\n");

    /* Create and attach shared memory */
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

    /* Initialize shared memory fields */
    p_shm->ready_count = 0;    /* children increment when ready */
    p_shm->start_flag = 0;     /* parent sets to 1 when all ready */
    p_shm->bw1 = 0;
    p_shm->bw2 = 0;
    p_shm->ba1 = 0;
    p_shm->ba2 = 0;
    p_shm->ba3 = 0;
    p_shm->bather_count = 0;
    printf("  Initialized shared memory values\n");

    /* Create semaphore S1 (mutex for pool access) */
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
    printf("  Created semaphore S1 (key: %d, id: %d, init: 1)\n", SEM_KEY_01, sem_id_01);

    /* Create semaphore S2 (mutex for bather count) */
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
    printf("  Created semaphore S2 (key: %d, id: %d, init: 1)\n", SEM_KEY_02, sem_id_02);

    /* 
     * ===== Create 5 Child Processes =====
     * Parent forks sequentially: B1, B2, A1, A2, A3
     * 
     * fork() returns:
     *   - pid < 0: error
     *   - pid == 0: we are in the CHILD process
     *   - pid > 0: we are in the PARENT process (pid is child's PID)
     */
    printf("\nCreating 5 child processes...\n");
    printf("Parent (Safeguard) PID: %d\n\n", getpid());

    /* ===== Fork B1 (Boiler-man 1) ===== */
    child_id = 1;
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Fork B1 failed. Terminating...\n");
        exit(1);
    }
    else if (pid == 0)
    {
        /* --- B1 CHILD PROCESS STARTS HERE --- */
        my_id = 1;                           /* B1's identifier */
        p_shm->ready_count++;                /* signal parent: I'm ready */
        while (p_shm->start_flag == 0)       /* wait for parent's go signal */
        {
            usleep(1000);                    /* not a busy loop - we sleep */
        }
        /* --- B1 main work will go here (Phase 5) --- */
        printf("B1 (PID: %d) starting...\n", getpid());
        usleep(500000);                      /* placeholder work */
        printf("B1 (PID: %d) done.\n", getpid());
        exit(0);                             /* B1 terminates */
    }
    /* --- PARENT continues here after fork --- */
    printf("Created B1 (PID: %d)\n", pid);

    /* ===== Fork B2 (Boiler-man 2) ===== */
    child_id = 2;
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Fork B2 failed. Terminating...\n");
        exit(1);
    }
    else if (pid == 0)
    {
        /* --- B2 CHILD PROCESS STARTS HERE --- */
        my_id = 2;
        p_shm->ready_count++;
        while (p_shm->start_flag == 0)
        {
            usleep(1000);
        }
        printf("B2 (PID: %d) starting...\n", getpid());
        usleep(500000);
        printf("B2 (PID: %d) done.\n", getpid());
        exit(0);
    }
    printf("Created B2 (PID: %d)\n", pid);

    /* ===== Fork A1 (Bather 1) ===== */
    child_id = 3;
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Fork A1 failed. Terminating...\n");
        exit(1);
    }
    else if (pid == 0)
    {
        /* --- A1 CHILD PROCESS STARTS HERE --- */
        my_id = 1;                           /* A1's identifier */
        p_shm->ready_count++;
        while (p_shm->start_flag == 0)
        {
            usleep(1000);
        }
        printf("A1 (PID: %d) starting...\n", getpid());
        usleep(500000);
        printf("A1 (PID: %d) done.\n", getpid());
        exit(0);
    }
    printf("Created A1 (PID: %d)\n", pid);

    /* ===== Fork A2 (Bather 2) ===== */
    child_id = 4;
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Fork A2 failed. Terminating...\n");
        exit(1);
    }
    else if (pid == 0)
    {
        /* --- A2 CHILD PROCESS STARTS HERE --- */
        my_id = 2;
        p_shm->ready_count++;
        while (p_shm->start_flag == 0)
        {
            usleep(1000);
        }
        printf("A2 (PID: %d) starting...\n", getpid());
        usleep(500000);
        printf("A2 (PID: %d) done.\n", getpid());
        exit(0);
    }
    printf("Created A2 (PID: %d)\n", pid);

    /* ===== Fork A3 (Bather 3) ===== */
    child_id = 5;
    pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "Fork A3 failed. Terminating...\n");
        exit(1);
    }
    else if (pid == 0)
    {
        /* --- A3 CHILD PROCESS STARTS HERE --- */
        my_id = 3;
        p_shm->ready_count++;
        while (p_shm->start_flag == 0)
        {
            usleep(1000);
        }
        printf("A3 (PID: %d) starting...\n", getpid());
        usleep(500000);
        printf("A3 (PID: %d) done.\n", getpid());
        exit(0);
    }
    printf("Created A3 (PID: %d)\n", pid);

    /* 
     * ===== Startup Synchronization =====
     * Parent waits for all 5 children to signal ready, then starts them.
     */
    printf("\nWaiting for all children to be ready...\n");
    while (p_shm->ready_count < 5)
    {
        usleep(1000);
    }
    printf("All 5 children ready. Setting start_flag = 1\n\n");
    p_shm->start_flag = 1;

    /* ===== Verify Process Count ===== */
    printf("Run 'ps -a' in another terminal to verify 6 processes.\n");
    printf("Press Enter to continue...\n");
    getchar();

    /* 
     * ===== Clean Termination =====
     * Parent waits for all children to exit, then deletes IPC resources.
     */
    printf("\nWaiting for all children to terminate...\n");
    for (i = 0; i < 5; i++)
    {
        pid = wait(&status);
        printf("Child (PID: %d) terminated.\n", pid);
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

    ret_val = semctl(sem_id_02, 0, IPC_RMID);
    if (ret_val != 0)
    {
        fprintf(stderr, "Failed to delete semaphore S2.\n");
    }
    else
    {
        printf("  Deleted semaphore S2\n");
    }

    printf("\n=== PHASE 3 Complete ===\n");
    printf("All 5 children created, synchronized, and terminated.\n");
    printf("Verify with 'ps -a' and 'ipcs -s' and 'ipcs -m'.\n");

    return 0;
}
