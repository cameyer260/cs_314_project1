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

    srand((unsigned int)time(NULL));

    printf("=== PHASE 2: Fork System Call Test ===\n\n");

    /* ===== Task 2.1: Test Single fork() ===== */
    printf("Task 2.1: Testing single fork()...\n");
    printf("Parent PID: %d\n\n", getpid());

    pid = fork();

    if (pid < 0)
    {
        fprintf(stderr, "Fork failed. Terminating...\n");
        exit(1);
    }
    else if (pid == 0)
    {
        /* Child process */
        printf("I am the child (PID: %d, PPID: %d)\n", getpid(), getppid());
        usleep(100000);
        printf("Child process exiting.\n");
        exit(0);
    }
    else
    {
        /* Parent process */
        printf("I am the parent (PID: %d)\n", getpid());
        printf("Parent waiting for child (child PID: %d)...\n", pid);
        wait(&status);
        printf("Child process terminated.\n");
    }

    printf("\nTask 2.1 complete.\n");

    /* ===== Task 2.2: Document fork() Behavior ===== */
    printf("\n=== Task 2.2: fork() Behavior Notes ===\n");
    printf("Variables INHERITED by child (copied):\n");
    printf("  - All variables (get a COPY of parent's memory)\n");
    printf("  - File descriptors\n");
    printf("  - Signal handlers\n");
    printf("  - Working directory\n");
    printf("\nVariables NOT inherited by child:\n");
    printf("  - PID (process ID) - child gets new PID\n");
    printf("  - PPID (parent process ID) - child's PPID is parent's PID\n");
    printf("  - Pending signals (reset to empty)\n");
    printf("  - File locks\n");
    printf("\nNOTE: Child gets a COPY of parent's memory, not shared memory.\n");
    printf("      Changes to variables in child do NOT affect parent.\n");

    /* ===== Task 2.3: Test Sequential fork() Calls ===== */
    printf("\n=== Task 2.3: Testing Sequential fork() Calls ===\n");
    printf("Creating 2 child processes sequentially...\n\n");

    /* First fork */
    child_id = 1;
    pid = fork();

    if (pid < 0)
    {
        fprintf(stderr, "First fork failed. Terminating...\n");
        exit(1);
    }
    else if (pid == 0)
    {
        /* First child process */
        printf("Child %d created (PID: %d, PPID: %d)\n", child_id, getpid(), getppid());
        usleep(200000);
        printf("Child %d (PID: %d) exiting.\n", child_id, getpid());
        exit(0);
    }

    /* Parent continues here */
    printf("Parent: Created first child (PID: %d)\n", pid);

    /* Second fork */
    child_id = 2;
    pid = fork();

    if (pid < 0)
    {
        fprintf(stderr, "Second fork failed. Terminating...\n");
        exit(1);
    }
    else if (pid == 0)
    {
        /* Second child process */
        printf("Child %d created (PID: %d, PPID: %d)\n", child_id, getpid(), getppid());
        usleep(100000);
        printf("Child %d (PID: %d) exiting.\n", child_id, getpid());
        exit(0);
    }

    /* Parent continues here */
    printf("Parent: Created second child (PID: %d)\n", pid);
    printf("Parent: Waiting for both children...\n\n");

    /* Wait for both children */
    for (i = 0; i < 2; i++)
    {
        pid = wait(&status);
        printf("Parent: Child with PID %d terminated.\n", pid);
    }

    printf("\n=== PHASE 2 Complete ===\n");
    printf("All fork tests completed successfully.\n");
    printf("Verify with 'ps -a' that no orphan processes remain.\n");

    return 0;
}
